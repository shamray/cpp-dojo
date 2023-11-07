#pragma once

#include <deque>
#include <mutex>
#include <optional>
#include <stack>

namespace dojo::lock_based
{
template <typename T, template <typename, typename> class Cont = std::deque>
class stack
{
public:
    stack() = default;

    stack(const stack&) = delete;
    stack& operator=(const stack&) = delete;

    void push(T val) {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_impl.push(std::move(val));
    }
    auto pop() -> std::optional<T> {
        std::scoped_lock<std::mutex> lock(m_mutex);

        if (m_impl.empty())
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