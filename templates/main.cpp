#include <variant>
#include <print>

template <typename... T>
struct Visitor: T...
{
    using T::operator()...;
};

int main() {
    std::variant<int, float, std::string> intFloatString { "Hello" };

    Visitor visitor{
        [](const int& i) { std::println("int: {}", i); },
        [](const float& f) { std::println("float: {}", f); },
        [](const std::string& s) { std::println("string: {}", s); }
    };
    std::visit(visitor, intFloatString);
}

