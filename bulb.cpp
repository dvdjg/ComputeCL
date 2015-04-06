#include "bulb.h"
#include "half.h"

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
                size_t depth,
                size_t slices,
                nchanels nchanel)
{
    compute::image_format format = compute::image_format(nchanel, compute::image_format::float16);
    m_queue = queue;
    m_context = queue.get_context();

    for (size_t i = 0; i < slices; ++i) {
        m_images.push_back(compute::image3d(m_context, width, height, depth, format));
    }

}

void Bulb::reset_slice(size_t slice, compute::float4_ fill,
                       const compute::wait_list &events,
                       compute::event * event)
{
    half hfill[4]; //{0.1, 0.01, 0.001, 0.0001};
    hfill[0] = fill[0];
    hfill[1] = fill[1];
    hfill[2] = fill[2];
    hfill[3] = fill[3];
    compute::image3d image = m_images[slice];
    m_queue.enqueue_fill_image(image, hfill, image.origin(), image.size(), events, event);

}

void Bulb::read_slice(size_t slice, const compute::wait_list &events, compute::event *event)
{
    compute::image3d image = m_images[slice];
    size_t row_pitch = 0;
    size_t slice_pitch = 0;
    compute::extents<3> origin = image.origin();
    compute::extents<3> size = image.size();
    cl_map_flags flags = compute::command_queue::map_read;
    void *pRawIn = m_queue.enqueue_map_image(image,
                                             flags,
                                             origin,
                                             size,
                                             &row_pitch,
                                             &slice_pitch);

    half *pImageIn = static_cast<half*>(pRawIn);
    for(size_t h = 0; h < height; ++h) {
        for(size_t w = 0; w < width; ++w) {
            half & hLuminance = pImageIn[h*width + w];
            float fLuminance = sinf(sqrtf((h*h+w*w)/100.f));
            hLuminance = fLuminance;
        }
    }
    queue.enqueue_unmap_buffer(input_image, pRawIn);
}

}
