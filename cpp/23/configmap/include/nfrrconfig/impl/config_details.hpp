#ifndef CONFIGMAP_IMPL_CONFIG_DETAILS
#define CONFIGMAP_IMPL_CONFIG_DETAILS

#include "all_includes.hpp"
#include "enums.hpp"

namespace nfrr {
namespace config {

// Small namespace for implementation details.
namespace config_detail {

    /**
     * @brief Helper concept to detect arithmetic types (integral or floating).
     */
    template<typename T>
    concept Arithmetic = std::is_arithmetic_v<T>;

    /**
     * @brief Convert an int64_t to an arithmetic target type with range checks.
     *
     * @tparam T Target arithmetic type.
     * @param value Source 64-bit signed integer.
     * @return std::expected<T, ConfigError> Converted value or error.
     */
    template<Arithmetic T>
    inline std::expected<T, ConfigError>
    numeric_from_int64(std::int64_t value)
    {
        using Limits = std::numeric_limits<T>;

        if constexpr (std::is_same_v<T, bool>) {
            return static_cast<bool>(value != 0);
        } else if constexpr (std::is_floating_point_v<T>) {
            // Any int64_t can be represented as double, but float may lose precision.
            return static_cast<T>(value);
        } else if constexpr (std::is_signed_v<T>) {
            if (value < static_cast<std::int64_t>(Limits::min()) ||
                value > static_cast<std::int64_t>(Limits::max())) {
                return std::unexpected(ConfigError::OutOfRange);
            }
            return static_cast<T>(value);
        } else { // T is unsigned integral
            if (value < 0 ||
                static_cast<std::uint64_t>(value) >
                    static_cast<std::uint64_t>(Limits::max())) {
                return std::unexpected(ConfigError::OutOfRange);
            }
            return static_cast<T>(value);
        }
    }

    /**
     * @brief Convert a double to an arithmetic target type with range checks.
     *
     * For floating targets, we simply static_cast with a finite-range check.
     * For integral targets, we enforce that the value is finite, within range,
     * and has no fractional part (trunc(value) == value).
     */
    template<Arithmetic T>
    inline std::expected<T, ConfigError>
    numeric_from_double(double value)
    {
        if (!std::isfinite(value)) {
            return std::unexpected(ConfigError::OutOfRange);
        }

        using Limits = std::numeric_limits<T>;

        if constexpr (std::is_floating_point_v<T>) {
            if (value < static_cast<double>(Limits::lowest()) ||
                value > static_cast<double>(Limits::max())) {
                return std::unexpected(ConfigError::OutOfRange);
            }
            return static_cast<T>(value);
        } else {
            // Integer target: require no fractional part.
            double truncated = std::trunc(value);
            if (truncated != value) {
                return std::unexpected(ConfigError::FractionalLoss);
            }
            if (truncated < static_cast<double>(Limits::min()) ||
                truncated > static_cast<double>(Limits::max())) {
                return std::unexpected(ConfigError::OutOfRange);
            }
            return static_cast<T>(truncated);
        }
    }

    /**
     * @brief Convert a bool to an arithmetic target type.
     *
     * For integers, use 0/1; for floating types use 0.0/1.0.
     */
    template<Arithmetic T>
    inline std::expected<T, ConfigError>
    numeric_from_bool(bool value)
    {
        if constexpr (std::is_same_v<T, bool>) {
            return value;
        } else if constexpr (std::is_integral_v<T>) {
            return static_cast<T>(value ? 1 : 0);
        } else { // floating
            return static_cast<T>(value ? 1.0 : 0.0);
        }
    }

    /**
     * @brief Parse an arithmetic value from a string_view using std::from_chars.
     *
     * Available for integral and floating-point types.
     */
    template<Arithmetic T>
    inline std::expected<T, ConfigError>
    parse_numeric(std::string_view s)
    {
        T result{};
        const char* first = s.data();
        const char* last  = s.data() + s.size();

        if constexpr (std::is_integral_v<T>) {
            auto [ptr, ec] = std::from_chars(first, last, result, 10);
            if (ec != std::errc{} || ptr != last) {
                return std::unexpected(ConfigError::ParseError);
            }
            return result;
        } else { // floating-point
            auto [ptr, ec] = std::from_chars(first, last, result);
            if (ec != std::errc{} || ptr != last) {
                return std::unexpected(ConfigError::ParseError);
            }
            return result;
        }
    }

    template<typename T, typename String, typename Array, typename Object>
    struct is_valid_pointee
    {
        static constexpr bool value =
            std::is_same_v<T, bool>        ||
            std::is_same_v<T, std::int64_t>||
            std::is_same_v<T, double>      ||
            std::is_same_v<T, String>      ||
            std::is_same_v<T, Array>       ||
            std::is_same_v<T, Object>;
    };

} // namespace config_detail

}
}

#endif // CONFIGMAP_IMPL_CONFIG_DETAILS