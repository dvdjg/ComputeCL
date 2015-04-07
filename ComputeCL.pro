TARGET = ComputeCL
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt rtti
CONFIG += c++11

CONFIG(debug, debug|release):SUFFIX=d
else:SUFFIX=

TARGET=$$TARGET$$SUFFIX

*-g++:QMAKE_TARGET.arch = $$QMAKE_HOST.arch
*-g++-32:QMAKE_TARGET.arch = x86
*-g++-64:QMAKE_TARGET.arch = x86_64

SPEC=$$basename(QMAKESPEC)
SPEC=$$replace(SPEC, win32, windows)-$$QMAKE_TARGET.arch

LIBDIR=$$PWD/../lib
staticlib {
    DEFINES += STATIC
    DESTDIR=$$LIBDIR
} else {
    # Binaries and dynamic libs goes to bin
    DESTDIR=$$PWD/../bin
    static:DESTDIR=$$DESTDIR-static
}

LIBDIR=$$LIBDIR/$$SPEC
DESTDIR=$$DESTDIR/$$SPEC

LIBS += "-L$$LIBDIR"

WD=$$PWD
win32:{
    INCLUDEPATH += "C:/Program Files (x86)/AMD APP SDK/2.9-1/include" "C:\Program Files (x86)\AMD APP SDK\3.0-0-Beta\include"
    INCLUDEPATH += C:/Programa/boost_1_57_0 "$$WD/../boost_1_57_0"
    LIBS += "-LC:/Programa/boost_1_57_0/stage/lib" "-L$$WD/../boost_1_57_0/stage/lib"

    message("arch=$$QMAKE_TARGET.arch")
    win32-*:contains(QMAKE_HOST.arch, x86_64) {
        LIBS += "-LC:/Program Files (x86)/AMD APP SDK/2.9-1/lib/x86_64" "-LC:/Program Files (x86)/AMD APP SDK/3.0-0-Beta/lib/x86_64"
    } else {
        LIBS += "-LC:/Program Files (x86)/AMD APP SDK/2.9-1/lib/x86" "-LC:/Program Files (x86)/AMD APP SDK/3.0-0-Beta/lib/x86"
    }
} else {
    INCLUDEPATH += /home/puesto/Programa/compute/include /opt/AMDAPPSDK-3.0-0-Beta/include
    LIBS += "-L/opt/AMDAPPSDK-3.0-0-Beta/lib/x86"

    linux-*:contains(QMAKE_TARGET.arch, x86_64) {
    } else {
    }
}

DEFINES += _SCL_SECURE_NO_WARNINGS CL_USE_DEPRECATED_OPENCL_1_0_APIS CL_USE_DEPRECATED_OPENCL_1_1_APIS CL_USE_DEPRECATED_OPENCL_2_0_APIS
INCLUDEPATH += "$$WD/../compute/include" "$$WD/../half"

LIBS += -lOpenCL -lhalf$$SUFFIX

