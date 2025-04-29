cmake_minimum_required(VERSION 3.16)

project(muli LANGUAGES C CXX VERSION 0.1.0)

set(HEADER_FILES
    include/muli/aabb.h
    include/muli/aabb_tree.h
    include/muli/broad_phase.h
    include/muli/contact_manager.h

    include/muli/collision.h
    include/muli/simplex.h
    include/muli/polytope.h
    include/muli/primitives.h
    include/muli/collision_filter.h
    include/muli/distance.h
    include/muli/raycast.h
    include/muli/time_of_impact.h

    include/muli/rigidbody.h

    include/muli/shape.h
    include/muli/circle.h
    include/muli/capsule.h
    include/muli/polygon.h

    include/muli/collider.h
    include/muli/material.h

    include/muli/constraint.h

    include/muli/joint.h
    include/muli/grab_joint.h
    include/muli/revolute_joint.h
    include/muli/distance_joint.h
    include/muli/angle_joint.h
    include/muli/weld_joint.h
    include/muli/line_joint.h
    include/muli/prismatic_joint.h
    include/muli/pulley_joint.h
    include/muli/motor_joint.h

    include/muli/contact.h
    include/muli/position_solver.h
    include/muli/contact_solver.h

    include/muli/island.h
    include/muli/world.h

    include/muli/common.h
    include/muli/muli.h
    include/muli/settings.h
    include/muli/growable_array.h
    include/muli/allocator.h
    include/muli/stack_allocator.h
    include/muli/block_allocator.h
    include/muli/predefined_block_allocator.h
    include/muli/fixed_block_allocator.h
    include/muli/callbacks.h
    include/muli/geometry.h
    include/muli/math.h
    include/muli/types.h
    include/muli/random.h
    include/muli/hash.h
)

set(SOURCE_FILES
    src/util/math.cpp
    src/util/stack_allocator.cpp
    src/util/linear_allocator.cpp
    src/util/block_allocator.cpp
    src/util/predefined_block_allocator.cpp

    src/geometry/circle.cpp
    src/geometry/capsule.cpp
    src/geometry/polygon.cpp
    src/geometry/geometry.cpp
    src/geometry/raycast.cpp

    src/collision/aabb.cpp
    src/collision/aabb_tree.cpp
    src/collision/broad_phase.cpp
    src/collision/simplex.cpp
    src/collision/polytope.cpp
    src/collision/collision.cpp
    src/collision/distance.cpp
    src/collision/time_of_impact.cpp

    src/dynamics/world.cpp
    src/dynamics/collider.cpp
    src/dynamics/rigidbody.cpp
    src/dynamics/island.cpp
    src/dynamics/contact_manager.cpp

    src/dynamics/constraint/constraint.cpp
    src/dynamics/constraint/contact/contact.cpp
    src/dynamics/constraint/contact/contact_solver.cpp
    src/dynamics/constraint/contact/block_solver.cpp
    src/dynamics/constraint/contact/position_solver.cpp

    src/dynamics/constraint/joint/joint.cpp
    src/dynamics/constraint/joint/grab_joint.cpp
    src/dynamics/constraint/joint/revolute_joint.cpp
    src/dynamics/constraint/joint/distance_joint.cpp
    src/dynamics/constraint/joint/angle_joint.cpp
    src/dynamics/constraint/joint/weld_joint.cpp
    src/dynamics/constraint/joint/line_joint.cpp
    src/dynamics/constraint/joint/prismatic_joint.cpp
    src/dynamics/constraint/joint/pulley_joint.cpp
    src/dynamics/constraint/joint/motor_joint.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "src" FILES ${SOURCE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "include" FILES ${HEADER_FILES})

add_library(muli ${SOURCE_FILES} ${HEADER_FILES})

target_precompile_headers(muli
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/muli/common.h
)

target_include_directories(muli
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(muli PROPERTIES
    CMAKE_COMPILE_WARNING_AS_ERROR ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

if(MSVC)
    target_compile_options(muli PRIVATE /W4 /WX /wd4819)
else()
    target_compile_options(muli PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

install(
    TARGETS muli
    EXPORT muliConfig
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT muliConfig
    NAMESPACE muli::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/muli
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/muliConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/muliConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/muli
)
