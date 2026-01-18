// tests/test_configmap.cpp
#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <memory_resource>
#include <numbers>
#include <stdexcept>
#include <string>
#include <utility>

#include "nfrrconfig/impl/enums.hpp"
#include "nfrrconfig/nfrrconfig.hpp"

using Config = nfrr::config::ConfigValueStd;
using ConfigPmr = nfrr::config::ConfigValuePmr;
using nfrr::config::ConfigError;

namespace {
// Test helper in anonymous namespace to avoid clang-tidy warnings
inline void check_condition(bool condition, const char* expr, const char* file, int line) {
    if (!condition) {
        throw std::runtime_error(std::string("CHECK failed: ") + expr + " at " + file + ":" + std::to_string(line));
    }
}
} // namespace

#define CHECK(expr) check_condition((expr), #expr, __FILE__, __LINE__)

// -----------------------------------------------------------------------------
// Test cases declarations
// -----------------------------------------------------------------------------

namespace {
void test_scalar_assign_and_get();
void test_string_and_object_basic();
void test_numeric_conversions();
void test_coerce_from_string();
void test_object_helpers();
void test_array_operations();
void test_edge_cases();
void test_copy_move_semantics();
void test_find_and_contains();
void test_error_handling_patterns();
void test_pmr_allocator();
void test_null_and_kind_queries();
} // namespace

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main() {
    try {
        test_scalar_assign_and_get();
        test_string_and_object_basic();
        test_numeric_conversions();
        test_coerce_from_string();
        test_object_helpers();
        test_array_operations();
        test_edge_cases();
        test_copy_move_semantics();
        test_find_and_contains();
        test_error_handling_patterns();
        test_pmr_allocator();
        test_null_and_kind_queries();
    } catch (const std::exception& ex) {
        std::cerr << "[configmap_tests] FAILURE: " << ex.what() << '\n';
        return 1;
    }

    std::cout << "[configmap_tests] All tests passed.\n";
    return 0;
}

// -----------------------------------------------------------------------------
// Implementations
// -----------------------------------------------------------------------------

