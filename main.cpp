//---------------------------------------------------------------------------//
// Copyright (c) 2013 Kyle Lutz <kyle.r.lutz@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://kylelutz.github.com/compute for more information.
//---------------------------------------------------------------------------//

#include <iostream>
#include <algorithm>

#include <math.h>
#include <CL/cl.h>

#include <boost/compute/core.hpp>
#include <boost/compute/system.hpp>
#include <boost/compute/image/image2d.hpp>
#include <boost/compute/utility/dim.hpp>
#include <boost/compute/utility/source.hpp>

#include <boost/compute/command_queue.hpp>
#include <boost/compute/exception/opencl_error.hpp>
#include <boost/compute/image/image_format.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

//namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

#include "bulb.h"
#include "types.hpp"

namespace compute = boost::compute;

inline void box_filter_image(const compute::image2d &input,
                             compute::image2d &output,
                             compute::uint_ box_height,
                             compute::uint_ box_width,
                             compute::command_queue &queue)
{
    using compute::dim;

    const compute::context &context = queue.get_context();

    // simple box filter kernel source
    const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
        __kernel void box_filter(__read_only image2d_t input,
                                 __write_only image2d_t output,
                                 uint box_height,
                                 uint box_width)
        {
            int x = get_global_id(0);
            int y = get_global_id(1);
//                    // djg
                    const sampler_t sampler = CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
                    float4 value = read_imagef(input, sampler, (int2)(x, y));
                    value += (float4)( 1.1f, 1.1f, 1.1f, 1.1f );
                    write_imagef(output, (int2)(x, y), value);

//                    int h = get_image_height(input);
//                    int w = get_image_width(input);
//                    int k = box_width;
//                    int l = box_height;

//            if(x < k/2 || y < l/2 || x >= w-(k/2) || y >= h-(l/2)){
//                write_imagef(output, (int2)(x, y), (float4)(0, 0, 0, 1));
//            }
//            else {
//                const sampler_t sampler = CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

//                float4 sum = { 0, 0, 0, 0 };
//                for(int i = 0; i < k; i++){
//                    for(int j = 0; j < l; j++){
//                        sum += read_imagef(input, sampler, (int2)(x+i-k, y+j-l));
//                    }
//                }
//                sum /= (float) k * l;
//                float4 value = (float4)( sum.x, sum.y, sum.z, 1.f );
//                write_imagef(output, (int2)(x, y), value);
//            }
        }
    );

    // build box filter program
    compute::program program = compute::program::create_with_source(source, context);
    program.build("-cl-no-signed-zeros -cl-fast-relaxed-math -cl-mad-enable");

    // setup box filter kernel
    compute::kernel kernel(program, "box_filter");
    kernel.set_arg(0, input);
    kernel.set_arg(1, output);
    kernel.set_arg(2, box_height);
    kernel.set_arg(3, box_width);

    // execute the box filter kernel
    queue.enqueue_nd_range_kernel(kernel, dim(0, 0), input.size(), dim(0, 0));
}

// this example shows how to load an image using Qt, apply a simple
// box blur filter, and then display it in a Qt window.
void halfCL()
{
    size_t width = 1024;
    size_t height = 1024;

    std::cout << "height:" << height << std::endl
              << "width:" << width << std::endl;

    // create compute context
    compute::device gpu = compute::system::find_device();
    compute::context context(gpu);
    compute::command_queue queue(context, gpu);
    std::cout << "\nHALF\n====\ndevice: " << gpu.name()
              << "\nversion: " << gpu.version() << std::endl;

    // get the opencl image format for the qimage
    // CL_HALF_FLOAT: CL_RGBA, CL_RG, CL_R
    compute::image_format format = compute::image_format(compute::image_format::r, compute::image_format::float16);

    // create input and output images on the gpu
    compute::image2d input_image(context, width, height, format);
    compute::image2d output_image(context, width, height, format);

    const size_t origin[3] = { 0, 0, 0 };
    const size_t region[3] = { width, height, 1 };
    size_t row_pitch = 0;

    cl_map_flags write_flags = compute::command_queue::map_write;
    void *pRawIn = queue.enqueue_map_image(input_image,
                                           write_flags,
                                           origin,
                                           region,
                                           &row_pitch);

    half *pImageIn = static_cast<half*>(pRawIn);
    for(size_t h = 0; h < height; ++h) {
        for(size_t w = 0; w < width; ++w) {
            half & hLuminance = pImageIn[h*width + w];
            float fLuminance = sinf(sqrtf((h*h+w*w)/100.f));
            hLuminance = fLuminance;
        }
    }
    queue.enqueue_unmap_buffer(input_image, pRawIn);

    // apply box filter
    box_filter_image(input_image, output_image, 7, 7, queue);

    // copy gpu blurred image from to host qimage
    void *pRawOut = queue.enqueue_map_image(output_image,
                                            compute::command_queue::map_read,
                                            origin,
                                            region,
                                            &row_pitch);
    half *pImageOut = static_cast<half*>(pRawOut);
    float fError = 0.f;
    for(size_t h = 0; h < height; ++h) {
        for(size_t w = 0; w < width; ++w) {
            half & hLuminance = pImageOut[h*width + w];
            float fStored = hLuminance;
            float fLuminance = sinf(sqrtf((h*h+w*w)/100.f));
            float fDiff = fStored - fLuminance - 1.1f;
            fError += fDiff;
        }
    }
    queue.enqueue_unmap_buffer(output_image, pRawOut);
    std::cout << "Error = " << fError << std::endl;
}

