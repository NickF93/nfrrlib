// main.cpp
#include <iostream>
#include <numbers>
#include <string>

#include "nfrrconfig/nfrrconfig.hpp"

int main() {
    using Config = nfrr::config::ConfigValueStd; // or ConfigValuePmr later

    Config root;

    // Assign simple scalar values
    root.assign(42); // integer
    auto int_val = root.get<int>();
    std::cout << "int_val = " << int_val << '\n';

    root.assign(3.14);
    auto dbl_val = root.get<double>();
    std::cout << "dbl_val = " << dbl_val << '\n';

    // Assign a string
    root.assign("hello");
    std::string s;
    root.get_to(s); // uses get_to<std::string>()
    std::cout << "string s = " << s << '\n';

    // Turn root into an object
    root.set_object();

    // Fill object fields using operator[]
    root["port"].assign(8080);
    root["host"].assign("localhost");
    root["pi"].assign(std::numbers::pi);

    // Numeric conversion with get<T>()
    auto port = root["port"].get<int>();
    auto pi = root["pi"].get<double>();

    std::cout << "port = " << port << '\n';
    std::cout << "pi   = " << pi << '\n';

    // String access with get_to and get_ref
    root["host"].assign("example.com");

    // get_to into std::string
    std::string host_copy;
    root["host"].get_to(host_copy);
    std::cout << "host_copy = " << host_copy << '\n';

    // get_ref to modify the internal String in-place
    auto& host_ref = root["host"].get_ref<Config::String&>();
    host_ref.append(":8080");
    std::cout << "host_ref  = " << host_ref << '\n';

    // Iterate over the object entries
    std::cout << "Object entries:\n";
    for (auto& [key, value] : root.as_object()) {
        std::cout << "  key = " << key << ", kind = " << static_cast<int>(value.kind()) << '\n';
    }

    // Example of coerce: parse numeric from string
    root["answer"].assign("123");
    int answer = root["answer"].coerce<int>();
    std::cout << "answer (coerced from string) = " << answer << '\n';

    return 0;
}