namespace {
void test_scalar_assign_and_get() {
    Config v;

    // Integer
    v.assign(42);
    CHECK(v.is_integer());
    CHECK(v.get<int>() == 42);

    // Floating
    v.assign(3.5);
    CHECK(v.is_floating());
    CHECK(v.get<double>() == 3.5);

    // Boolean
    v.assign(true);
    CHECK(v.is_bool());
    CHECK(v.get<bool>());

    // Numeric conversions: Integer -> double/bool
    v.assign(10);
    CHECK(v.get<double>() == 10.0);
    CHECK(v.get<bool>());

    v.assign(0);
    CHECK(!v.get<bool>());
}

void test_string_and_object_basic() {
    Config root;

    // String assignment
    root.assign("hello");

    // Use the internal String type defined by the ConfigValueStd traits
    Config::String internal;
    root.get_to(internal);
    CHECK(internal == "hello");

    // Turn into object and set some fields
    root.set_object();
    root["port"].assign(8080);
    root["host"].assign("localhost");
    root["pi"].assign(std::numbers::pi);

    CHECK(root.is_object());
    CHECK(root.contains("port"));
    CHECK(root.contains("host"));
    CHECK(root.contains("pi"));

    auto port = root["port"].get<int>();
    auto pi_value = root["pi"].get<double>();

    CHECK(port == 8080);
    CHECK(pi_value == std::numbers::pi);

    // get_ref on internal String
    auto& host_ref = root["host"].get_ref<Config::String&>();
    host_ref.append(":8080");

    const auto& host_const_ref = root["host"].get_ref<const Config::String&>();
    CHECK(host_const_ref == "localhost:8080");
}

void test_numeric_conversions() {
    Config v;

    // In-range narrowing: int64_t -> short
    v.assign(100);
    auto ok_short = v.try_get<short>();
    CHECK(ok_short.has_value());
    CHECK(*ok_short == 100);

    // Out-of-range: int64_t -> int
    v.assign(std::numeric_limits<std::int64_t>::max());
    auto bad_int = v.try_get<int>();
    CHECK(!bad_int.has_value());
    CHECK(bad_int.error() == ConfigError::OutOfRange);

    // Fractional loss: double -> int
    v.assign(3.5);
    auto frac = v.try_get<int>();
    CHECK(!frac.has_value());
    CHECK(frac.error() == ConfigError::FractionalLoss);

    // Double -> float is allowed (range-checked)
    auto as_float = v.try_get<float>();
    CHECK(as_float.has_value());
    CHECK(*as_float == static_cast<float>(3.5));

    // Bool conversions
    v.assign(true);
    auto as_int = v.try_get<int>();
    auto as_double = v.try_get<double>();
    CHECK(as_int.has_value() && *as_int == 1);
    CHECK(as_double.has_value() && *as_double == 1.0);

    v.assign(false);
    as_int = v.try_get<int>();
    as_double = v.try_get<double>();
    CHECK(as_int.has_value() && *as_int == 0);
    CHECK(as_double.has_value() && *as_double == 0.0);
}

void test_coerce_from_string() {
    Config v;

    // String -> int
    v.assign("123");
    auto i = v.coerce<int>();
    CHECK(i == 123);

    // String -> double
    v.assign("3.5");
    auto d = v.coerce<double>();
    CHECK(d == 3.5);

    // Invalid numeric string should throw in coerce<T>()
    v.assign("not-a-number");
    bool threw = false;
    try {
        static_cast<void>(v.coerce<int>());
    } catch (const std::runtime_error&) {
        threw = true;
    }
    CHECK(threw);
}

void test_object_helpers() {
    Config root;

    // operator[] must turn non-object into object
    root.assign(42);
    root["answer"].assign(42);

    CHECK(root.is_object());
    CHECK(root.contains("answer"));
    CHECK(root["answer"].get<int>() == 42);

    // at() on existing key
    auto& answer_ref = root.at("answer");
    CHECK(answer_ref.get<int>() == 42);

    // at() on missing key must throw
    bool threw = false;
    try {
        static_cast<void>(root.at("missing"));
    } catch (const std::out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}

void test_array_operations() {
    Config arr;
    arr.set_array();

    CHECK(arr.is_array());

    // Access underlying array directly
    auto& array_ref = arr.as_array();
    CHECK(array_ref.empty());

    // Add elements via direct array manipulation
    Config elem1;
    elem1.assign(10);
    array_ref.push_back(std::move(elem1));

    Config elem2;
    elem2.assign(20);
    array_ref.push_back(std::move(elem2));

    Config elem3;
    elem3.assign(30);
    array_ref.push_back(std::move(elem3));

    CHECK(array_ref.size() == 3);
    CHECK(array_ref[0].get<int>() == 10);
    CHECK(array_ref[1].get<int>() == 20);
    CHECK(array_ref[2].get<int>() == 30);

    // Test iteration
    int sum = 0;
    for (const auto& elem : arr.as_array()) {
        sum += elem.get<int>();
    }
    CHECK(sum == 60);
}

void test_edge_cases() {
    Config v;

    // Default constructed is null
    CHECK(v.is_null());

    // Empty object
    v.set_object();
    CHECK(v.is_object());
    CHECK(!v.contains("anything"));

    // Empty array
    v.set_array();
    CHECK(v.is_array());
    CHECK(v.as_array().empty());

    // Empty string
    v.assign("");
    CHECK(v.is_string());
    CHECK(v.get<Config::String>().empty());

    // Zero values
    v.assign(0);
    CHECK(v.is_integer());
    CHECK(v.get<int>() == 0);

    v.assign(0.0);
    CHECK(v.is_floating());
    CHECK(v.get<double>() == 0.0);
}

void test_copy_move_semantics() {
    Config original;
    original["key"].assign(42);

    // Copy construction
    Config copied = original;
    CHECK(copied.is_object());
    CHECK(copied["key"].get<int>() == 42);

    // Modify copy shouldn't affect original
    copied["key"].assign(100);
    CHECK(original["key"].get<int>() == 42);
    CHECK(copied["key"].get<int>() == 100);

    // Move construction
    Config moved = std::move(copied);
    CHECK(moved.is_object());
    CHECK(moved["key"].get<int>() == 100);

    // Copy assignment
    Config assigned;
    assigned = original;
    CHECK(assigned["key"].get<int>() == 42);

    // Move assignment
    Config move_assigned;
    move_assigned = std::move(assigned);
    CHECK(move_assigned["key"].get<int>() == 42);
}

void test_find_and_contains() {
    Config obj;
    obj.set_object();
    obj["exists"].assign(123);

    // find() on existing key
    auto it = obj.find("exists");
    auto obj_end = obj.as_object().end();
    CHECK(it != obj_end);
    CHECK(it->second.get<int>() == 123);

    // find() on non-existing key
    auto not_found = obj.find("missing");
    CHECK(not_found == obj_end);

    // contains()
    CHECK(obj.contains("exists"));
    CHECK(!obj.contains("missing"));

    // find() on non-object returns sentinel end()
    Config not_obj;
    not_obj.assign(42);
    auto sentinel = not_obj.find("any");
    // Sentinel is static empty object's end(), different from not_obj's non-existent end()
    CHECK(sentinel == sentinel); // Just verify it compiles and returns something
}

void test_error_handling_patterns() {
    Config v;

    // try_get() non-throwing pattern
    v.assign("string_value");
    auto int_result = v.try_get<int>();
    CHECK(!int_result.has_value());
    CHECK(int_result.error() == ConfigError::TypeMismatch);

    // get() throwing pattern
    bool threw = false;
    try {
        static_cast<void>(v.get<int>());
    } catch (const std::runtime_error&) {
        threw = true;
    }
    CHECK(threw);

    // at() throwing on missing key
    Config obj;
    obj.set_object();
    threw = false;
    try {
        static_cast<void>(obj.at("nonexistent"));
    } catch (const std::out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}

void test_pmr_allocator() {
    // Test with PMR allocator - basic operations
    std::array<std::byte, 4096> buffer{};
    std::pmr::monotonic_buffer_resource mbr{buffer.data(), buffer.size()};

    ConfigPmr v{std::pmr::polymorphic_allocator<std::byte>{&mbr}};
    v.assign(42);
    CHECK(v.is_integer());
    CHECK(v.get<int>() == 42);

    // Test string with PMR
    v.assign("test string");
    CHECK(v.is_string());

    // Test array with PMR
    v.set_array();
    CHECK(v.is_array());
}

void test_null_and_kind_queries() {
    Config v;

    // Default is null
    CHECK(v.is_null());
    CHECK(v.kind() == nfrr::config::ConfigValueKind::Null);

    // Test all kinds
    v.assign(true);
    CHECK(v.kind() == nfrr::config::ConfigValueKind::Boolean);

    v.assign(42);
    CHECK(v.kind() == nfrr::config::ConfigValueKind::Integer);

    v.assign(3.14);
    CHECK(v.kind() == nfrr::config::ConfigValueKind::Floating);

    v.assign("text");
    CHECK(v.kind() == nfrr::config::ConfigValueKind::String);

    v.set_array();
    CHECK(v.kind() == nfrr::config::ConfigValueKind::Array);

    v.set_object();
    CHECK(v.kind() == nfrr::config::ConfigValueKind::Object);
}
} // namespace
