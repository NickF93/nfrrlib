#ifndef TRAITS_HPP
#define TRAITS_HPP

#include "all_includes.hpp"

namespace nfrr {
namespace config {
    // Traits that define internal storage types for a given base allocator.
    // Alloc is expected to be a standard-conforming Allocator, e.g.
    //   - std::allocator<std::byte>
    //   - std::pmr::polymorphic_allocator<std::byte>
    template<typename Alloc>
    struct ConfigStorageTraits {
        using base_allocator_type = Alloc;
        using value_type          = BasicConfigValue<Alloc>;

        // Rebind base allocator to char for strings
        using char_allocator = typename std::allocator_traits<base_allocator_type>
            ::template rebind_alloc<char>;

        using string_type = std::basic_string<
            char,
            std::char_traits<char>,
            char_allocator
        >;

        // Rebind base allocator to value_type for arrays
        using value_allocator = typename std::allocator_traits<base_allocator_type>
            ::template rebind_alloc<value_type>;

        using array_type = std::vector<
            value_type,
            value_allocator
        >;

        // Key/value pair for objects
        using key_value_type = std::pair<string_type, value_type>;

        // Rebind base allocator to key_value_type for objects
        using kv_allocator = typename std::allocator_traits<base_allocator_type>
            ::template rebind_alloc<key_value_type>;

        using object_type = std::vector<
            key_value_type,
            kv_allocator
        >;

        // Variant storage for all supported kinds
        using variant_type = std::variant<
            std::monostate,   // Null
            bool,             // Boolean
            std::int64_t,     // Integer
            double,           // Floating
            string_type,      // String
            array_type,       // Array
            object_type       // Object
        >;
    };
}
}

#endif