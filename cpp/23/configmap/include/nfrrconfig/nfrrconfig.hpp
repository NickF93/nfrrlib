#ifndef CONFIGMAP_HPP
#define CONFIGMAP_HPP

#include <cstddef>
#include <memory>
#include <memory_resource>

#include "impl/basic_config_value.hpp"
#include "impl/bcv_impl.hpp"

namespace nfrr::config {
// 1) Version using std::allocator (heap via new/delete).
using StdByteAllocator = std::allocator<std::byte>;
using ConfigValueStd = BasicConfigValue<StdByteAllocator>;

// 2) Version using pmr::polymorphic_allocator (memory_resource based).
using PmrByteAllocator = std::pmr::polymorphic_allocator<std::byte>;
using ConfigValuePmr = BasicConfigValue<PmrByteAllocator>;
} // namespace nfrr::config

#endif // CONFIGMAP_HPP
