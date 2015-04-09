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
    void init(compute::command_queue & queue, compute::image2d input,
              size_t width = 256,
              size_t height = 256,
              size_t slices = 8);

    void fill_slices(compute::float4_ mem_fill = compute::float4_(0,0,0,0),
                     compute::float4_ wei_fill = compute::float4_(1,1,1,1),
                     compute::int2_ off_fill = compute::int2_(0,0),
                     const compute::wait_list &events = compute::wait_list(),
                     compute::wait_list * event_list = NULL);

    void read_slice(size_t slice = 0,
                    const compute::wait_list &events = compute::wait_list(),
                    compute::event * event = NULL);

    compute::extents<2> size() const { return m_memory.front().size(); }
    size_t slices() const { return m_memory.size(); }

    static size_t bytes_per_pixel(nchanels nc);
    void execute_kernel_1(size_t slice = 0,
                          const compute::wait_list &events = compute::wait_list(),
                          compute::event * event = NULL);
protected:
    compute::command_queue m_queue;
    compute::context m_context;
    compute::kernel m_kernel_1;

    std::vector<compute::image2d> m_memory;
    std::vector<compute::image2d> m_weights;
    std::vector<compute::image2d> m_offsets;
    void make_kernels();
    void fill_slices_inner(const std::vector<compute::image2d> &slices,
                           const void *fill_color,
                           const compute::wait_list &events,
                           compute::wait_list *event_list);
};
}

#endif // BULB_H
