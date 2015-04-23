#include "bulb.h"
#include "half.h"
#include <boost/compute/user_event.hpp>

#include <boost/compute/core.hpp>
#include <boost/compute/system.hpp>
#include <boost/compute/utility/source.hpp>

#define BOOST_NO_VARIADIC_TEMPLATES
#include <boost/compute/utility/dim.hpp>

#include "exception_error.hpp"
#include "types.hpp"

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
                size_t width,
                size_t height,
                size_t slices,
                compute::image2d input)
{
    if (slices < 4)
        slices = 4;

    m_queue = queue;
    m_context = queue.get_context();

    compute::image_format memory_format = get_memory_format();
    compute::image_format weight_format = compute::image_format(compute::image_format::rgba, compute::image_format::float16);
    compute::image_format offset_format = compute::image_format(compute::image_format::rg,   compute::image_format::signed_int8);

    if (input.get()){
//        if (input.size() != compute::dim(width, height)) {
//            BOOST_THROW_EXCEPTION(computecl_error("Input image size mismatch."));
//        }
        BOOST_ASSERT(input.get_context() == this->get_context());
        compute::image_format iformat = input.get_format();
        compute::image_format::channel_order input_channel_order = iformat.get_channel_order();
        if (input_channel_order == compute::image_format::r
         || input_channel_order == compute::image_format::a
         || input_channel_order == compute::image_format::luminance
         || input_channel_order == compute::image_format::intensity){
            m_imageIn = input;
        } else {
            BOOST_THROW_EXCEPTION(computecl_error("Only one channel input images allowed."));
        }
    } else {
        m_imageIn = compute::image2d(m_context, width, height, memory_format);
    }
    for (size_t i = 0; i < slices; ++i) {
        m_memory.push_back(compute::image2d(m_context, width, height, memory_format));
        m_weight.push_back(compute::image2d(m_context, width*2, height, weight_format)); // 4 channels * 2 = 8 weights
        m_k.push_back(compute::image2d(m_context, width/4, height, weight_format)); // 4 channels / 4 = 1 k
        m_u.push_back(compute::image2d(m_context, width/4, height, weight_format)); // 4 channels / 4 = 1 u
        m_offset.push_back(compute::image2d(m_context, width*8, height, offset_format)); // 2 channels * 8 = 8 offsets (x,y)
    }
    m_imageTempOut = compute::image2d(m_context, width, height, memory_format);

    make_kernels();
}

#if defined(CL_VERSION_1_2) || defined(BOOST_COMPUTE_DOXYGEN_INVOKED)
void Bulb::fill_slices(compute::float4_ mem_fill,
                       compute::float4_ wei_fill,
                       compute::int2_ off_fill,
                       const compute::wait_list &events,
                       compute::wait_list *event_list)
{
    fill_slices_inner(m_memory, &mem_fill, events, event_list);
    fill_slices_inner(m_weight, &wei_fill, events, event_list);
    fill_slices_inner(m_offset, &off_fill, events, event_list);
}
#endif

void Bulb::clear_slices(const compute::wait_list &events,
                        compute::wait_list *event_list)
{
    const compute::half4_ hzero(0,0,0,0);
    const compute::half4_ hone(1,1,1,1);
    const compute::char4_ czero(0,0,0,0);
    rawfill_slices_inner(m_memory, &hzero, events, event_list);
    rawfill_slices_inner(m_weight, &hone, events, event_list);
    rawfill_slices_inner(m_k, &hone, events, event_list);
    rawfill_slices_inner(m_u, &hone, events, event_list);
    rawfill_slices_inner(m_offset, &czero, events, event_list);
}

