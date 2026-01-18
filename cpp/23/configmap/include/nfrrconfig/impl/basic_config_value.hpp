#ifndef NFRRCONFIG_IMPL_BASIC_CONFIG_VALUE_HPP
#define NFRRCONFIG_IMPL_BASIC_CONFIG_VALUE_HPP

#include <algorithm>
#include <cstdint>
#include <expected>
#include <string_view>
#include <type_traits>
#include <variant>

#include "enums.hpp"
#include "traits.hpp"

namespace nfrr::config {
// Primary configuration value type parametrized on allocator.
template <typename Alloc>
class BasicConfigValue {
  public:
    using allocator_type = Alloc;
    using storage_traits = ConfigStorageTraits<allocator_type>;

    using String = typename storage_traits::string_type;
    using Array = typename storage_traits::array_type;
    using Object = typename storage_traits::object_type;
    using KeyValue = typename storage_traits::key_value_type;
    using Storage = typename storage_traits::variant_type;

    using expected_error = std::expected<void, ConfigError>;

  private:
    Storage storage_;
    allocator_type allocator_{}; // stored allocator instance if needed

  public:
    // --------- ctors / assignment ---------

    /**
     * @brief Construct a null BasicConfigValue using the default-constructed allocator.
     */
    BasicConfigValue() = default;

    /**
     * @brief Construct a null BasicConfigValue with an explicit allocator.
     *
     * @param alloc Allocator instance to be stored and propagated to nested containers.
     */
    explicit BasicConfigValue(const allocator_type& alloc) : storage_{std::monostate{}}, allocator_{alloc} {}

    BasicConfigValue(const BasicConfigValue&) = default;
    BasicConfigValue(BasicConfigValue&&) noexcept = default;
    BasicConfigValue& operator=(const BasicConfigValue&) = default;
    BasicConfigValue& operator=(BasicConfigValue&&) noexcept = default;

    ~BasicConfigValue() = default;

    /**
     * @brief Get a copy of the stored allocator.
     */
    allocator_type get_allocator() const noexcept {
        return allocator_;
    }

    // --------- basic kind inspection ---------

    /**
     * @brief Return the high-level kind of the stored value.
     */
    [[nodiscard]] ConfigValueKind kind() const noexcept {
        switch (storage_.index()) {
            case 0:
                return ConfigValueKind::Null;
            case 1:
                return ConfigValueKind::Boolean;
            case 2:
                return ConfigValueKind::Integer;
            case 3:
                return ConfigValueKind::Floating;
            case 4:
                return ConfigValueKind::String;
            case 5:
                return ConfigValueKind::Array;
            case 6:
                return ConfigValueKind::Object;
            default:
                return ConfigValueKind::Null;
        }
    }

    [[nodiscard]] bool is_null() const noexcept {
        return kind() == ConfigValueKind::Null;
    }
    [[nodiscard]] bool is_bool() const noexcept {
        return kind() == ConfigValueKind::Boolean;
    }
    [[nodiscard]] bool is_integer() const noexcept {
        return kind() == ConfigValueKind::Integer;
    }
    [[nodiscard]] bool is_floating() const noexcept {
        return kind() == ConfigValueKind::Floating;
    }
    [[nodiscard]] bool is_string() const noexcept {
        return kind() == ConfigValueKind::String;
    }
    [[nodiscard]] bool is_array() const noexcept {
        return kind() == ConfigValueKind::Array;
    }
    [[nodiscard]] bool is_object() const noexcept {
        return kind() == ConfigValueKind::Object;
    }

    // --------- raw accessors (exact-type only) ---------

    /**
     * @brief Access the stored boolean by reference (exact-type accessor).
     * @pre kind() == ConfigValueKind::Boolean
     */
    [[nodiscard]] bool& as_bool();
    [[nodiscard]] const bool& as_bool() const;

    /**
     * @brief Access the stored integer by reference (exact-type accessor).
     * @pre kind() == ConfigValueKind::Integer
     */
    [[nodiscard]] std::int64_t& as_integer();
    [[nodiscard]] const std::int64_t& as_integer() const;

    /**
     * @brief Access the stored floating-point value by reference (exact-type accessor).
     * @pre kind() == ConfigValueKind::Floating
     */
    [[nodiscard]] double& as_floating();
    [[nodiscard]] const double& as_floating() const;

    /**
     * @brief Access the stored string by reference (exact-type accessor).
     * @pre kind() == ConfigValueKind::String
     */
    [[nodiscard]] String& as_string();
    [[nodiscard]] const String& as_string() const;

