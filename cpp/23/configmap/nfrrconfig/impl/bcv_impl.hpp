#ifndef BCV_IMPL_HPP
#define BCV_IMPL_HPP

#include "basic_config_value.hpp"

namespace nfrr {
namespace config {
    // --------- inline definitions for raw as_* accessors ---------

    template<typename Alloc>
    inline bool& BasicConfigValue<Alloc>::as_bool() {
        return std::get<bool>(storage_);
    }

    template<typename Alloc>
    inline const bool& BasicConfigValue<Alloc>::as_bool() const {
        return std::get<bool>(storage_);
    }

    template<typename Alloc>
    inline std::int64_t& BasicConfigValue<Alloc>::as_integer() {
        return std::get<std::int64_t>(storage_);
    }

    template<typename Alloc>
    inline const std::int64_t& BasicConfigValue<Alloc>::as_integer() const {
        return std::get<std::int64_t>(storage_);
    }

    template<typename Alloc>
    inline double& BasicConfigValue<Alloc>::as_floating() {
        return std::get<double>(storage_);
    }

    template<typename Alloc>
    inline const double& BasicConfigValue<Alloc>::as_floating() const {
        return std::get<double>(storage_);
    }

    template<typename Alloc>
    inline typename BasicConfigValue<Alloc>::String&
    BasicConfigValue<Alloc>::as_string() {
        return std::get<String>(storage_);
    }

    template<typename Alloc>
    inline const typename BasicConfigValue<Alloc>::String&
    BasicConfigValue<Alloc>::as_string() const {
        return std::get<String>(storage_);
    }

    template<typename Alloc>
    inline typename BasicConfigValue<Alloc>::Array&
    BasicConfigValue<Alloc>::as_array() {
        return std::get<Array>(storage_);
    }

    template<typename Alloc>
    inline const typename BasicConfigValue<Alloc>::Array&
    BasicConfigValue<Alloc>::as_array() const {
        return std::get<Array>(storage_);
    }

    template<typename Alloc>
    inline typename BasicConfigValue<Alloc>::Object&
    BasicConfigValue<Alloc>::as_object() {
        return std::get<Object>(storage_);
    }

    template<typename Alloc>
    inline const typename BasicConfigValue<Alloc>::Object&
    BasicConfigValue<Alloc>::as_object() const {
        return std::get<Object>(storage_);
    }


    // --------- object find() implementations ---------

    template<typename Alloc>
    inline typename BasicConfigValue<Alloc>::Object::iterator
    BasicConfigValue<Alloc>::find(std::string_view key)
    {
        if (!is_object()) {
            static Object empty{};
            return empty.end(); // not great, but consistent: "not found"
        }
        return find_in_object(as_object(), key);
    }

    template<typename Alloc>
    inline typename BasicConfigValue<Alloc>::Object::const_iterator
    BasicConfigValue<Alloc>::find(std::string_view key) const
    {
        if (!is_object()) {
            static const Object empty{};
            return empty.end();
        }
        return find_in_object(as_object(), key);
    }


    // --------- operator[] and at() for object access ---------

    template<typename Alloc>
    template<typename Key>
    inline BasicConfigValue<Alloc>&
    BasicConfigValue<Alloc>::operator[](Key&& key)
    {
        // Convert key to std::string_view
        std::string_view key_view{std::forward<Key>(key)};

        Object& obj = ensure_object();

        auto it = find_in_object(obj, key_view);
        if (it == obj.end()) {
            // Insert new key with null value.
            String key_str{key_view.begin(), key_view.end(), allocator_rebind_char()};
            BasicConfigValue<Alloc> val{allocator_};
            val.set_null();
            obj.emplace_back(std::move(key_str), std::move(val));
            return obj.back().second;
        }
        return it->second;
    }

    template<typename Alloc>
    template<typename Key>
    inline BasicConfigValue<Alloc>&
    BasicConfigValue<Alloc>::at(Key&& key)
    {
        if (!is_object()) {
            throw std::out_of_range{"Config value is not an object"};
        }
        std::string_view key_view{std::forward<Key>(key)};
        Object& obj = as_object();
        auto it = find_in_object(obj, key_view);
        if (it == obj.end()) {
            throw std::out_of_range{"Key not found in object"};
        }
        return it->second;
    }

    template<typename Alloc>
    template<typename Key>
    inline const BasicConfigValue<Alloc>&
    BasicConfigValue<Alloc>::at(Key&& key) const
    {
        if (!is_object()) {
            throw std::out_of_range{"Config value is not an object"};
        }
        std::string_view key_view{std::forward<Key>(key)};
        const Object& obj = as_object();
        auto it = find_in_object(obj, key_view);
        if (it == obj.end()) {
            throw std::out_of_range{"Key not found in object"};
        }
        return it->second;
    }


    // --------- get_impl: core logic for get/try_get ---------

