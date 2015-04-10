#include "bulb.h"
#include "half.h"
#include <boost/compute/user_event.hpp>

#include <boost/compute/core.hpp>
#include <boost/compute/system.hpp>
#include <boost/compute/utility/source.hpp>

#define BOOST_NO_VARIADIC_TEMPLATES
#include <boost/compute/utility/dim.hpp>

namespace djg
{
namespace compute = boost::compute;

Bulb::Bulb()
{

}

Bulb::~Bulb()
{

}


void Bulb::init(compute::command_queue & queue,
                compute::image2d input,
                size_t width,
                size_t height,
                size_t slices)
{
    if (slices < 4)
        slices = 4;

    m_queue = queue;
    m_context = queue.get_context();

    compute::image_format mformat = compute::image_format(compute::image_format::r,    compute::image_format::float16);
    compute::image_format wformat = compute::image_format(compute::image_format::rgba, compute::image_format::snorm_int8);
    compute::image_format oformat = compute::image_format(compute::image_format::rg,   compute::image_format::signed_int8);

    if (input.get()){
        m_memory.push_back(input);
    } else {
        m_memory.push_back(compute::image2d(m_context, width, height, mformat));
    }
    for (size_t i = 0; i < slices; ++i) {
        m_memory.push_back(compute::image2d(m_context, width, height, mformat));
        m_weights.push_back(compute::image2d(m_context, width*2, height, wformat)); // 4 channels * 2 = 8 weights [0..1]
        m_offsets.push_back(compute::image2d(m_context, width*8, height, oformat)); // 2 channels * 8 = 8 offsets (x,y)
    }
}

void Bulb::fill_slices(compute::float4_ mem_fill, compute::float4_ wei_fill, compute::int2_ off_fill,
                       const compute::wait_list &events,
                       compute::wait_list *event_list)
{
    fill_slices_inner(m_memory, &mem_fill, events, event_list);
    fill_slices_inner(m_weights, &wei_fill, events, event_list);
    fill_slices_inner(m_offsets, &off_fill, events, event_list);
}

void Bulb::fill_slices_inner(const std::vector<compute::image2d> &slices,
                             const void *fill_color,
                             const compute::wait_list &events,
                             compute::wait_list *event_list)
{
    size_t nslices = slices.size();
    while (nslices--) {
        const compute::image2d & image = slices[nslices];
        compute::event newevent;
        compute::event* pevent = NULL;
        if (event_list) {
            pevent = &newevent;
            event_list->insert(newevent);
        }
        m_queue.enqueue_fill_image(image, fill_color, image.origin(), image.size(), events, pevent);
    }
}

void Bulb::read_slice(size_t slice, const compute::wait_list &events, compute::event *event)
{
    compute::image2d image = m_memory[slice];
    size_t row_pitch = 0;
    size_t slice_pitch = 0;
    const compute::extents<2> origin = image.origin();
    const compute::extents<2> size = image.size();
    cl_map_flags flags = compute::command_queue::map_read;
    compute::event map_event, *pmap_event = NULL;
    compute::user_event *puser_event = NULL;
    compute::wait_list unmap_wait;

    if (event) {
        // Async exec
        compute::user_event user_event(m_context);
        unmap_wait.insert(user_event);
        puser_event = &user_event;
        pmap_event = &map_event;
    }

    char * const pImage2D = reinterpret_cast<char *>(
                m_queue.enqueue_map_image(
                    image,
                    flags,
                    origin,
                    size,
                    &row_pitch,
                    &slice_pitch,
                    events,
                    pmap_event)
                );

    size_t element_size = image.get_info<size_t>(CL_IMAGE_ELEMENT_SIZE);

    auto func = [=]()
    {
        char * pImage1D = pImage2D;
        for(size_t h = origin[1]; h < size[1]; ++h) {
            char *pRow = pImage1D;
            for(size_t w = origin[0]; w < size[0]; ++w) {
                half& element = *reinterpret_cast<half*>(pRow);
                float fLuminance = element;
                element = fLuminance;
                pRow += element_size;
            }
            pImage1D += row_pitch;
        }
        if(puser_event) {
            puser_event->set_status(compute::event::complete);
        }
    };

    if (event) {
        // Async exec
        //compute::event::execution_status status = pmap_event->get_status();
        pmap_event->set_callback(func);
    } else {
        func();
    }
    m_queue.enqueue_unmap_buffer(image, pImage2D, unmap_wait, event);
}

size_t Bulb::bytes_per_pixel(Bulb::nchanels nc)
{
    size_t bytes = 0;
    switch (nc) {
    case Bulb::R:
        bytes = 1;
        break;
    case Bulb::RG:
        bytes = 2;
        break;
    case Bulb::RGBA:
        bytes = 4;
        break;
    default:
        break;
    }
    return bytes;
}

//#include "types.h"
//__kernel void pack_textures_2D(__global i2 * images, __read_only image2d_t tex0, __read_only image2d_t tex1, __read_only image2d_t tex2)
//{
// images[0] = tex0;
// images[1] = tex1;
// images[2] = tex2;
//}

void Bulb::make_kernels()
{
    //using compute::dim;

    const compute::context &context = m_queue.get_context();

    // simple box filter kernel source
    const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
        float get_pixel(int2 coord,
                        __read_only image2d_t mem_input_0,
                        __read_only image2d_t mem_input_1,
                        __read_only image2d_t mem_input_2,
                        __read_only image2d_t mem_input_3,
                        __read_only image2d_t wei_input,
                        __read_only image2d_t off_input)
        {
            float acc = 0;
            float value;
            int2 offsets;
            const sampler_t sampler = CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

            int2 c_off = (int2)(coord.x*8, coord.y);
            float4 weights_0 = read_imagef(wei_input, sampler, (int2)(coord.x*2, coord.y));
            float4 weights_1 = read_imagef(wei_input, sampler, (int2)(coord.x*2+1, coord.y));
            offsets = read_imagei(off_input, sampler, c_off).xy;
            value = read_imagef(mem_input_0, sampler, offsets + coord).r;
            acc += value * weights_0.x;
            offsets = read_imagei(off_input, sampler, c_off + (int2)(1, 0)).xy;
            value = read_imagef(mem_input_1, sampler, offsets + coord).r;
            acc += value * weights_0.y;
            offsets = read_imagei(off_input, sampler, c_off + (int2)(2, 0)).xy;
            value = read_imagef(mem_input_2, sampler, offsets + coord).r;
            acc += value * weights_0.z;
            offsets = read_imagei(off_input, sampler, c_off + (int2)(3, 0)).xy;
            value = read_imagef(mem_input_3, sampler, offsets + coord).r;
            acc += value * weights_0.w;
            offsets = read_imagei(off_input, sampler, c_off + (int2)(4, 0)).xy;
            value = read_imagef(mem_input_0, sampler, offsets + coord).r;
            acc += value * weights_1.x;
            offsets = read_imagei(off_input, sampler, c_off + (int2)(5, 0)).xy;
            value = read_imagef(mem_input_1, sampler, offsets + coord).r;
            acc += value * weights_1.y;
            offsets = read_imagei(off_input, sampler, c_off + (int2)(6, 0)).xy;
            value = read_imagef(mem_input_2, sampler, offsets + coord).r;
            acc += value * weights_1.z;
            offsets = read_imagei(off_input, sampler, c_off + (int2)(7, 0)).xy;
            value = read_imagef(mem_input_3, sampler, offsets + coord).r;
            acc += value * weights_1.w;

            return acc;
        }

        __kernel void kernel_1(__read_only image2d_t mem_input_0,
                               __read_only image2d_t mem_input_1,
                               __read_only image2d_t mem_input_2,
                               __read_only image2d_t mem_input_3,
                               __read_only image2d_t wei_input,
                               __read_only image2d_t off_input,
                               __write_only image2d_t mem_output)
        {
            int x = get_global_id(0);
            int y = get_global_id(1);

            int2 coord_0 = (int2)(x*2+0, y);
            int2 coord_1 = (int2)(x*2+1, y);

            float acc_0 = get_pixel(coord_0,
                                    mem_input_0,
                                    mem_input_1,
                                    mem_input_2,
                                    mem_input_3,
                                    wei_input,
                                    off_input);
            float acc_1 = get_pixel(coord_1,
                                    mem_input_0,
                                    mem_input_1,
                                    mem_input_2,
                                    mem_input_3,
                                    wei_input,
                                    off_input);

            // Escribir 32 bits
            write_imagef(mem_output, coord_0, acc_0);
            write_imagef(mem_output, coord_1, acc_1);
        }
    );

