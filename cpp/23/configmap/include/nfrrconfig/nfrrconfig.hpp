#ifndef CONFIGMAP_HPP
#define CONFIGMAP_HPP

#include "impl/all_includes.hpp"
#include "impl/bcv_impl.hpp"

namespace nfrr {
namespace config {
    // 1) Version using std::allocator (heap via new/delete).
    using StdByteAllocator   = std::allocator<std::byte>;
    using ConfigValueStd     = BasicConfigValue<StdByteAllocator>;

    // 2) Version using pmr::polymorphic_allocator (memory_resource based).
    using PmrByteAllocator   = std::pmr::polymorphic_allocator<std::byte>;
    using ConfigValuePmr     = BasicConfigValue<PmrByteAllocator>;
}
}

#endif // CONFIGMAP_HPP