SOURCES += main.cpp \
    bulb.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    ../compute/include/boost/compute.hpp \
    ../compute/include/boost/compute/algorithm/detail/balanced_path.hpp \
    ../compute/include/boost/compute/algorithm/detail/binary_find.hpp \
    ../compute/include/boost/compute/algorithm/detail/compact.hpp \
    ../compute/include/boost/compute/algorithm/detail/copy_on_device.hpp \
    ../compute/include/boost/compute/algorithm/detail/copy_to_device.hpp \
    ../compute/include/boost/compute/algorithm/detail/copy_to_host.hpp \
    ../compute/include/boost/compute/algorithm/detail/count_if_with_ballot.hpp \
    ../compute/include/boost/compute/algorithm/detail/count_if_with_reduce.hpp \
    ../compute/include/boost/compute/algorithm/detail/count_if_with_threads.hpp \
    ../compute/include/boost/compute/algorithm/detail/find_extrema.hpp \
    ../compute/include/boost/compute/algorithm/detail/find_extrema_with_atomics.hpp \
    ../compute/include/boost/compute/algorithm/detail/find_if_with_atomics.hpp \
    ../compute/include/boost/compute/algorithm/detail/inplace_reduce.hpp \
    ../compute/include/boost/compute/algorithm/detail/insertion_sort.hpp \
    ../compute/include/boost/compute/algorithm/detail/merge_path.hpp \
    ../compute/include/boost/compute/algorithm/detail/merge_with_merge_path.hpp \
    ../compute/include/boost/compute/algorithm/detail/radix_sort.hpp \
    ../compute/include/boost/compute/algorithm/detail/random_fill.hpp \
    ../compute/include/boost/compute/algorithm/detail/reduce_on_gpu.hpp \
    ../compute/include/boost/compute/algorithm/detail/scan.hpp \
    ../compute/include/boost/compute/algorithm/detail/scan_on_cpu.hpp \
    ../compute/include/boost/compute/algorithm/detail/scan_on_gpu.hpp \
    ../compute/include/boost/compute/algorithm/detail/search_all.hpp \
    ../compute/include/boost/compute/algorithm/detail/serial_accumulate.hpp \
    ../compute/include/boost/compute/algorithm/detail/serial_count_if.hpp \
    ../compute/include/boost/compute/algorithm/detail/serial_find_extrema.hpp \
    ../compute/include/boost/compute/algorithm/detail/serial_merge.hpp \
    ../compute/include/boost/compute/algorithm/detail/serial_reduce.hpp \
    ../compute/include/boost/compute/algorithm/accumulate.hpp \
    ../compute/include/boost/compute/algorithm/adjacent_difference.hpp \
    ../compute/include/boost/compute/algorithm/adjacent_find.hpp \
    ../compute/include/boost/compute/algorithm/all_of.hpp \
    ../compute/include/boost/compute/algorithm/any_of.hpp \
    ../compute/include/boost/compute/algorithm/binary_search.hpp \
    ../compute/include/boost/compute/algorithm/copy.hpp \
    ../compute/include/boost/compute/algorithm/copy_if.hpp \
    ../compute/include/boost/compute/algorithm/copy_n.hpp \
    ../compute/include/boost/compute/algorithm/count.hpp \
    ../compute/include/boost/compute/algorithm/count_if.hpp \
    ../compute/include/boost/compute/algorithm/equal.hpp \
    ../compute/include/boost/compute/algorithm/equal_range.hpp \
    ../compute/include/boost/compute/algorithm/exclusive_scan.hpp \
    ../compute/include/boost/compute/algorithm/fill.hpp \
    ../compute/include/boost/compute/algorithm/fill_n.hpp \
    ../compute/include/boost/compute/algorithm/find.hpp \
    ../compute/include/boost/compute/algorithm/find_end.hpp \
    ../compute/include/boost/compute/algorithm/find_if.hpp \
    ../compute/include/boost/compute/algorithm/find_if_not.hpp \
    ../compute/include/boost/compute/algorithm/for_each.hpp \
    ../compute/include/boost/compute/algorithm/for_each_n.hpp \
    ../compute/include/boost/compute/algorithm/gather.hpp \
    ../compute/include/boost/compute/algorithm/generate.hpp \
    ../compute/include/boost/compute/algorithm/generate_n.hpp \
    ../compute/include/boost/compute/algorithm/includes.hpp \
    ../compute/include/boost/compute/algorithm/inclusive_scan.hpp \
    ../compute/include/boost/compute/algorithm/inner_product.hpp \
    ../compute/include/boost/compute/algorithm/inplace_merge.hpp \
    ../compute/include/boost/compute/algorithm/iota.hpp \
    ../compute/include/boost/compute/algorithm/is_partitioned.hpp \
    ../compute/include/boost/compute/algorithm/is_permutation.hpp \
    ../compute/include/boost/compute/algorithm/is_sorted.hpp \
    ../compute/include/boost/compute/algorithm/lexicographical_compare.hpp \
    ../compute/include/boost/compute/algorithm/lower_bound.hpp \
    ../compute/include/boost/compute/algorithm/max_element.hpp \
    ../compute/include/boost/compute/algorithm/merge.hpp \
    ../compute/include/boost/compute/algorithm/min_element.hpp \
    ../compute/include/boost/compute/algorithm/minmax_element.hpp \
    ../compute/include/boost/compute/algorithm/mismatch.hpp \
    ../compute/include/boost/compute/algorithm/next_permutation.hpp \
    ../compute/include/boost/compute/algorithm/none_of.hpp \
    ../compute/include/boost/compute/algorithm/nth_element.hpp \
    ../compute/include/boost/compute/algorithm/partial_sum.hpp \
    ../compute/include/boost/compute/algorithm/partition.hpp \
    ../compute/include/boost/compute/algorithm/partition_copy.hpp \
    ../compute/include/boost/compute/algorithm/partition_point.hpp \
    ../compute/include/boost/compute/algorithm/prev_permutation.hpp \
    ../compute/include/boost/compute/algorithm/random_shuffle.hpp \
    ../compute/include/boost/compute/algorithm/reduce.hpp \
    ../compute/include/boost/compute/algorithm/remove.hpp \
    ../compute/include/boost/compute/algorithm/remove_if.hpp \
    ../compute/include/boost/compute/algorithm/replace.hpp \
    ../compute/include/boost/compute/algorithm/replace_copy.hpp \
    ../compute/include/boost/compute/algorithm/reverse.hpp \
    ../compute/include/boost/compute/algorithm/reverse_copy.hpp \
    ../compute/include/boost/compute/algorithm/rotate.hpp \
    ../compute/include/boost/compute/algorithm/rotate_copy.hpp \
    ../compute/include/boost/compute/algorithm/scatter.hpp \
    ../compute/include/boost/compute/algorithm/search.hpp \
    ../compute/include/boost/compute/algorithm/search_n.hpp \
    ../compute/include/boost/compute/algorithm/set_difference.hpp \
    ../compute/include/boost/compute/algorithm/set_intersection.hpp \
    ../compute/include/boost/compute/algorithm/set_symmetric_difference.hpp \
    ../compute/include/boost/compute/algorithm/set_union.hpp \
    ../compute/include/boost/compute/algorithm/sort.hpp \
    ../compute/include/boost/compute/algorithm/sort_by_key.hpp \
    ../compute/include/boost/compute/algorithm/stable_partition.hpp \
    ../compute/include/boost/compute/algorithm/stable_sort.hpp \
    ../compute/include/boost/compute/algorithm/swap_ranges.hpp \
    ../compute/include/boost/compute/algorithm/transform.hpp \
    ../compute/include/boost/compute/algorithm/transform_reduce.hpp \
    ../compute/include/boost/compute/algorithm/unique.hpp \
    ../compute/include/boost/compute/algorithm/unique_copy.hpp \
    ../compute/include/boost/compute/algorithm/upper_bound.hpp \
    ../compute/include/boost/compute/allocator/buffer_allocator.hpp \
    ../compute/include/boost/compute/allocator/pinned_allocator.hpp \
    ../compute/include/boost/compute/async/future.hpp \
    ../compute/include/boost/compute/async/wait.hpp \
    ../compute/include/boost/compute/async/wait_guard.hpp \
    ../compute/include/boost/compute/container/detail/scalar.hpp \
    ../compute/include/boost/compute/container/array.hpp \
    ../compute/include/boost/compute/container/basic_string.hpp \
    ../compute/include/boost/compute/container/dynamic_bitset.hpp \
    ../compute/include/boost/compute/container/flat_map.hpp \
    ../compute/include/boost/compute/container/flat_set.hpp \
    ../compute/include/boost/compute/container/mapped_view.hpp \
    ../compute/include/boost/compute/container/stack.hpp \
    ../compute/include/boost/compute/container/string.hpp \
    ../compute/include/boost/compute/container/valarray.hpp \
    ../compute/include/boost/compute/container/vector.hpp \
    ../compute/include/boost/compute/detail/assert_cl_success.hpp \
    ../compute/include/boost/compute/detail/buffer_value.hpp \
    ../compute/include/boost/compute/detail/device_ptr.hpp \
    ../compute/include/boost/compute/detail/duration.hpp \
    ../compute/include/boost/compute/detail/get_object_info.hpp \
    ../compute/include/boost/compute/detail/getenv.hpp \
    ../compute/include/boost/compute/detail/global_static.hpp \
    ../compute/include/boost/compute/detail/is_buffer_iterator.hpp \
    ../compute/include/boost/compute/detail/is_contiguous_iterator.hpp \
    ../compute/include/boost/compute/detail/iterator_plus_distance.hpp \
    ../compute/include/boost/compute/detail/iterator_range_size.hpp \
    ../compute/include/boost/compute/detail/iterator_traits.hpp \
    ../compute/include/boost/compute/detail/literal.hpp \
    ../compute/include/boost/compute/detail/lru_cache.hpp \
    ../compute/include/boost/compute/detail/meta_kernel.hpp \
    ../compute/include/boost/compute/detail/mpl_vector_to_tuple.hpp \
    ../compute/include/boost/compute/detail/nvidia_compute_capability.hpp \
    ../compute/include/boost/compute/detail/print_range.hpp \
    ../compute/include/boost/compute/detail/read_write_single_value.hpp \
    ../compute/include/boost/compute/detail/sha1.hpp \
    ../compute/include/boost/compute/detail/variadic_macros.hpp \
    ../compute/include/boost/compute/detail/vendor.hpp \
    ../compute/include/boost/compute/detail/work_size.hpp \
    ../compute/include/boost/compute/exception/context_error.hpp \
    ../compute/include/boost/compute/exception/no_device_found.hpp \
    ../compute/include/boost/compute/exception/opencl_error.hpp \
    ../compute/include/boost/compute/exception/unsupported_extension_error.hpp \
    ../compute/include/boost/compute/experimental/clamp_range.hpp \
    ../compute/include/boost/compute/experimental/malloc.hpp \
    ../compute/include/boost/compute/experimental/sort_by_transform.hpp \
    ../compute/include/boost/compute/experimental/tabulate.hpp \
    ../compute/include/boost/compute/experimental/transform_if.hpp \
    ../compute/include/boost/compute/functional/detail/macros.hpp \
    ../compute/include/boost/compute/functional/detail/nvidia_ballot.hpp \
    ../compute/include/boost/compute/functional/detail/nvidia_popcount.hpp \
    ../compute/include/boost/compute/functional/detail/unpack.hpp \
    ../compute/include/boost/compute/functional/as.hpp \
    ../compute/include/boost/compute/functional/atomic.hpp \
    ../compute/include/boost/compute/functional/bind.hpp \
    ../compute/include/boost/compute/functional/common.hpp \
    ../compute/include/boost/compute/functional/convert.hpp \
    ../compute/include/boost/compute/functional/field.hpp \
    ../compute/include/boost/compute/functional/geometry.hpp \
    ../compute/include/boost/compute/functional/get.hpp \
    ../compute/include/boost/compute/functional/hash.hpp \
    ../compute/include/boost/compute/functional/identity.hpp \
    ../compute/include/boost/compute/functional/integer.hpp \
    ../compute/include/boost/compute/functional/logical.hpp \
    ../compute/include/boost/compute/functional/math.hpp \
    ../compute/include/boost/compute/functional/operator.hpp \
    ../compute/include/boost/compute/functional/popcount.hpp \
    ../compute/include/boost/compute/functional/relational.hpp \
    ../compute/include/boost/compute/image/image1d.hpp \
    ../compute/include/boost/compute/image/image2d.hpp \
    ../compute/include/boost/compute/image/image3d.hpp \
    ../compute/include/boost/compute/image/image_format.hpp \
    ../compute/include/boost/compute/image/image_object.hpp \
    ../compute/include/boost/compute/image/image_sampler.hpp \
    ../compute/include/boost/compute/interop/eigen/core.hpp \
    ../compute/include/boost/compute/interop/opencv/core.hpp \
    ../compute/include/boost/compute/interop/opencv/highgui.hpp \
    ../compute/include/boost/compute/interop/opencv/ocl.hpp \
    ../compute/include/boost/compute/interop/opengl/acquire.hpp \
    ../compute/include/boost/compute/interop/opengl/cl_gl.hpp \
    ../compute/include/boost/compute/interop/opengl/cl_gl_ext.hpp \
    ../compute/include/boost/compute/interop/opengl/context.hpp \
    ../compute/include/boost/compute/interop/opengl/gl.hpp \
    ../compute/include/boost/compute/interop/opengl/opengl_buffer.hpp \
    ../compute/include/boost/compute/interop/opengl/opengl_renderbuffer.hpp \
    ../compute/include/boost/compute/interop/opengl/opengl_texture.hpp \
    ../compute/include/boost/compute/interop/qt/qimage.hpp \
    ../compute/include/boost/compute/interop/qt/qpoint.hpp \
    ../compute/include/boost/compute/interop/qt/qpointf.hpp \
    ../compute/include/boost/compute/interop/qt/qtcore.hpp \
    ../compute/include/boost/compute/interop/qt/qtgui.hpp \
    ../compute/include/boost/compute/interop/qt/qvector.hpp \
    ../compute/include/boost/compute/interop/vtk/bounds.hpp \
    ../compute/include/boost/compute/interop/vtk/data_array.hpp \
    ../compute/include/boost/compute/interop/vtk/matrix4x4.hpp \
    ../compute/include/boost/compute/interop/vtk/points.hpp \
    ../compute/include/boost/compute/interop/eigen.hpp \
    ../compute/include/boost/compute/interop/opencv.hpp \
    ../compute/include/boost/compute/interop/opengl.hpp \
    ../compute/include/boost/compute/interop/qt.hpp \
    ../compute/include/boost/compute/interop/vtk.hpp \
    ../compute/include/boost/compute/iterator/detail/get_base_iterator_buffer.hpp \
    ../compute/include/boost/compute/iterator/detail/swizzle_iterator.hpp \
    ../compute/include/boost/compute/iterator/buffer_iterator.hpp \
    ../compute/include/boost/compute/iterator/constant_buffer_iterator.hpp \
    ../compute/include/boost/compute/iterator/constant_iterator.hpp \
    ../compute/include/boost/compute/iterator/counting_iterator.hpp \
    ../compute/include/boost/compute/iterator/discard_iterator.hpp \
    ../compute/include/boost/compute/iterator/function_input_iterator.hpp \
    ../compute/include/boost/compute/iterator/permutation_iterator.hpp \
    ../compute/include/boost/compute/iterator/transform_iterator.hpp \
    ../compute/include/boost/compute/iterator/zip_iterator.hpp \
    ../compute/include/boost/compute/lambda/context.hpp \
    ../compute/include/boost/compute/lambda/functional.hpp \
    ../compute/include/boost/compute/lambda/get.hpp \
    ../compute/include/boost/compute/lambda/make_pair.hpp \
    ../compute/include/boost/compute/lambda/make_tuple.hpp \
    ../compute/include/boost/compute/lambda/placeholder.hpp \
    ../compute/include/boost/compute/lambda/placeholders.hpp \
    ../compute/include/boost/compute/lambda/result_of.hpp \
    ../compute/include/boost/compute/memory/local_buffer.hpp \
    ../compute/include/boost/compute/memory/svm_ptr.hpp \
    ../compute/include/boost/compute/random/bernoulli_distribution.hpp \
    ../compute/include/boost/compute/random/default_random_engine.hpp \
    ../compute/include/boost/compute/random/discrete_distribution.hpp \
    ../compute/include/boost/compute/random/linear_congruential_engine.hpp \
    ../compute/include/boost/compute/random/mersenne_twister_engine.hpp \
    ../compute/include/boost/compute/random/normal_distribution.hpp \
    ../compute/include/boost/compute/random/uniform_int_distribution.hpp \
    ../compute/include/boost/compute/random/uniform_real_distribution.hpp \
    ../compute/include/boost/compute/type_traits/detail/capture_traits.hpp \
    ../compute/include/boost/compute/type_traits/common_type.hpp \
    ../compute/include/boost/compute/type_traits/is_device_iterator.hpp \
    ../compute/include/boost/compute/type_traits/is_fundamental.hpp \
    ../compute/include/boost/compute/type_traits/is_vector_type.hpp \
    ../compute/include/boost/compute/type_traits/make_vector_type.hpp \
    ../compute/include/boost/compute/type_traits/result_of.hpp \
    ../compute/include/boost/compute/type_traits/scalar_type.hpp \
    ../compute/include/boost/compute/type_traits/type_definition.hpp \
    ../compute/include/boost/compute/type_traits/type_name.hpp \
    ../compute/include/boost/compute/type_traits/vector_size.hpp \
    ../compute/include/boost/compute/types/builtin.hpp \
    ../compute/include/boost/compute/types/complex.hpp \
    ../compute/include/boost/compute/types/fundamental.hpp \
    ../compute/include/boost/compute/types/pair.hpp \
    ../compute/include/boost/compute/types/struct.hpp \
    ../compute/include/boost/compute/types/tuple.hpp \
    ../compute/include/boost/compute/utility/dim.hpp \
    ../compute/include/boost/compute/utility/extents.hpp \
    ../compute/include/boost/compute/utility/program_cache.hpp \
    ../compute/include/boost/compute/utility/source.hpp \
    ../compute/include/boost/compute/utility/wait_list.hpp \
    ../compute/include/boost/compute/algorithm.hpp \
    ../compute/include/boost/compute/allocator.hpp \
    ../compute/include/boost/compute/async.hpp \
    ../compute/include/boost/compute/buffer.hpp \
    ../compute/include/boost/compute/cl.hpp \
    ../compute/include/boost/compute/cl_ext.hpp \
    ../compute/include/boost/compute/closure.hpp \
    ../compute/include/boost/compute/command_queue.hpp \
    ../compute/include/boost/compute/config.hpp \
    ../compute/include/boost/compute/container.hpp \
    ../compute/include/boost/compute/context.hpp \
    ../compute/include/boost/compute/core.hpp \
    ../compute/include/boost/compute/device.hpp \
    ../compute/include/boost/compute/event.hpp \
    ../compute/include/boost/compute/exception.hpp \
    ../compute/include/boost/compute/function.hpp \
    ../compute/include/boost/compute/functional.hpp \
    ../compute/include/boost/compute/image.hpp \
    ../compute/include/boost/compute/image2d.hpp \
    ../compute/include/boost/compute/image3d.hpp \
    ../compute/include/boost/compute/image_format.hpp \
    ../compute/include/boost/compute/image_sampler.hpp \
    ../compute/include/boost/compute/iterator.hpp \
    ../compute/include/boost/compute/kernel.hpp \
    ../compute/include/boost/compute/lambda.hpp \
    ../compute/include/boost/compute/memory.hpp \
    ../compute/include/boost/compute/memory_object.hpp \
    ../compute/include/boost/compute/pipe.hpp \
    ../compute/include/boost/compute/platform.hpp \
    ../compute/include/boost/compute/program.hpp \
    ../compute/include/boost/compute/random.hpp \
    ../compute/include/boost/compute/source.hpp \
    ../compute/include/boost/compute/svm.hpp \
    ../compute/include/boost/compute/system.hpp \
    ../compute/include/boost/compute/type_traits.hpp \
    ../compute/include/boost/compute/types.hpp \
    ../compute/include/boost/compute/user_event.hpp \
    ../compute/include/boost/compute/utility.hpp \
    ../compute/include/boost/compute/version.hpp \
    ../compute/include/boost/compute/wait_list.hpp \
    bulb.h \
    types.hpp


