#ifndef ALL_INCLUDES_HPP
#define ALL_INCLUDES_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <variant>
#include <memory>          // std::allocator, std::allocator_traits
#include <cstddef>         // std::byte
#include <memory_resource> // std::pmr::polymorphic_allocator
#include <type_traits>     // std::is_arithmetic_v, etc.
#include <expected>        // std::expected (C++23)
#include <charconv>        // std::from_chars
#include <limits>          // std::numeric_limits
#include <cmath>           // std::trunc, std::isfinite
#include <stdexcept>       // std::runtime_error
#include <algorithm>

namespace nfrr {
namespace config {
    // Forward declaration for allocator-aware config value.
    template<typename Alloc>
    class BasicConfigValue;
} // namespace config
} // namespace nfrr

#endif // ALL_INCLUDES_HPP