    // build box filter program
    compute::program program = compute::program::create_with_source(source, context);
    program.build("-cl-no-signed-zeros -cl-fast-relaxed-math -cl-mad-enable");

    // setup box filter kernel
    m_kernel_1 = compute::kernel(program, "kernel_1");
}

void Bulb::execute_kernel_1(size_t slice, const compute::wait_list &events, compute::event *event)
{
    if (slice == 0) {
        m_kernel_1.set_arg(0, m_memory[0]);
        m_kernel_1.set_arg(1, m_memory[0]);
        m_kernel_1.set_arg(2, m_memory[0]);
        m_kernel_1.set_arg(3, m_memory[0]);
    } else if (slice == 1) {
        m_kernel_1.set_arg(0, m_memory[0]);
        m_kernel_1.set_arg(1, m_memory[0]);
        m_kernel_1.set_arg(2, m_memory[1]);
        m_kernel_1.set_arg(3, m_memory[1]);
    } else if (slice == 2) {
        m_kernel_1.set_arg(0, m_memory[0]);
        m_kernel_1.set_arg(1, m_memory[0]);
        m_kernel_1.set_arg(2, m_memory[1]);
        m_kernel_1.set_arg(3, m_memory[2]);
    } else if (slice < m_memory.size()-1) {
        m_kernel_1.set_arg(0, m_memory[slice-3]);
        m_kernel_1.set_arg(1, m_memory[slice-2]);
        m_kernel_1.set_arg(2, m_memory[slice-1]);
        m_kernel_1.set_arg(3, m_memory[slice-0]);
    } else {
        return;
    }

    m_kernel_1.set_arg(4, m_weights[slice]);
    m_kernel_1.set_arg(5, m_offsets[slice]);
    m_kernel_1.set_arg(6, m_memory[slice+1]);

    compute::extents<2> size = m_memory[0].size();
    m_queue.enqueue_nd_range_kernel(m_kernel_1, compute::dim(0, 0), compute::dim(size[0]/2, size[1]), compute::dim(0, 0), events, event);
}

void Bulb::execute_kernel_1(const compute::wait_list &in_events, compute::wait_list *out_events)
{
    size_t slice = m_memory.size();

    if (slice--) {
        while (slice--) {
            if (out_events) {
                compute::event event;
                execute_kernel_1(slice, in_events, &event);
                out_events->insert(event);
            } else {
                execute_kernel_1(slice, in_events);
            }
        }
    }
}

}
