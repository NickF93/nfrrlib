// tests/test_configmap.cpp
#include "nfrrconfig/nfrrconfig.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <limits>

using Config = nfrr::config::ConfigValueStd;
using nfrr::config::ConfigError;

// Very small test helper
#define CHECK(expr)                                                          \
    do {                                                                     \
        if (!(expr)) {                                                       \
            throw std::runtime_error(                                        \
                std::string("CHECK failed: ") + #expr +                      \
                " at " + __FILE__ + ":" + std::to_string(__LINE__));         \
        }                                                                    \
    } while (false)

// -----------------------------------------------------------------------------
// Test cases declarations
// -----------------------------------------------------------------------------

static void test_scalar_assign_and_get();
static void test_string_and_object_basic();
static void test_numeric_conversions();
static void test_coerce_from_string();
static void test_object_helpers();

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main()
{
    try {
        test_scalar_assign_and_get();
        test_string_and_object_basic();
        test_numeric_conversions();
        test_coerce_from_string();
        test_object_helpers();
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

static void test_scalar_assign_and_get()
{
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
    CHECK(v.get<bool>() == true);

    // Numeric conversions: Integer -> double/bool
    v.assign(10);
    CHECK(v.get<double>() == 10.0);
    CHECK(v.get<bool>() == true);

    v.assign(0);
    CHECK(v.get<bool>() == false);
}

static void test_string_and_object_basic()
{
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
    root["pi"].assign(3.14159);

    CHECK(root.is_object());
    CHECK(root.contains("port"));
    CHECK(root.contains("host"));
    CHECK(root.contains("pi"));

    int port = root["port"].get<int>();
    double pi = root["pi"].get<double>();

    CHECK(port == 8080);
    CHECK(pi == 3.14159);

    // get_ref on internal String
    auto& host_ref = root["host"].get_ref<Config::String&>();
    host_ref.append(":8080");

    const auto& host_const_ref = root["host"].get_ref<const Config::String&>();
    CHECK(host_const_ref == "localhost:8080");
}

static void test_numeric_conversions()
{
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

static void test_coerce_from_string()
{
    Config v;

    // String -> int
    v.assign("123");
    int i = v.coerce<int>();
    CHECK(i == 123);

    // String -> double
    v.assign("3.5");
    double d = v.coerce<double>();
    CHECK(d == 3.5);

    // Invalid numeric string should throw in coerce<T>()
    v.assign("not-a-number");
    bool threw = false;
    try {
        (void)v.coerce<int>();
    } catch (const std::runtime_error&) {
        threw = true;
    }
    CHECK(threw);
}

static void test_object_helpers()
{
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
        (void)root.at("missing");
    } catch (const std::out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}
