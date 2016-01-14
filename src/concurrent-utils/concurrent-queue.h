/*
 * Copyright (C) 2014-2015 Max Plutonium <plutonium.max@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization.
 */
#ifndef CONCURRENT_UTILS_CONCURRENT_QUEUE_H
#define CONCURRENT_UTILS_CONCURRENT_QUEUE_H

#include <memory>
#include <type_traits>
#include <condition_variable>

#include "locks.h"

namespace concurrent_utils {

namespace details {

  template <typename Tp, typename Alloc>
    struct basic_forward_queue
    {
        struct node
        {
            node *next;
            Tp   t;

          template <typename... Args>
            node(node *anext, Args &&...args) : next(anext)
              , t(std::forward<Args>(args)...) { }
        };

        // Rebind to node's allocator type
        typedef typename Alloc::template rebind<node>::other
            node_alloc_type;

        // Queue's root structure.
        // Derived from node's allocator to use EBO.
        struct queue_impl : public node_alloc_type
        {
            node  *next = nullptr, *last = nullptr;

            queue_impl() : node_alloc_type() { }
            queue_impl(const node_alloc_type &a) : node_alloc_type(a) { }
            queue_impl(node_alloc_type &&a) : node_alloc_type(std::move(a)) { }

            queue_impl(const queue_impl&) = default;
            queue_impl &operator=(const queue_impl&) = default;

            queue_impl(queue_impl &&other) noexcept
                : node_alloc_type(std::move(other)) { swap(other); }
            queue_impl &operator=(queue_impl &&other) noexcept
            { queue_impl(std::move(other)).swap(*this); return *this; }

            inline void swap(queue_impl &other) noexcept {
                std::swap(next, other.next);
                std::swap(last, other.last);
            }
        };

        void operator()(node *ptr)
        {
            using traits = std::allocator_traits<node_alloc_type>;
            node_alloc_type &alloc = _get_node_allocator();
            traits::destroy(alloc, ptr);
            traits::deallocate(alloc, ptr, 1);
        }

        using scoped_node_ptr = std::unique_ptr<node, basic_forward_queue<Tp, Alloc> &>;

        queue_impl _impl;

        // Returns node's allocator reference
        inline node_alloc_type &_get_node_allocator() noexcept
        { return *static_cast<node_alloc_type*>(&_impl); }

        // Returns node's allocator const-reference
        inline const node_alloc_type &_get_node_allocator() const noexcept
        { return *static_cast<const node_alloc_type*>(&_impl); }

      template <typename... Args>
        scoped_node_ptr _create_node(Args&&...);

        inline void  _hook(node*) noexcept;
        inline scoped_node_ptr _unhook_next() noexcept;
        void _clear();
        inline bool _empty() const noexcept { return !_impl.last; }

    }; // struct basic_forward_queue

} // namespace details


template <typename Tp, typename Lock, typename Alloc = std::allocator<Tp>>
class concurrent_queue : protected details::basic_forward_queue<Tp, Alloc>
{
#ifndef DOXYGEN
    static_assert(std::is_copy_constructible<Tp>::value
                  || std::is_move_constructible<Tp>::value,
        "concurrent_queue requires copyable or movable template argument");

    static_assert(is_lockable<Lock>::value,
        "concurrent_queue only works with lockable type");
#endif

    using _base = details::basic_forward_queue<Tp, Alloc>;
    using _cond_type = typename std::conditional<
        std::is_same<Lock, std::mutex>::value,
        std::condition_variable, std::condition_variable_any>::type;

    mutable Lock _lock;
    _cond_type _cond;
    bool _closed = false;

    // To allow construction and assignment from any incompatible types
  template <typename Tp2, typename Lock2, typename Alloc2>
    void _assign(concurrent_queue<Tp2, Lock2, Alloc2> const&);

  template <typename Lock2>
    void _append(concurrent_queue<Tp, Lock2, Alloc> &&);

    // We can append queues with any compatible types
  template <typename Tp2, typename Lock2, typename Alloc2>
    void _append(concurrent_queue<Tp2, Lock2, Alloc2> const&);

    bool _wait(std::unique_lock<Lock> &lk);

  template<typename Clock, typename Duration>
    bool _wait(std::unique_lock<Lock> &lk,
        const std::chrono::time_point<Clock, Duration> &atime);

