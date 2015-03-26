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

#include <boost/compute/core.hpp>

namespace compute = boost::compute;

int main()
{
    std::vector<compute::platform> platforms = compute::system::platforms();

    for(size_t i = 0; i < platforms.size(); i++){
        const compute::platform &platform = platforms[i];

        std::cout << "Platform '" << platform.name() << "'" << std::endl;

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
            else
                type = "Unknown Device";

            std::cout << std::endl
                << "  " << type << ": " << device.name() << std::endl
                << "  vendor: " << device.vendor() << std::endl
                << "  profile: " << device.profile() << std::endl
                << "  version: " << device.version() << std::endl
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
    }

    return 0;
}
