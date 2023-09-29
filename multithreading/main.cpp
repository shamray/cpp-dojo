#include <iostream>
#include <deque>
#include <stack>
#include <optional>
#include <mutex>
#include <future>

namespace dojo::lock_based {
    template<typename T, template <typename, typename> class Cont = std::deque>
    class stack {
    public:
        stack() = default;

        stack(const stack&) = delete;
        stack& operator = (const stack&) = delete;

        void push(T val) {
            std::scoped_lock<std::mutex> lock(m_mutex);
            m_impl.push(std::move(val));
        }
        auto pop() -> std::optional<T> {
            std::scoped_lock<std::mutex> lock(m_mutex);

            if ( m_impl.empty() )
                return std::nullopt;

            auto val = m_impl.top();
            m_impl.pop();
            return val;
        }
    private:
        mutable std::mutex m_mutex;
        std::stack<T, Cont<T, std::allocator<T>>> m_impl;
    };
}

int main() {
    dojo::lock_based::stack<int> my_stack;

    auto fut = std::async([&my_stack]{ my_stack.push(2014); });
    auto fut1 = std::async([&my_stack]{ my_stack.push(2020); });
    auto fut2 = std::async([&my_stack]{ my_stack.push(2023); });

    auto fut3 = std::async([&my_stack]{ return my_stack.pop(); });
    auto fut4 = std::async([&my_stack]{ return my_stack.pop(); });
    auto fut5 = std::async([&my_stack]{ return my_stack.pop(); });

    fut.get(), fut1.get(), fut2.get();

    std::cout << fut3.get().value() << '\n';
    std::cout << fut4.get().value() << '\n';
    std::cout << fut5.get().value() << '\n';

    return 0;
}