    template<typename Alloc>
    template<typename T, typename Self>
    inline std::expected<T, ConfigError>
    BasicConfigValue<Alloc>::get_impl(Self& self) noexcept
    {
        using RawT = std::remove_cvref_t<T>;
        constexpr bool wants_ref = std::is_reference_v<T>;

        // Reference access is allowed only when the stored type matches exactly.
        if constexpr (wants_ref) {
            using BaseT = std::remove_reference_t<T>;

            // Match stored type and return reference directly.
            if constexpr (std::is_same_v<BaseT, bool>) {
                if (self.is_bool()) {
                    return static_cast<T>(self.as_bool());
                }
                return std::unexpected(ConfigError::TypeMismatch);
            } else if constexpr (std::is_same_v<BaseT, std::int64_t>) {
                if (self.is_integer()) {
                    return static_cast<T>(self.as_integer());
                }
                return std::unexpected(ConfigError::TypeMismatch);
            } else if constexpr (std::is_same_v<BaseT, double>) {
                if (self.is_floating()) {
                    return static_cast<T>(self.as_floating());
                }
                return std::unexpected(ConfigError::TypeMismatch);
            } else if constexpr (std::is_same_v<BaseT, String>) {
                if (self.is_string()) {
                    return static_cast<T>(self.as_string());
                }
                return std::unexpected(ConfigError::TypeMismatch);
            } else if constexpr (std::is_same_v<BaseT, Array>) {
                if (self.is_array()) {
                    return static_cast<T>(self.as_array());
                }
                return std::unexpected(ConfigError::TypeMismatch);
            } else if constexpr (std::is_same_v<BaseT, Object>) {
                if (self.is_object()) {
                    return static_cast<T>(self.as_object());
                }
                return std::unexpected(ConfigError::TypeMismatch);
            } else {
                // Unsupported reference target.
                return std::unexpected(ConfigError::TypeMismatch);
            }
        } else {
            // Value access: allow conversions.

            // Arithmetic targets: numeric conversion from Integer/Floating/Boolean.
            if constexpr (config_detail::Arithmetic<RawT>) {
                if (self.is_integer()) {
                    return config_detail::numeric_from_int64<RawT>(self.as_integer());
                } else if (self.is_floating()) {
                    return config_detail::numeric_from_double<RawT>(self.as_floating());
                } else if (self.is_bool()) {
                    return config_detail::numeric_from_bool<RawT>(self.as_bool());
                } else {
                    return std::unexpected(ConfigError::TypeMismatch);
                }
            } else if constexpr (std::is_same_v<RawT, String>) {
                if (!self.is_string()) {
                    return std::unexpected(ConfigError::TypeMismatch);
                }
                return self.as_string(); // copy
            } else if constexpr (std::is_same_v<RawT, Array>) {
                if (!self.is_array()) {
                    return std::unexpected(ConfigError::TypeMismatch);
                }
                return self.as_array(); // copy
            } else if constexpr (std::is_same_v<RawT, Object>) {
                if (!self.is_object()) {
                    return std::unexpected(ConfigError::TypeMismatch);
                }
                return self.as_object(); // copy
            } else {
                // Unsupported target type.
                return std::unexpected(ConfigError::TypeMismatch);
            }
        }
    }


    // --------- public get / try_get / coerce implementations ---------

    template<typename Alloc>
    template<typename T>
    inline T BasicConfigValue<Alloc>::get()
    {
        static_assert(!std::is_reference_v<T>,
                    "BasicConfigValue::get<T>() does not support reference types; "
                    "use get_ref<T&>() for reference access.");

        auto res = get_impl<T>(*this);
        if (!res) {
            throw std::runtime_error{"BasicConfigValue::get(): type mismatch or conversion error"};
        }
        return *std::move(res);
    }

    template<typename Alloc>
    template<typename T>
    inline T BasicConfigValue<Alloc>::get() const
    {
        static_assert(!std::is_reference_v<T>,
                    "BasicConfigValue::get<T>() const does not support reference types; "
                    "use get_ref<const T&>() for reference access.");

        auto res = get_impl<T>(*this);
        if (!res) {
            throw std::runtime_error{"BasicConfigValue::get() const: type mismatch or conversion error"};
        }
        return *std::move(res);
    }

    template<typename Alloc>
    template<typename T>
    inline std::expected<T, ConfigError>
    BasicConfigValue<Alloc>::try_get() const noexcept
    {
        static_assert(!std::is_reference_v<T>,
                    "BasicConfigValue::try_get<T>() does not support reference types; "
                    "use get_ref<T&>() or as_*( ) for reference access.");

        return get_impl<T>(*this);
    }

    template<typename Alloc>
    template<typename ValueType>
    inline void BasicConfigValue<Alloc>::get_to(ValueType& out) const
    {
        static_assert(!std::is_reference_v<ValueType>,
                    "BasicConfigValue::get_to<ValueType>() does not support reference types; "
                    "store into a non-reference ValueType.");

        auto res = try_get<ValueType>();
        if (!res) {
            throw std::runtime_error{"BasicConfigValue::get_to(): type mismatch or conversion error"};
        }
        out = std::move(*res);
    }