    /**
     * @brief Access the stored array by reference (exact-type accessor).
     * @pre kind() == ConfigValueKind::Array
     */
    [[nodiscard]] Array& as_array();
    [[nodiscard]] const Array& as_array() const;

    /**
     * @brief Access the stored object by reference (exact-type accessor).
     * @pre kind() == ConfigValueKind::Object
     */
    [[nodiscard]] Object& as_object();
    [[nodiscard]] const Object& as_object() const;

    // --------- mutation helpers (exact-type setters) ---------

    /// Set the value to null.
    void set_null() {
        storage_.template emplace<std::monostate>();
    }

    /// Set the value to a boolean.
    void set_bool(bool value) {
        storage_.template emplace<bool>(value);
    }

    /// Set the value to a 64-bit signed integer.
    void set_integer(std::int64_t value) {
        storage_.template emplace<std::int64_t>(value);
    }

    /// Set the value to a double.
    void set_floating(double value) {
        storage_.template emplace<double>(value);
    }

    /// Set the value to a string constructed from std::string_view.
    void set_string(std::string_view s) {
        using Str = String;
        Str tmp{s.begin(), s.end(), allocator_rebind_char()};
        storage_.template emplace<String>(std::move(tmp));
    }

    /// Set the value to an empty array using the proper allocator.
    void set_array() {
        storage_.template emplace<Array>(allocator_rebind_value());
    }

    /// Set the value to an empty object using the proper allocator.
    void set_object() {
        storage_.template emplace<Object>(allocator_rebind_kv());
    }

    // --------- generic assign API ---------

    /**
     * @brief Assign from a boolean.
     */
    void assign(bool value) {
        set_bool(value);
    }

    /**
     * @brief Assign from any signed or unsigned integral type.
     */
    template <typename T>
        requires(std::is_integral_v<std::remove_cv_t<T>> && !std::is_same_v<std::remove_cv_t<T>, bool>)
    void assign(T value) {
        set_integer(static_cast<std::int64_t>(value));
    }

    /**
     * @brief Assign from any floating-point type.
     */
    template <typename T>
        requires std::is_floating_point_v<std::remove_cv_t<T>>
    void assign(T value) {
        set_floating(static_cast<double>(value));
    }

    /**
     * @brief Assign from string-like types (std::string_view, const char*, std::string).
     */
    void assign(std::string_view sv) {
        set_string(sv);
    }

    void assign(const char* s) {
        set_string(std::string_view{s});
    }

    void assign(const String& s) {
        set_string(std::string_view{s});
    }

    /**
     * @brief Assign from another BasicConfigValue (copy semantics).
     */
    void assign(const BasicConfigValue& other) {
        storage_ = other.storage_;
        allocator_ = other.allocator_;
    }

    /**
     * @brief Assign from another BasicConfigValue (move semantics).
     */
    void assign(BasicConfigValue&& other) noexcept {
        *this = std::move(other);
    }

    // --------- high-level get / try_get / coerce ---------

    /**
     * @brief Get the value converted to type T (by value), throwing on error.
     *
     * API Design Note:
     *  - get<T>()        : Performs type conversions, returns by value, throws on error
     *  - get_ref<T&>()   : Direct access without conversion, returns reference, throws on error
     *  - get_to(T&)      : Output parameter style, writes to existing variable, throws on error
     *  - try_get<T>()    : Non-throwing version, returns std::expected<T, ConfigError>
     *  - coerce<T>()     : Like get<T>() but also parses strings to numbers
     *
     * This function supports:
     *  - T = bool, integral, floating: numeric conversions from Integer/Floating/Boolean.
     *  - T = String / Array / Object: exact-type access with copy.
     *  - T = String&, Array&, Object& (or const&): exact-type reference access.
     *
     * @throws std::runtime_error on type mismatch or conversion failure.
     */
    template <typename T>
    [[nodiscard]] T get();

    template <typename T>
    [[nodiscard]] T get() const;

    /**
     * @brief Write the converted value into an existing variable (throwing version).
     *
     * This function is conceptually equivalent to:
     *   out = this->get<ValueType>();
     *
     * @tparam ValueType target type, must be a non-reference type.
     * @param out destination variable that will receive the converted value.
     *
     * @throws std::runtime_error on type mismatch or conversion failure.
     */
    template <typename ValueType>
    void get_to(ValueType& out) const;

