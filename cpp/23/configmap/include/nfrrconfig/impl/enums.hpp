#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <cstdint>

namespace nfrr::config {
// Enum describing the high-level kind of the stored value.
enum class ConfigValueKind : std::uint8_t { Null, Boolean, Integer, Floating, String, Array, Object };

/**
 * @brief Error codes used by safe value accessors such as try_get().
 */
enum class ConfigError : std::uint8_t {
    None,           ///< No error (internal use only).
    TypeMismatch,   ///< Value kind is not compatible with requested type.
    OutOfRange,     ///< Numeric conversion would overflow or underflow.
    FractionalLoss, ///< Floating-to-integer conversion would lose fraction.
    ParseError,     ///< String-to-number conversion failed.
    KeyNotFound     ///< Requested key does not exist (for object access).
};
} // namespace nfrr::config

#endif // ENUMS_HPP
