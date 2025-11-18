cmake_minimum_required(VERSION 3.26)

project(nfrrconfig
    VERSION 0.1.0
    LANGUAGES CXX
)

# Option to choose between -std=c++23 and -std=gnu++23
option(NFRRCONFIG_USE_GNU_EXTENSIONS
    "Use GNU++ dialect (-std=gnu++23) instead of -std=c++23"
    OFF
)

# Header-only library
add_library(nfrrconfig INTERFACE)

# Public include directories
target_include_directories(nfrrconfig
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Request C++23 via compile features (CMake maps this to -std=... for the compiler)
target_compile_features(nfrrconfig INTERFACE cxx_std_23)

# CXX_EXTENSIONS decides whether to use -std=gnu++23 or -std=c++23
if (NFRRCONFIG_USE_GNU_EXTENSIONS)
    set_property(TARGET nfrrconfig PROPERTY CXX_EXTENSIONS ON)
else()
    set_property(TARGET nfrrconfig PROPERTY CXX_EXTENSIONS OFF)
endif()

# Warnings and optimizations for GCC/Clang
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(nfrrconfig
        INTERFACE
            -Wall -Wextra -pedantic
            $<$<CONFIG:Release>:-O3 -march=native -mtune=native -flto=auto>
    )

    # LTO also on link step (-flto=auto lets the compiler decide how to apply LTO)
    target_link_options(nfrrconfig
        INTERFACE
            $<$<CONFIG:Release>:-flto=auto>
    )
endif()

# ------------------------------------------------------------------------------
# Example executable (your main.cpp)
# ------------------------------------------------------------------------------

add_executable(test_configmap
    examples/main.cpp
)

target_link_libraries(test_configmap
    PRIVATE
        nfrrconfig
)

# ------------------------------------------------------------------------------
# Testing with CTest
# ------------------------------------------------------------------------------

include(CTest)   # defines BUILD_TESTING and configures testing infrastructure

if (BUILD_TESTING)
    add_executable(configmap_tests
        tests/test_configmap.cpp
    )

    target_link_libraries(configmap_tests
        PRIVATE
            nfrrconfig
    )

    # Register both the example and the unit tests as CTest tests
    add_test(NAME configmap_example
             COMMAND test_configmap)

    add_test(NAME configmap_unit_tests
             COMMAND configmap_tests)
endif()