  template <typename Rep, typename Period>
    bool _wait(std::unique_lock<Lock> &lk,
        const std::chrono::duration<Rep, Period> &rtime);

public:
    using allocator_type = Alloc;
    using value_type = Tp;
    using size_type = std::size_t;

    /// Returns the allocator used by the queue
    allocator_type get_allocator() const noexcept
    { return allocator_type(_base::_get_node_allocator()); }

    concurrent_queue() noexcept;
    ~concurrent_queue();

    concurrent_queue(concurrent_queue const&);
    concurrent_queue &operator=(concurrent_queue const&);

    // To allow construction and assignment from any compatible types

    /// @copydoc concurrent_queue(const concurrent_queue &other)
  template <typename Tp2, typename Lock2, typename Alloc2>
    concurrent_queue(concurrent_queue<Tp2, Lock2, Alloc2> const&);

    /// @copydoc concurrent_queue::operator=(const concurrent_queue &other)
  template <typename Tp2, typename Lock2, typename Alloc2>
    concurrent_queue &operator=(concurrent_queue<Tp2, Lock2, Alloc2> const&);

    /// @brief Appends contents of @a other to itself by moving items
  template <typename Lock2>
    concurrent_queue &append(concurrent_queue<Tp, Lock2, Alloc> &&other);

    /// @copydoc append
  template <typename Tp2, typename Lock2, typename Alloc2>
    concurrent_queue &append(concurrent_queue<Tp2, Lock2, Alloc2> const&);

    /// Moves content from \a other to self
    concurrent_queue(concurrent_queue &&other) noexcept
        : concurrent_queue() { swap(other); }

    /// Clears content and moves it from \a other to self
    concurrent_queue &operator=(concurrent_queue &&other) noexcept
    { concurrent_queue(std::move(other)).swap(*this); return *this; }

    /// @copydoc concurrent_queue(concurrent_queue &&other)
  template <typename Lock2>
    concurrent_queue(concurrent_queue<Tp, Lock2, Alloc> &&other) noexcept
        : concurrent_queue() { swap(other); }

    /// @copydoc operator=(concurrent_queue &&other)
  template <typename Lock2>
    concurrent_queue &operator=(concurrent_queue<Tp, Lock2, Alloc> &&other) noexcept
    { concurrent_queue(std::move(other)).swap(*this); return *this; }

#ifndef DOXYGEN
    // Moving from another types is prohibited
  template <typename Tp2, typename Lock2, typename Alloc2>
    concurrent_queue(concurrent_queue<Tp2, Lock2, Alloc2> &&) = delete;
  template <typename Tp2, typename Lock2, typename Alloc2>
    concurrent_queue &operator=(concurrent_queue<Tp2, Lock2, Alloc2> &&) = delete;
#endif

    /// Returns true, if queue's size equals zero
    inline bool empty() const
    { std::lock_guard<Lock> lk(_lock); return _base::_empty(); }

    /// Returns true, if queue closed
    inline bool closed() const
    { std::lock_guard<Lock> lk(_lock); return _closed; }

    /// Clears queue's contents
    inline void clear() noexcept
    { concurrent_queue<Tp, Lock, Alloc>().swap(*this); }

    void close();

    /// Returns reference to a internal queue's lock
    inline Lock &underlying_lock() const noexcept { return _lock; }

  template <typename Lock2>
    void swap(concurrent_queue<Tp, Lock2, Alloc> &) noexcept;

  template <typename Lock2>
    void swap_unsafe(concurrent_queue<Tp, Lock2, Alloc> &) noexcept;

  template <typename... Args>
    bool push(Args &&...args);

    bool pull(value_type &val);

    bool wait_pull(value_type &val);

  template <typename Clock, typename Duration>
    bool wait_pull(const std::chrono::time_point<Clock, Duration> &atime,
                   value_type &val);

  template <typename Rep, typename Period>
    bool wait_pull(const std::chrono::duration<Rep, Period> &rtime,
                   value_type &val);

  template <typename... Args>
    bool push_unsafe(Args &&...args);

    bool pull_unsafe(value_type &val);

  template <typename Tpa, typename Locka, typename Alloca>
    friend class concurrent_queue;

}; // class concurrent_queue

} // namespace concurrent_utils

#include "concurrent-queue.tcc"

#endif // CONCURRENT_UTILS_CONCURRENT_QUEUE_H