#if defined(CL_VERSION_1_2) || defined(BOOST_COMPUTE_DOXYGEN_INVOKED)
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
#endif
void Bulb::rawfill_slices_inner(const std::vector<compute::image2d> &slices,
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
        m_queue.enqueue_rawfill_image_walking(image, fill_color, image.origin(), image.size(), events, pevent);
    }
}
/*
void Bulb::walk_memory_slice(size_t slice,
                         std::function<void(void * pelement, size_t x, size_t y)> f,
                         cl_map_flags flags,
                         const compute::wait_list &events,
                         compute::event *event)
{
    compute::image2d image = m_memory[slice];
    walk_image(image, f, flags, events, event);
}

void Bulb::walk_image(compute::image2d image,
                      std::function<void(void * pelement, size_t x, size_t y)> f,
                      cl_map_flags flags,
                      const compute::wait_list &events,
                      compute::event *event)
{
    size_t row_pitch = 0;
    size_t slice_pitch = 0;
    const compute::extents<2> origin = image.origin();
    const compute::extents<2> size = image.size();
    compute::event map_event, *pmap_event = NULL;
    compute::user_event user_event;
    compute::wait_list unmap_wait;

    if (event) {
        // Async exec
        user_event = compute::user_event(m_context);
        unmap_wait.insert(user_event);
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
        // Recorre todos los elementos de la imagen
        char * pImage1D = pImage2D;
        for(size_t h = origin[1]; h < size[1]; ++h) {
            char *pRow = pImage1D;
            for(size_t w = origin[0]; w < size[0]; ++w) {
                f(pRow, w, h);
                pRow += element_size;
            }
            pImage1D += row_pitch;
        }
        if(event) {
            user_event.set_status(compute::event::complete);
        }
    };

    if (event) {
        // Async exec
        pmap_event->set_callback(func);
    } else {
        func();
    }
    m_queue.enqueue_unmap_buffer(image, pImage2D, unmap_wait, event);
}
*/
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


