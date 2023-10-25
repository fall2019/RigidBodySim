cmake_minimum_required(VERSION 3.12)

project(RigidBodySim)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

aux_source_directory(./src CPP_LIST)

set(VULKAN_ABSTRACTION_LAYER_BUILD_EXAMPLES OFF CACHE BOOL "build examples")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/val)

add_executable(RigidBodySim ${CPP_LIST})

target_link_directories(RigidBodySim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(RigidBodySim PUBLIC VulkanAbstractionLayer)

target_include_directories(RigidBodySim PUBLIC ${VULKAN_ABSTRACTION_LAYER_INCLUDE_DIR})
 
target_compile_definitions(RigidBodySim PUBLIC -D APPLICATION_WORKING_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}")