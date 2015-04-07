#include "bulb.h"
#include "half.h"
#include <boost/compute/user_event.hpp>

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

void Bulb::fill_slice(size_t slice, compute::float4_ fill,
                       const compute::wait_list &events,
                       compute::event * event)
{
    //half hfill[4] {fill[0], fill[1], fill[2], fill[3]};

    compute::image3d image = m_images[slice];
    m_queue.enqueue_fill_image(image, &fill, image.origin(), image.size(), events, event);

}

void Bulb::read_slice(size_t slice, const compute::wait_list &events, compute::event *event)
{
    compute::image3d image = m_images[slice];
    size_t row_pitch = 0;
    size_t slice_pitch = 0;
    const compute::extents<3> origin = image.origin();
    const compute::extents<3> size = image.size();
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

    char * const pImage = reinterpret_cast<char *>(
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
        char * pImage2D = pImage;
        for(size_t d = origin[2]; d < size[2]; ++d) {
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
            pImage2D += slice_pitch;
        }
        if(puser_event) {
            puser_event->set_status(compute::event::complete);
        }
    };

    if (event) {
        // Async exec
        compute::event::execution_status status = pmap_event->get_status();
        pmap_event->set_callback(func);
    } else {
        func();
    }
    m_queue.enqueue_unmap_buffer(image, pImage, unmap_wait, event);
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

}
