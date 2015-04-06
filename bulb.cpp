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
    //compute::event map_event, *pEvent = event ? &map_event : NULL;
    char * const pImage = reinterpret_cast<char *>(m_queue.enqueue_map_image(
                image,
                flags,
                origin,
                size,
                &row_pitch,
                &slice_pitch,
                events,
                NULL));

//    compute::wait_list unmap_events;
//    if(pEvent) {
//        unmap_events.insert(*pEvent);
//    }

    size_t element_size = image.get_info<size_t>(CL_IMAGE_ELEMENT_SIZE);
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
    m_queue.enqueue_unmap_buffer(image, pImage, compute::wait_list(), event);
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