void Bulb::make_kernels()
{
    const compute::context &context = m_queue.get_context();

    // simple box filter kernel source
    const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
        float4 transfer(float4 x, float4 k, float4 u)
        {
            float4 denom = (float4)(1,1,1,1) + half_exp(k*(u-x));
            float4 ret = half_divide(x, denom);
            return ret;
        }

        float get_pixel(int2 coord,
                        __read_only image2d_t mem_input_0,
                        __read_only image2d_t mem_input_1,
                        __read_only image2d_t mem_input_2,
                        __read_only image2d_t mem_input_3,
                        __read_only image2d_t wei_input,
                        __read_only image2d_t off_input)
        {
            float acc = 0;
            float value = 0.0;
            int2 offsets;
            const sampler_t sampler = CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

            int2 c_off = (int2)(coord.x*8, coord.y);
            float4 weights_0 = read_imagef(wei_input, sampler, (int2)(coord.x*2, coord.y));
            float4 weights_1 = read_imagef(wei_input, sampler, (int2)(coord.x*2+1, coord.y));
            offsets = read_imagei(off_input, sampler, c_off).xy;
            value = read_imagef(mem_input_0, sampler, offsets + coord).x;
            acc += value * weights_0.x;
            c_off.x += 1;
            offsets = read_imagei(off_input, sampler, c_off).xy;
            value = read_imagef(mem_input_1, sampler, offsets + coord).x;
            acc += value * weights_0.y;
            c_off.x += 1;
            offsets = read_imagei(off_input, sampler, c_off).xy;
            value = read_imagef(mem_input_2, sampler, offsets + coord).x;
            acc += value * weights_0.z;
            c_off.x += 1;
            offsets = read_imagei(off_input, sampler, c_off).xy;
            value = read_imagef(mem_input_3, sampler, offsets + coord).x;
            acc += value * weights_0.w;
            c_off.x += 1;
            offsets = read_imagei(off_input, sampler, c_off).xy;
            value = read_imagef(mem_input_0, sampler, offsets + coord).x;
            acc += value * weights_1.x;
            c_off.x += 1;
            offsets = read_imagei(off_input, sampler, c_off).xy;
            value = read_imagef(mem_input_1, sampler, offsets + coord).x;
            acc += value * weights_1.y;
            c_off.x += 1;
            offsets = read_imagei(off_input, sampler, c_off).xy;
            value = read_imagef(mem_input_2, sampler, offsets + coord).x;
            acc += value * weights_1.z;
            c_off.x += 1;
            offsets = read_imagei(off_input, sampler, c_off).xy;
            value = read_imagef(mem_input_3, sampler, offsets + coord).x;
            acc += value * weights_1.w;
//            printf("coord[%d,%d] acc = %f, value = %f, weight0x = %f, weight1w = %f\n\n",
//                    coord.x, coord.y, acc, value, weights_0.x, weights_1.w);

            return acc;
        }

        __kernel void kernel_1(__read_only image2d_t mem_input_0,
                               __read_only image2d_t mem_input_1,
                               __read_only image2d_t mem_input_2,
                               __read_only image2d_t mem_input_3,
                               __read_only image2d_t wei_input,
                               __read_only image2d_t k_input,
                               __read_only image2d_t u_input,
                               __read_only image2d_t off_input,
                               __write_only image2d_t mem_output)
        {
            int x = get_global_id(0);
            int y = get_global_id(1);

            const sampler_t sampler = CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

            int2 coord_0 = (int2)(x*4+0, y);
            int2 coord_1 = (int2)(x*4+1, y);
            int2 coord_2 = (int2)(x*4+2, y);
            int2 coord_3 = (int2)(x*4+3, y);

            float4 acc = (float4)(
                get_pixel(coord_0,
                    mem_input_0,
                    mem_input_1,
                    mem_input_2,
                    mem_input_3,
                    wei_input,
                    off_input),
                get_pixel(coord_1,
                    mem_input_0,
                    mem_input_1,
                    mem_input_2,
                    mem_input_3,
                    wei_input,
                    off_input),
                get_pixel(coord_2,
                    mem_input_0,
                    mem_input_1,
                    mem_input_2,
                    mem_input_3,
                    wei_input,
                    off_input),
                get_pixel(coord_3,
                    mem_input_0,
                    mem_input_1,
                    mem_input_2,
                    mem_input_3,
                    wei_input,
                    off_input));

            float4 k = read_imagef(k_input, sampler, coord_0);
            float4 u = read_imagef(u_input, sampler, coord_0);
            float4 tr = transfer(acc, k, u);

            // Escribir 64 bits
            write_imagef(mem_output, coord_0, tr.x);
            write_imagef(mem_output, coord_1, tr.y);
            write_imagef(mem_output, coord_2, tr.z);
            write_imagef(mem_output, coord_3, tr.w);
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
        m_kernel_1.set_arg(0, m_imageIn);
        m_kernel_1.set_arg(1, m_imageIn);
        m_kernel_1.set_arg(2, m_imageIn);
        m_kernel_1.set_arg(3, m_memory[slice]);
    } else if (slice == 1) {
        m_kernel_1.set_arg(0, m_imageIn);
        m_kernel_1.set_arg(1, m_imageIn);
        m_kernel_1.set_arg(2, m_memory[slice-1]);
        m_kernel_1.set_arg(3, m_memory[slice]);
    } else if (slice == 2) {
        m_kernel_1.set_arg(0, m_imageIn);
        m_kernel_1.set_arg(1, m_memory[slice-2]);
        m_kernel_1.set_arg(2, m_memory[slice-1]);
        m_kernel_1.set_arg(3, m_memory[slice]);
    } else if (slice < m_memory.size()-1) {
        m_kernel_1.set_arg(0, m_memory[slice-3]);
        m_kernel_1.set_arg(1, m_memory[slice-2]);
        m_kernel_1.set_arg(2, m_memory[slice-1]);
        m_kernel_1.set_arg(3, m_memory[slice]);
    } else {
        return;
    }

    m_kernel_1.set_arg(4, m_weight[slice]);
    m_kernel_1.set_arg(5, m_k[slice]);
    m_kernel_1.set_arg(6, m_u[slice]);
    m_kernel_1.set_arg(7, m_offset[slice]);
    m_kernel_1.set_arg(8, m_imageTempOut); // Read/Write Output trick.
    m_imageTempOut.swap(m_memory[slice]);
    compute::extents<2> size = m_memory[0].size();
    m_queue.enqueue_nd_range_kernel(m_kernel_1,
                                    compute::dim(0, 0),
                                    compute::dim(size[0]/4,
                                    size[1]),
                                    compute::dim(0, 0),
                                    events, event);
}

void Bulb::execute_kernel_1(const compute::wait_list &in_events, compute::wait_list *out_events)
{
    size_t slice = m_memory.size();

    // Execute all slices
    if (out_events) {
        while (slice--) {
            compute::event event;
            execute_kernel_1(slice, in_events, &event);
            out_events->insert(event);
        }
    } else {
        while (slice--) {
            execute_kernel_1(slice, in_events);
        }
    }
}

}
