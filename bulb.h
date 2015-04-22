#ifndef BULB_H
#define BULB_H

#include <vector>
#include <boost/compute/command_queue.hpp>
#include <boost/compute/image3d.hpp>
#include <functional>

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
              size_t slices = 8,
              compute::image2d input = compute::image2d());

#if defined(CL_VERSION_1_2) || defined(BOOST_COMPUTE_DOXYGEN_INVOKED)
    void fill_slices(compute::float4_ mem_fill = compute::float4_(0,0,0,0),
                     compute::float4_ wei_fill = compute::float4_(1,1,1,1),
                     compute::int2_ off_fill = compute::int2_(0,0),
                     const compute::wait_list &events = compute::wait_list(),
                     compute::wait_list * event_list = NULL);
#endif
    void clear_slices(const compute::wait_list &events = compute::wait_list(),
                      compute::wait_list * event_list = NULL);

//    void walk_memory_slice(size_t slice,
//                       std::function<void(void *, size_t, size_t)> f,
//                       cl_map_flags flags = compute::command_queue::map_read,
//                       const compute::wait_list &events = compute::wait_list(),
//                       compute::event * event = NULL);
//    void walk_memory_slice(size_t slice,
//                       std::function<void(void *pmem, size_t x, size_t y, size_t z)> f,
//                       cl_map_flags flags = compute::command_queue::map_read,
//                       const compute::wait_list &events = compute::wait_list(),
//                       compute::event * event = NULL)
//    {

//    }

    compute::extents<2> size() const { return m_memory.front().size(); }
    size_t slices() const { return m_memory.size(); }

    static size_t bytes_per_pixel(nchanels nc);
    void execute_kernel_1(size_t slice,
                          const compute::wait_list &events = compute::wait_list(),
                          compute::event * event = NULL);
    void execute_kernel_1(const compute::wait_list &in_events = compute::wait_list(),
                          compute::wait_list *out_events = NULL);

    compute::image2d & input_image() { return m_imageIn; }
    const compute::image2d & input_image() const { return m_imageIn; }

    std::vector<compute::image2d> & memory_images() { return m_memory; }
    const std::vector<compute::image2d> & memory_images() const { return m_memory; }

    std::vector<compute::image2d> & weights_images() { return m_weight; }
    const std::vector<compute::image2d> & weights_images() const { return m_weight; }

    std::vector<compute::image2d> & k_images() { return m_k; }
    const std::vector<compute::image2d> & k_images() const { return m_k; }

    std::vector<compute::image2d> & u_images() { return m_u; }
    const std::vector<compute::image2d> & u_images() const { return m_u; }

    std::vector<compute::image2d> & offsets_images() { return m_offset; }
    const std::vector<compute::image2d> & offsets_images() const { return m_offset; }

    const compute::command_queue & get_command_queue() const { return m_queue; }
    const compute::context & get_context() const { return m_context; }

    static compute::image_format get_memory_format() { return compute::image_format(compute::image_format::r,    compute::image_format::float16); }

protected:
    compute::command_queue m_queue;
    compute::context m_context;
    compute::kernel m_kernel_1;

    compute::image2d m_imageIn, m_imageTempOut;
    std::vector<compute::image2d> m_memory;
    std::vector<compute::image2d> m_weight;
    std::vector<compute::image2d> m_k;
    std::vector<compute::image2d> m_u;
    std::vector<compute::image2d> m_offset;

    void make_kernels();
#if defined(CL_VERSION_1_2) || defined(BOOST_COMPUTE_DOXYGEN_INVOKED)
    void fill_slices_inner(const std::vector<compute::image2d> &slices,
                           const void *fill_color,
                           const compute::wait_list &events,
                           compute::wait_list *event_list);
#endif
    void rawfill_slices_inner(const std::vector<compute::image2d> &slices,
                              const void *fill_color,
                              const compute::wait_list &events,
                              compute::wait_list *event_list);

//    void walk_image(compute::image2d image,
//                       std::function<void(void *, size_t, size_t)> f,
//                       cl_map_flags flags = compute::command_queue::map_read,
//                       const compute::wait_list &events = compute::wait_list(),
//                       compute::event * event = NULL);
};
}

#endif // BULB_H
