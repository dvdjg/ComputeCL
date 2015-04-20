#ifndef TYPES_HPP
#define TYPES_HPP

#include <boost/compute/types/fundamental.hpp>
#include "half.h"

namespace boost {
namespace compute {

// Do not use the ::cl_half OpenCL definition because of it creates a short alias.
typedef half cl_half;

BOOST_COMPUTE_DECLARE_VECTOR_TYPES(half);

}
}

#endif // TYPES_HPP

