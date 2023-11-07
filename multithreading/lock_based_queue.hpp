#pragma once

#include <deque>
#include <mutex>
#include <optional>
#include <queue>

namespace dojo::lock_based
{

enum class synchronization_policy {
    coarse_grained,
    fine_grained
};

template <typename T, synchronization_policy sp, template <typename, typename> class Cont = std::deque>
class queue;


template <typename T, template <typename, typename> class Cont>
class queue<T, synchronization_policy::coarse_grained, Cont>
{
public:
    queue() = default;

    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;

    void push(T value) {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_impl.push(std::move(value));
        m_value_pushed.notify_one();
    }

    template <typename Rep, typename Period>
    auto pop(std::optional<std::chrono::duration<Rep, Period>> duration) -> std::optional<T> {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (!wait_push(duration, lock))
            return std::nullopt;

        auto val = m_impl.front();
        m_impl.pop();
        return val;
    }

    auto pop() -> std::optional<T> {
        return pop(std::optional<std::chrono::seconds>{});
    }

private:
    template <typename Rep, typename Period>
    auto wait_push(std::optional<std::chrono::duration<Rep, Period>> duration, auto& lock) const {
        auto wait_predicate = [this]() { return !m_impl.empty(); };
        if (duration.has_value()) {
            return m_value_pushed.wait_for(lock, duration.value(), wait_predicate);
        } else {
            m_value_pushed.wait(lock, wait_predicate);
            return true;
        }
    }

private:
    mutable std::mutex m_mutex;
    mutable std::condition_variable m_value_pushed;
    std::queue<T, Cont<T, std::allocator<T>>> m_impl;
};

template <typename T>
class queue<T, synchronization_policy::fine_grained>
{
public:
    queue() = default;

    void push(T value) {
        auto dummy_node = std::make_unique<node>(T{});
        auto new_tail = dummy_node.get();

        std::scoped_lock tail_lock{m_tail_mutex};

        m_tail->next = std::move(dummy_node);
        m_tail->value = value;
        m_tail = new_tail;

        m_value_pushed.notify_one();
    }

    template <typename Rep, typename Period>
    auto pop(std::optional<std::chrono::duration<Rep, Period>> duration) -> std::optional<T> {
        std::scoped_lock head_lock{m_head_mutex};

        if (!wait_push(duration))
            return std::nullopt;

        auto res = m_head->value;
        auto old_head = std::move(m_head);
        m_head = std::move(old_head->next);

        assert(m_head != nullptr);
        return res;
    }

    auto pop() -> std::optional<T> {
        return pop(std::optional<std::chrono::seconds>{});
    }

private:
    template <typename Rep, typename Period>
    auto wait_push(std::optional<std::chrono::duration<Rep, Period>> duration) const {
        std::unique_lock tail_lock{m_tail_mutex};

        auto wait_predicate = [this]() { return m_head.get() != m_tail; };
        if (duration.has_value()) {
            return m_value_pushed.wait_for(tail_lock, duration.value(), wait_predicate);
        } else {
            m_value_pushed.wait(tail_lock, wait_predicate);
            return true;
        }
    }

private:
    struct node {
        std::unique_ptr<node> next;
        mutable std::mutex mutex;
        T value;

        explicit node(T value)
            : value{std::move(value)} {}
    };

    std::unique_ptr<node> m_head{std::make_unique<node>(T{})};
    node* m_tail{m_head.get()};
    mutable std::mutex m_head_mutex;
    mutable std::mutex m_tail_mutex;
    mutable std::condition_variable m_value_pushed;
};

}// namespace dojo::lock_based