void BOOST_COMPUTE_CL_CALLBACK func(void *pmem, size_t x, size_t y, size_t z)
{
    half & h = *(half *)pmem;
    float f = h;
    f += 0.0f;
    h = f;
    std::cout << "x=" << x << " y=" << y << " :" << f << "\n";
    (void)x; (void)y; (void)z;
}

void bulb()
{
    compute::device gpu = compute::system::find_device(std::string());
    compute::context context(gpu);
    compute::command_queue queue(context, gpu);

    djg::Bulb bulb;

    bulb.init(queue, compute::image2d(), 4, 4, 4);
    compute::half4_ hlf4(1.f,1.f,1.f,1.f);

    queue.enqueue_rawfill_image_walking(bulb.input_image(),
                                        &hlf4,
                                        bulb.input_image().origin(),
                                        bulb.input_image().size());
    bulb.clear_slices();
//    bulb.fill_slices(compute::float4_(1,1,1,1), // memory
//                     compute::float4_(1,1,1,0.25f), // weights ( [-1..1])
//                     compute::int2_(0,0)); // offsets
    bulb.execute_kernel_1();

//    auto func = [=](void *pmem, size_t x, size_t y, size_t z)
//    {
//        half & h = *(half *)pmem;
//        float f = h;
//        f += 0.0f;
//        h = f;
//        (void)x; (void)y; (void)z;
//    };

    queue.enqueue_walk_image(bulb.input_image(), func );
    std::cout << "\n";
    queue.enqueue_walk_image(bulb.memory_images()[0], func );
}

int main()
{
    std::vector<compute::platform> platforms = compute::system::platforms();

    for(size_t i = 0; i < platforms.size(); i++){
        const compute::platform &platform = platforms[i];

        std::cout << "Platform '" << platform.name() << "'" << std::endl
                  << "  Version '" << platform.version() << "'" << std::endl
                  << "  Vendor '" << platform.vendor() << "'" << std::endl
                  << "  Profile '" << platform.profile() << "'" << std::endl;

        std::vector<compute::device> devices = platform.devices();
        for(size_t j = 0; j < devices.size(); j++){
            const compute::device &device = devices[j];

            std::string type;
            if(device.type() & compute::device::gpu)
                type = "GPU Device";
            else if(device.type() & compute::device::cpu)
                type = "CPU Device";
            else if(device.type() & compute::device::accelerator)
                type = "Accelerator Device";
#ifdef CL_DEVICE_TYPE_CUSTOM
            else if(device.get_type() & compute::device::custom)
                type = "Custom Device";
#endif
            else
                type = "Unknown Device";

            std::cout
                << "  " << type << ": " << device.name() << std::endl
                << "  vendor: " << device.vendor() << std::endl
                << "  profile: " << device.profile() << std::endl
                << "  version: " << device.version() << std::endl
                << "  get_version: " << device.get_version() << std::endl
                << "  driver_version: " << device.driver_version() << std::endl
                << "  address_bits: " << device.address_bits() << std::endl
                << "  global_memory_size: " << device.global_memory_size()/(1024*1024) << " MiB" << std::endl
                << "  local_memory_size: " << device.local_memory_size()/1024 << " kiB" << std::endl
                << "  clock_frequency: " << device.clock_frequency() << " MHz" << std::endl
                << "  compute_units: " << device.compute_units() << std::endl
                << "  max_memory_alloc_size: " << device.max_memory_alloc_size()/(1024*1024) << " MiB" << std::endl
                << "  max_work_group_size: " << device.max_work_group_size() << std::endl
                << "  max_work_item_dimensions: " << device.max_work_item_dimensions() << std::endl
                << "  profiling_timer_resolution: " << device.profiling_timer_resolution() << std::endl
                << "  is_subdevice: " << device.is_subdevice() << std::endl
                << "  extensions: " << device.get_info<std::string>(CL_DEVICE_EXTENSIONS) << std::endl;
        }
        std::cout << std::endl;
    }

    try {
        //halfCL();
        bulb();
    } catch(boost::exception const&  bex) {
        std::string str = boost::diagnostic_information(bex);
        std::cerr << "Boost Exception: " << str;
    } catch(std::exception const&  ex) {
        std::cerr << "Std Exception: " << ex.what();
    }
    return 0;
}
