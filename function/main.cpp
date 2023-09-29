#include <format>
#include <iostream>
#include <cassert>

namespace dojo {

    template <typename T>
    class function;

    template <typename Ret, typename ... Arg>
    class function<Ret (Arg ...)> {
    public:
        template <typename Callable>
        explicit function(Callable callback)
            : m_impl{std::make_unique<callable<Callable>>(std::move(callback))}
        {}

        function(const function& other)
            : m_impl{other->clone()}
        {}

        function& operator=(const function& other) {
            m_impl = other->clone;
            return *this;
        }

        auto operator() (Arg ... args) -> Ret {
            return m_impl->call(args...);
        }

    private:
        struct callable_interface {
            callable_interface() = default;

            virtual ~callable_interface() = default;
            virtual auto call(Arg...) -> Ret = 0;
            [[nodiscard]]
            virtual auto clone() const -> std::unique_ptr<callable_interface> = 0;

            callable_interface(const callable_interface&) = delete;
            callable_interface& operator=(const callable_interface&) = delete;
        };

        template <typename F>
        struct callable: callable_interface {
            explicit callable(F callback)
                : m_callback{std::move(callback)}
            {}

            auto call(Arg... args) -> Ret override {
                return m_callback(args...);
            }

            [[nodiscard]]
            auto clone() const -> std::unique_ptr<callable_interface> override {
                return std::make_unique<callable<F>>(m_callback);
            }

            F m_callback;
        };

        std::unique_ptr<callable_interface> m_impl;
    };
}

auto add(int x, int y) {
    return x + y;
}

struct functor {
    int arg{0};

    auto operator() (int operand) const {
        return arg + operand;
    }
};

int main() {
    {
        // dojo::function<int> f; // compile error
        // dojo::function<int, int, int> f; // compile error
    }

    {
        dojo::function<int(int, int)> func{add};
        assert( func(2, 2) == 4 );
    }
    {
        functor adder{.arg = 42};
        dojo::function<int(int)> func{adder};
        assert( func(1) == 43 );
    }
    {
        dojo::function<int(int, int)> func{[x = 4](auto a, auto b) -> int{ return a * b * 4; }};
        assert( func(2, 2) == 16 );
    }

    std::cout << "OK\n";
    return 0;
}