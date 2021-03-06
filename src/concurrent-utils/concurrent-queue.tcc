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
#include "concurrent-queue.h"

namespace concurrent_utils {

/**
 * @internal
 * @brief Creates a queue's node from given arguments
 * @param args List of arguments.
 * @return Address of the constructed node.
 */
  template <typename Tp, typename Alloc>
      template<typename... Args>
    auto
    details::basic_forward_queue<Tp, Alloc>::
    _create_node(Args &&...args) -> scoped_node_ptr
    {
        using traits = std::allocator_traits<node_alloc_type>;
        node_alloc_type &alloc = _get_node_allocator();
        auto guarded_ptr = std::__allocate_guarded(alloc);
        traits::construct(alloc, guarded_ptr.get(), std::forward<Args>(args)...);
        scoped_node_ptr node { guarded_ptr.get(), *this };
        guarded_ptr = nullptr;
        return node;
    }

/**
 * @internal
 * @brief Puts the given node @a p to end of the queue
 */
  template <typename Tp, typename Alloc>
    void
    details::basic_forward_queue<Tp, Alloc>::
    _hook(node *p) noexcept
    {
        if(_impl.last)
            _impl.last->next = p;
        else
            _impl.next = p;
        _impl.last = p;
    }

/**
 * @internal
 * @brief Takes the next node from the queue
 * @return Address of taken node.
 */
  template <typename Tp, typename Alloc>
    auto
    details::basic_forward_queue<Tp, Alloc>::
    _unhook_next() noexcept -> scoped_node_ptr
    {
        scoped_node_ptr node { _impl.next, *this };
        if(!node)
            return node;
        else if(!_impl.next->next)
            _impl.last = nullptr;
        _impl.next = _impl.next->next;
        return node;
    }

/**
 * @internal
 * @brief Sequentially takes and deletes all nodes from queue
 * @note Without blocking.
 */
  template <typename Tp, typename Alloc>
    void
    details::basic_forward_queue<Tp, Alloc>::_clear()
    {
        while(_unhook_next());
    }

/**
 * @internal
 * @brief Copies content of @a other to self
 * @note If an exception occurs during nodes are copying, saves
 * the original queue's state.
 */
  template <typename Tp, typename Lock, typename Alloc>
      template<typename Tp2, typename Lock2, typename Alloc2>
    void
    concurrent_queue<Tp, Lock, Alloc>::
    _assign(concurrent_queue<Tp2, Lock2, Alloc2> const &other)
    {
        static_assert(std::is_constructible<Tp, Tp2>::value,
                "template argument substituting Tp in the second"
           " queue object declaration must be constructible from"
                     " Tp in the first queue object declaration");

        // temp's destructor should be executed after all unlocks
        concurrent_queue<Tp, Lock, Alloc> temp;

        // to protect deadlocks
        ordered_lock<Lock, Lock2> locker { _lock, other._lock };

        for(auto *other_node_ptr = other._impl.next;
            other_node_ptr;
            other_node_ptr = other_node_ptr->next)
        {
            // might throw
            typename _base::scoped_node_ptr
                node_ptr = temp._create_node(nullptr, std::ref(other_node_ptr->t));
            temp._hook(node_ptr.release());
        }

        // we can unlock other now
        std::unique_lock<Lock> locker2 { _lock, std::adopt_lock };
        locker.release();
        other._lock.unlock();

        // swap contents with the temp object
        swap_unsafe(temp);
    }

/**
 * @internal
 * @brief Appends contents of @a other to itself by moving
 * nodes using a simple exchange of pointers
 */
  template <typename Tp, typename Lock, typename Alloc>
      template <typename Lock2>
    void
    concurrent_queue<Tp, Lock, Alloc>::
    _append(concurrent_queue<Tp, Lock2, Alloc> &&other)
    {
        // to protect deadlocks
        ordered_lock<Lock, Lock2> locker { _lock, other._lock };
        if(this->_impl.last)
            this->_impl.last->next = other._impl.next;
        else
            this->_impl.next = other._impl.next;
        this->_impl.last = other._impl.last;
        other._impl.next = other._impl.last = nullptr;
    }

/**
 * @internal
 * @brief Appends contents of @a other to itself by copying nodes
 * @note The original queue is still in initial state.
 */
  template <typename Tp, typename Lock, typename Alloc>
      template<typename Tp2, typename Lock2, typename Alloc2>
    void
    concurrent_queue<Tp, Lock, Alloc>::
    _append(concurrent_queue<Tp2, Lock2, Alloc2> const &other)
    {
        static_assert(std::is_constructible<Tp, Tp2>::value,
                "template argument substituting Tp in the second"
           " queue object declaration must be constructible from"
                    " Tp in the first queue object declaration");

        // temp's destructor should be executed after all unlocks
        concurrent_queue<Tp, Lock, Alloc> temp;

        // to protect deadlocks
        ordered_lock<Lock, Lock2> locker { _lock, other._lock };

        for(auto *other_node_ptr = other._impl.next;
            other_node_ptr;
            other_node_ptr = other_node_ptr->next)
        {
            // might throw
            typename _base::scoped_node_ptr
                node_ptr = temp._create_node(nullptr, std::ref(other_node_ptr->t));
            temp._hook(node_ptr.release());
        }

        locker.unlock();

        // move contents from the temp object
        _append(std::move(temp));
    }

/**
 * @internal
 * @brief Waits for items to appear in the queue
 * @return true, if the queue has been closed.
 */
  template <typename Tp, typename Lock, typename Alloc>
    bool
    concurrent_queue<Tp, Lock, Alloc>::
    _wait(std::unique_lock<Lock> &lk)
    {
        _cond.wait(lk, [this]() { return _closed || !_base::_empty(); });
        return _closed;
    }

/**
 * @internal
 * @brief Waits for items to appear in the queue until @a atime
 * @return true, if the queue is closed.
 */
  template <typename Tp, typename Lock, typename Alloc>
      template<typename Clock, typename Duration>
    bool
    concurrent_queue<Tp, Lock, Alloc>::
    _wait(std::unique_lock<Lock> &lk,
          const std::chrono::time_point<Clock, Duration> &atime)
    {
        _cond.wait_until(lk, atime, [this]() {
            return _closed || !_base::_empty(); });
        return _closed;
    }

/**
 * @internal
 * @brief Waits for items to appear in the queue within @a rtime
 * @return true, if the queue is closed.
 */
  template <typename Tp, typename Lock, typename Alloc>
      template <typename Rep, typename Period>
    bool
    concurrent_queue<Tp, Lock, Alloc>::
    _wait(std::unique_lock<Lock> &lk,
          const std::chrono::duration<Rep, Period> &rtime)
    {
        _cond.wait_for(lk, rtime, [this]() {
            return _closed || !_base::_empty(); });
        return _closed;
    }

/**
 * Initializes all fields
 */
  template <typename Tp, typename Lock, typename Alloc>
    concurrent_queue<Tp, Lock, Alloc>::
    concurrent_queue() noexcept
    {
    }

/**
 * Blocks and clears the queue
 */
  template <typename Tp, typename Lock, typename Alloc>
    concurrent_queue<Tp, Lock, Alloc>::
    ~concurrent_queue()
    {
        close();
        _lock.lock();
        _base::_clear();
    }

/**
 * @brief Copies contents of @a other to itself
 * @note If an exception occurs during items are copying,
 * the queue is still empty.
 */
  template <typename Tp, typename Lock, typename Alloc>
    concurrent_queue<Tp, Lock, Alloc>::
    concurrent_queue(const concurrent_queue<Tp, Lock, Alloc> &other)
        : concurrent_queue()
    {
        _assign(other);
    }

/**
 * @brief Clears all contents and copies contents of @a other to itself
 * @note If an exception occurs during items are copying, saves
 * the original queue's state.
 */
  template <typename Tp, typename Lock, typename Alloc>
    concurrent_queue<Tp, Lock, Alloc> &
    concurrent_queue<Tp, Lock, Alloc>::
    operator=(concurrent_queue<Tp, Lock, Alloc> const &other)
    {
        if(this != std::addressof(other))
            _assign(other);
        return *this;
    }

/**
 * @copydoc concurrent_queue(const concurrent_queue &other)
 */
  template <typename Tp, typename Lock, typename Alloc>
      template <typename Tp2, typename Lock2, typename Alloc2>
    concurrent_queue<Tp, Lock, Alloc>::
    concurrent_queue(concurrent_queue<Tp2, Lock2, Alloc2> const &other)
        : concurrent_queue()
    {
        _assign(other);
    }

/**
 * @copydoc concurrent_queue &concurrent_queue::operator=(const concurrent_queue &other)
 */
  template <typename Tp, typename Lock, typename Alloc>
      template <typename Tp2, typename Lock2, typename Alloc2>
    concurrent_queue<Tp, Lock, Alloc> &
    concurrent_queue<Tp, Lock, Alloc>::
    operator=(concurrent_queue<Tp2, Lock2, Alloc2> const &other)
    {
        _assign(other); return *this;
    }

/**
 * @brief Appends contents of @a other to itself by moving items
 */
  template <typename Tp, typename Lock, typename Alloc>
      template <typename Lock2>
    concurrent_queue<Tp, Lock, Alloc> &
    concurrent_queue<Tp, Lock, Alloc>::
    append(concurrent_queue<Tp, Lock2, Alloc> &&other)
    {
        _append(std::move(other)); return *this;
    }

/**
 * @brief Appends contents of @a other to itself by copying items
 * @note The original queue is still in initial state.
 */
  template <typename Tp, typename Lock, typename Alloc>
      template <typename Tp2, typename Lock2, typename Alloc2>
    concurrent_queue<Tp, Lock, Alloc> &
    concurrent_queue<Tp, Lock, Alloc>::
    append(concurrent_queue<Tp2, Lock2, Alloc2> const &other)
    {
        _append(other); return *this;
    }

/**
 * @brief Closes the queue
 */
  template <typename Tp, typename Lock, typename Alloc>
    void
    concurrent_queue<Tp, Lock, Alloc>::close()
    {
        std::lock_guard<Lock> lk(_lock);
        _closed = true;
        _cond.notify_all();
    }

/**
 * @brief Exchanges contents with @a other
 */
  template <typename Tp, typename Lock, typename Alloc>
      template <typename Lock2>
    void
    concurrent_queue<Tp, Lock, Alloc>::
    swap(concurrent_queue<Tp, Lock2, Alloc> &other) noexcept
    {
        // to protect deadlocks
        ordered_lock<Lock, Lock2> locker(_lock, other._lock);
        swap_unsafe(other);
    }

/**
 * @brief Exchanges contents with @a other
 * @note Without blocking.
 */
  template <typename Tp, typename Lock, typename Alloc>
      template <typename Lock2>
    void
    concurrent_queue<Tp, Lock, Alloc>::
    swap_unsafe(concurrent_queue<Tp, Lock2, Alloc> &other) noexcept
    {
        _base::_impl.swap(other._base::_impl);
    }

/**
 * @brief Creates an item from given arguments and puts it
 * to the queue, if it is not closed
 * @return true, if the queue is not closed.
 * @snippet concurrent-queue.cc push
 */
  template <typename Tp, typename Lock, typename Alloc>
      template<typename... Args>
    bool
    concurrent_queue<Tp, Lock, Alloc>::
    push(Args &&...args)
    {
        static_assert(std::is_constructible<value_type, Args...>::value,
                      "template argument substituting Tp"
            " must be constructible from given arguments");

        typename _base::scoped_node_ptr node
            = _base::_create_node(nullptr, std::forward<Args>(args)...);

        std::lock_guard<Lock> lk(_lock);
        if(_closed) return false;
        _base::_hook(node.release());
        _cond.notify_one();
        return true;
    }

/**
 * @brief Takes the next item from the queue and forwards
 * it by reference @a val if the queue is not empty
 * @return false, if the queue is already empty; true otherwise.
 * @snippet concurrent-queue.cc pull
 */
  template <typename Tp, typename Lock, typename Alloc>
    bool
    concurrent_queue<Tp, Lock, Alloc>::
    pull(value_type &val)
    {
        typename _base::scoped_node_ptr node { nullptr, *this };

        {
            std::lock_guard<Lock> lk(_lock);
            node = _base::_unhook_next();
        }

        if(node) {
            val = std::move_if_noexcept(node->t);
            return true;
        }

        return false;
    }

/**
 * @brief Waits for items to appear in the queue,
 * then takes first item and forwards it by reference @a val,
 * if the queue is not closed
 * @return false, if the queue is empty or closed, true otherwise.
 * @snippet concurrent-queue.cc wait_pull
 */
  template <typename Tp, typename Lock, typename Alloc>
    bool
    concurrent_queue<Tp, Lock, Alloc>::
    wait_pull(value_type &val)
    {
        typename _base::scoped_node_ptr node { nullptr, *this };

        {
            std::unique_lock<Lock> lk(_lock);
            if(_wait(lk)) return false; // if closed
            node = _base::_unhook_next();
        }

        if(node) {
            val = std::move_if_noexcept(node->t);
            return true;
        }

        return false;
    }

/**
 * @brief Waits for items to appear in the queue until @a atime,
 * then takes first item and forwards it by reference @a val,
 * if the queue is not closed
 * @return false, if the queue is empty or closed, true otherwise.
 * @snippet concurrent-queue.cc wait_pull
 */
  template <typename Tp, typename Lock, typename Alloc>
      template <typename Clock, typename Duration>
    bool
    concurrent_queue<Tp, Lock, Alloc>::
    wait_pull(const std::chrono::time_point<Clock, Duration> &atime,
              value_type &val)
    {
        typename _base::scoped_node_ptr node { nullptr, *this };

        {
            std::unique_lock<Lock> lk(_lock);
            if(_wait(lk, atime)) return false; // if closed
            node = _base::_unhook_next();
        }

        if(node) {
            val = std::move_if_noexcept(node->t);
            return true;
        }

        return false;
    }

/**
 * @brief Waits for items to appear in the queue within @a rtime,
 * then takes first item and forwards it by reference @a val,
 * if the queue is not closed
 * @return false, if the queue is empty or closed, true otherwise.
 * @snippet concurrent-queue.cc wait_pull
 */
  template <typename Tp, typename Lock, typename Alloc>
      template <typename Rep, typename Period>
    bool
    concurrent_queue<Tp, Lock, Alloc>::
    wait_pull(const std::chrono::duration<Rep, Period> &rtime,
              value_type &val)
    {
        typename _base::scoped_node_ptr node { nullptr, *this };

        {
            std::unique_lock<Lock> lk(_lock);
            if(_wait(lk, rtime)) return false; // if closed
            node = _base::_unhook_next();
        }

        if(node) {
            val = std::move_if_noexcept(node->t);
            return true;
        }

        return false;
    }

/**
 * @brief Creates an item from given arguments and puts it to the queue
 * without blocking, if it is not closed
 * @return true, if the queue is not closed.
 */
  template <typename Tp, typename Lock, typename Alloc>
      template<typename... Args>
    bool
    concurrent_queue<Tp, Lock, Alloc>::
    push_unsafe(Args &&...args)
    {
        static_assert(std::is_constructible<value_type, Args...>::value,
                      "template argument substituting Tp"
            " must be constructible from given arguments");

        if(_closed) return false;
        typename _base::scoped_node_ptr node
            = _base::_create_node(nullptr, std::forward<Args>(args)...);
        _base::_hook(node.release());
        _cond.notify_one();
        return true;
    }

/**
 * @brief Takes the next item from the queue without blocking
 * and forwards it by reference @a val if the queue is not empty
 * @return false, if the queue is already empty; true otherwise.
 */
  template <typename Tp, typename Lock, typename Alloc>
    bool
    concurrent_queue<Tp, Lock, Alloc>::
    pull_unsafe(value_type &val)
    {
        typename _base::scoped_node_ptr node = _base::_unhook_next();

        if(node) {
            val = std::move_if_noexcept(node->t);
            return true;
        }

        return false;
    }

} // namespace concurrent_utils
