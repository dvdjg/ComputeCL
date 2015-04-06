#ifndef BULB_H
#define BULB_H

#include <vector>
#include <boost/compute/command_queue.hpp>
#include <boost/compute/image3d.hpp>

namespace djg
{
namespace compute = boost::compute;

class Bulb
{
public:
    Bulb();
    ~Bulb();

    enum nchanels { R = compute::image_format::r, RG = compute::image_format::rg, RGBA = compute::image_format::rgba };
    // CL_HALF_FLOAT: CL_RGBA, CL_RG, CL_R
    void init(compute::command_queue & queue,
              size_t width = 256,
              size_t height = 256,
              size_t depth = 256,
              size_t slices = 8,
              nchanels nchanel = R);

    void reset_slice(size_t slice = 0,
                     compute::float4_ fill = compute::float4_(0,0,0,0),
                     const compute::wait_list &events = compute::wait_list(),
                     compute::event * event = NULL);

    void read_slice(size_t slice = 0,
                     const compute::wait_list &events = compute::wait_list(),
                     compute::event * event = NULL);

    compute::extents<3> size() const { return m_images.front().size(); }
    size_t slices() const { return m_images.size(); }

    static size_t bytes_per_pixel(nchanels nc);
protected:
    compute::command_queue m_queue;
    compute::context m_context;

    std::vector<compute::image3d> m_images;
};
}

#endif // BULB_H