    // --------- get_ref implementation ---------

    template<typename Alloc>
    template<typename ReferenceType>
    inline ReferenceType BasicConfigValue<Alloc>::get_ref()
    {
        static_assert(std::is_reference_v<ReferenceType>,
                    "BasicConfigValue::get_ref requires a reference type, e.g. String&.");

        using BaseType = std::remove_reference_t<ReferenceType>;
        using Plain    = std::remove_cv_t<BaseType>;

        static_assert(
            config_detail::is_valid_pointee<Plain, String, Array, Object>::value,
            "ReferenceType must refer to one of the internal types: "
            "bool, std::int64_t, double, String, Array, Object."
        );

        if constexpr (std::is_same_v<Plain, bool>) {
            if (!is_bool()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<bool&>(): type mismatch"};
            }
            return static_cast<ReferenceType>(as_bool());
        } else if constexpr (std::is_same_v<Plain, std::int64_t>) {
            if (!is_integer()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<int64_t&>(): type mismatch"};
            }
            return static_cast<ReferenceType>(as_integer());
        } else if constexpr (std::is_same_v<Plain, double>) {
            if (!is_floating()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<double&>(): type mismatch"};
            }
            return static_cast<ReferenceType>(as_floating());
        } else if constexpr (std::is_same_v<Plain, String>) {
            if (!is_string()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<String&>(): type mismatch"};
            }
            return static_cast<ReferenceType>(as_string());
        } else if constexpr (std::is_same_v<Plain, Array>) {
            if (!is_array()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<Array&>(): type mismatch"};
            }
            return static_cast<ReferenceType>(as_array());
        } else if constexpr (std::is_same_v<Plain, Object>) {
            if (!is_object()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<Object&>(): type mismatch"};
            }
            return static_cast<ReferenceType>(as_object());
        } else {
            throw std::runtime_error{"BasicConfigValue::get_ref(): unsupported ReferenceType"};
        }
    }

    template<typename Alloc>
    template<typename ReferenceType>
    inline const ReferenceType BasicConfigValue<Alloc>::get_ref() const
    {
        static_assert(std::is_reference_v<ReferenceType>,
                    "BasicConfigValue::get_ref requires a reference type, e.g. const String&.");

        using BaseType = std::remove_reference_t<ReferenceType>;
        using Plain    = std::remove_cv_t<BaseType>;

        static_assert(
            config_detail::is_valid_pointee<Plain, String, Array, Object>::value,
            "ReferenceType must refer to one of the internal types: "
            "bool, std::int64_t, double, String, Array, Object."
        );

        if constexpr (std::is_same_v<Plain, bool>) {
            if (!is_bool()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<bool&>() const: type mismatch"};
            }
            return static_cast<ReferenceType>(as_bool());
        } else if constexpr (std::is_same_v<Plain, std::int64_t>) {
            if (!is_integer()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<int64_t&>() const: type mismatch"};
            }
            return static_cast<ReferenceType>(as_integer());
        } else if constexpr (std::is_same_v<Plain, double>) {
            if (!is_floating()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<double&>() const: type mismatch"};
            }
            return static_cast<ReferenceType>(as_floating());
        } else if constexpr (std::is_same_v<Plain, String>) {
            if (!is_string()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<String&>() const: type mismatch"};
            }
            return static_cast<ReferenceType>(as_string());
        } else if constexpr (std::is_same_v<Plain, Array>) {
            if (!is_array()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<Array&>() const: type mismatch"};
            }
            return static_cast<ReferenceType>(as_array());
        } else if constexpr (std::is_same_v<Plain, Object>) {
            if (!is_object()) {
                throw std::runtime_error{"BasicConfigValue::get_ref<Object&>() const: type mismatch"};
            }
            return static_cast<ReferenceType>(as_object());
        } else {
            throw std::runtime_error{"BasicConfigValue::get_ref() const: unsupported ReferenceType"};
        }
    }

    template<typename Alloc>
    template<typename T>
    inline T BasicConfigValue<Alloc>::coerce() const
    {
        using RawT = std::remove_cvref_t<T>;
        // First try normal get<T>().
        if (auto res = get_impl<RawT>(*this)) {
            return *std::move(res);
        }

        // If that fails and T is arithmetic, allow coercion from string.
        if constexpr (config_detail::Arithmetic<RawT>) {
            if (is_string()) {
                auto parsed = config_detail::parse_numeric<RawT>(
                    std::string_view{as_string().data(), as_string().size()}
                );
                if (!parsed) {
                    throw std::runtime_error{"BasicConfigValue::coerce(): string parse error"};
                }
                return *parsed;
            }
        }

        throw std::runtime_error{"BasicConfigValue::coerce(): type mismatch or unsupported coercion"};
    }
}
}

#endif