    /**
     * @brief Reference access to the internally stored value (throwing on mismatch).
     *
     * This function does not perform any type conversions. It simply returns
     * a reference to the internal storage if the requested ReferenceType
     * matches exactly.
     *
     * @tparam ReferenceType reference to one of the internal storage types:
     *         bool&, std::int64_t&, double&, String&, Array&, Object&
     *         (or their const-qualified variants).
     *
     * @throws std::runtime_error if the stored kind does not match.
     */
    template <typename ReferenceType>
    ReferenceType get_ref();

    template <typename ReferenceType>
    ReferenceType get_ref() const;

    /**
     * @brief Try to get the value converted to type T without throwing.
     *
     * On success returns std::expected<T, ConfigError> with a value.
     * On failure returns std::unexpected(error_code).
     */
    template <typename T>
    [[nodiscard]] std::expected<T, ConfigError> try_get() const noexcept;

    /**
     * @brief Get the value converted to type T, allowing coercion from strings.
     *
     * This behaves like get<T>(), but additionally:
     *  - For arithmetic T, if the stored value is a String, attempts to parse
     *    the numeric value using std::from_chars.
     *
     * @throws std::runtime_error on type mismatch or conversion failure.
     */
    template <typename T>
    [[nodiscard]] T coerce() const;

    // --------- object helpers (map-like access) ---------

    /**
     * @brief Ensure that the current value is an object, constructing an empty one if needed.
     *
     * This is similar to the behavior of some JSON libraries where writing
     * j["key"] on a non-object converts it to an empty object first.
     */
    Object& ensure_object() {
        if (!is_object()) {
            set_object();
        }
        return as_object();
    }

    /**
     * @brief Check if the object contains a given key.
     *
     * @note If the value is not an object, returns false.
     */
    [[nodiscard]] bool contains(std::string_view key) const {
        if (!is_object()) {
            return false;
        }
        const Object& obj = as_object();
        return find_in_object(obj, key) != obj.end();
    }

    /**
     * @brief Find the iterator to a key in the object, or end() if not found.
     *
     * @note If the value is not an object, returns an end() iterator of an empty object.
     */
    typename Object::iterator find(std::string_view key);
    typename Object::const_iterator find(std::string_view key) const;

    /**
     * @brief Map-like operator[] for object access with auto-vivification.
     *
     * Behavior (follows standard JSON library patterns):
     *  - If the value is not an object, it is first converted to an empty object
     *  - If the key does not exist, it is inserted with a null value
     *  - Never throws, always returns a valid reference
     *
     * This enables convenient chained access: root["a"]["b"]["c"] = 42;
     * Use at() for bounds-checked access that throws on missing keys.
     *
     * @return Reference to the associated BasicConfigValue.
     */
    template <typename Key>
    BasicConfigValue& operator[](Key&& key);

    /**
     * @brief Bounds-checked object access, throws if the key does not exist.
     *
     * @throws std::out_of_range if the key is not found, or if the value is not an object.
     */
    template <typename Key>
    BasicConfigValue& at(Key&& key);

    template <typename Key>
    const BasicConfigValue& at(Key&& key) const;

  private:
    // Helper to obtain the rebinded allocators for the internal containers.
    typename storage_traits::char_allocator allocator_rebind_char() const {
        return typename storage_traits::char_allocator{allocator_};
    }

    typename storage_traits::value_allocator allocator_rebind_value() const {
        return typename storage_traits::value_allocator{allocator_};
    }

    typename storage_traits::kv_allocator allocator_rebind_kv() const {
        return typename storage_traits::kv_allocator{allocator_};
    }

    // Helper: find key in object (non-const).
    // Performs O(n) linear search. Efficient for typical config objects with few keys.
    static typename Object::iterator find_in_object(Object& obj, std::string_view key) {
        return std::find_if(obj.begin(), obj.end(), [key](const KeyValue& kv) { return kv.first == key; });
    }

    // Helper: find key in object (const).
    // Performs O(n) linear search. Efficient for typical config objects with few keys.
    static typename Object::const_iterator find_in_object(const Object& obj, std::string_view key) {
        return std::find_if(obj.begin(), obj.end(), [key](const KeyValue& kv) { return kv.first == key; });
    }

    // Internal implementation for get/try_get, factorized on const/non-const.
    template <typename T, typename Self>
    static std::expected<T, ConfigError> get_impl(Self& self) noexcept;
};
} // namespace nfrr::config

#endif
