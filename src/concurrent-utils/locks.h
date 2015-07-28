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
#ifndef CONCURRENT_UTILS_LOCKS_H
#define CONCURRENT_UTILS_LOCKS_H

#include <atomic>
#include <mutex>
#include <thread>

namespace concurrent_utils {

/**
 * @internal
 */
template <typename Tp> class is_lockable
{
  template <typename Up>
    static constexpr bool check(decltype(std::declval<Up>().lock())*,
                                decltype(std::declval<Up>().unlock())*)
    { return true; }

  template <typename>
    static constexpr bool check(...) { return false; }

public:
    enum { value = check<Tp>(nullptr, nullptr) };
};

/**
 * @brief Spin-lock implementation
 */
class spinlock
{
    std::atomic_flag _flag = ATOMIC_FLAG_INIT;
    std::atomic_uint _sleep_dur;

    using duration_type = std::chrono::microseconds;

    void _sleep() const noexcept {
        if(_sleep_dur) {
            try {
                std::this_thread::sleep_for(duration_type(_sleep_dur));
            } catch (...) { }
        } else {
            std::uint_fast32_t i = 1000;
            while(--i) { }
        }
    }

public:
    /**
     * @brief Create spin-lock
     * @param duration_usecs Duration in microseconds of waiting
     * until lock has been released
     */
    explicit spinlock(unsigned int duration_usecs = 0) noexcept
        : _sleep_dur(duration_usecs) { }

    /**
     * @brief Destroy spin-lock
     */
    ~spinlock() noexcept;

#ifndef DOXYGEN
    spinlock(const spinlock&) = delete;
    spinlock &operator=(const spinlock&) = delete;
    spinlock(spinlock&&) = delete;
    spinlock &operator=(spinlock&&) = delete;
#endif

    /**
     * @brief Acquire the lock
     */
    void lock() noexcept {
        while(_flag.test_and_set(std::memory_order_acquire))
            _sleep();
    }

    /**
     * @brief Release the lock
     */
    void unlock() noexcept {
        _flag.clear(std::memory_order_release);
    }

    /**
     * @brief Tries to acquire the lock @a n times
     * @return true, if lock was acquired
     */
    bool try_lock(unsigned n = 1) noexcept
    {
        while(_flag.test_and_set(std::memory_order_acquire)) {
            if(!--n) return false;
            _sleep();
        }
        return true;
    }

    /**
     * @brief Tries to acquire the lock until @a atime
     * @return true, if lock was acquired
     */
  template <typename Clock, typename Duration>
    inline bool
    try_lock_until(const std::chrono::time_point<Clock, Duration> &atime)
    {
        while(!try_lock()) {
            if(std::chrono::system_clock::now() >= atime)
                return false;
            _sleep();
        }
        return true;
    }

    /**
     * @brief Tries to acquire the lock for @a rtime
     * @return true, if lock was acquired
     */
  template <typename Rep, typename Period>
    inline bool
    try_lock_for(const std::chrono::duration<Rep, Period>& rtime) {
        return try_lock_until(std::chrono::system_clock::now() + rtime);
    }

    /**
     * @return Waiting time on locked spin-lock
     */
    unsigned get_sleep_dur() const noexcept { return _sleep_dur; }

    /**
     * @brief Sets a waiting duration according to @a usecs
     */
    void set_sleep_dur(unsigned usecs) noexcept { _sleep_dur = usecs; }

    /**
     * @brief Sets a waiting duration according to @a rtime
     */
  template <typename Rep, typename Period>
    inline void
    set_sleep_dur(const std::chrono::duration<Rep, Period> &rtime) {
        set_sleep_dur(std::chrono::duration_cast<duration_type>(rtime).count());
    }

    /**
     * @brief Reset a waiting duration
     */
    inline void reset_sleep_dur() noexcept { set_sleep_dur(0); }
};


/**
 * @brief Class for ordered locks acquisition
 *
 * Some algorithms need for acquire just two locks. That can
 * lead to a hang when two or more threads wait for each other.
 * ordered_lock acquires locks sequentially according to their
 * addresses, ie when called from different threads will
 * still acquire the lock in same manner.
 *
 * Common application - in copy or move constructors
 * in classes containing the lock.
 */
template <typename Lockable1, typename Lockable2>
class ordered_lock
{
#ifndef DOXYGEN
    static_assert(is_lockable<Lockable1>::value
                  && is_lockable<Lockable2>::value,
        "ordered_lock only works with lockable types");
#endif

    std::pair<Lockable1 *, Lockable2 *> locks;
    bool locked;

    ordered_lock(Lockable1 &l1, Lockable2 &l2, bool is_locked) noexcept
        : locks(std::addressof(l1), std::addressof(l2))
        , locked(is_locked) { }

public:
    /**
     * @brief Creates empty object
     */
    constexpr ordered_lock() noexcept
        : locks(nullptr, nullptr), locked(false) { }

    /**
     * @brief Creates and acquires locks
     */
    ordered_lock(Lockable1 &l1, Lockable2 &l2)
        : ordered_lock(l1, l2, false) { lock(); }

    /**
     * @brief Creates object without locking
     *
     * Necessary for deferred locks acquisition.
     *
     * @sa lock()
     */
    ordered_lock(Lockable1 &l1, Lockable2 &l2, std::defer_lock_t)
        noexcept : ordered_lock(l1, l2, false) { }

    /**
     * @brief Creates object without locking
     *
     * Necessary in order to construct an object from
     * already acquired locks.
     *
     * @sa unlock()
     */
    ordered_lock(Lockable1 &l1, Lockable2 &l2, std::adopt_lock_t)
        noexcept : ordered_lock(l1, l2, true) { }

    /**
     * @brief Releases locks if they are locked
     */
    ~ordered_lock() {
        if(locked)
            unlock();
    }

    // Disallow copying
    ordered_lock(const ordered_lock&) = delete;
    ordered_lock &operator=(const ordered_lock&) = delete;

    /**
     * @brief Move constructor
     */
    ordered_lock(ordered_lock &&other) noexcept
        : ordered_lock() { swap(other); }

    /**
     * @brief Assigns all locks from @a other
     */
    ordered_lock &operator=(ordered_lock &&other) noexcept {
        ordered_lock(std::move(other)).swap(*this);
        return *this;
    }

    /**
     * @brief Acquires all locks
     */
    void lock()
    {
        using namespace std;
        if(!locks.first && !locks.second && !locked) {
            locked = true;
            return;
        }
        else if(!locks.first || !locks.second)
            throw system_error(make_error_code(
                    errc::operation_not_permitted));
        else if(locked)
            throw system_error(make_error_code(
                    errc::resource_deadlock_would_occur));
        else if(static_cast<void *>(locks.first)
           < static_cast<void *>(locks.second)) {
            locks.first->lock();
            locks.second->lock();
        } else {
            locks.second->lock();
            locks.first->lock();
        }

        locked = true;
    }

    /**
     * @brief Releases all locks
     */
    void unlock()
    {
        using namespace std;
        if(!locks.first && !locks.second && locked) {
            locked = false;
            return;
        }
        else if(!locks.first || !locks.second)
            throw system_error(make_error_code(
                    errc::operation_not_permitted));
        else if(!locked)
            throw system_error(make_error_code(
                    errc::operation_not_permitted));
        else if(static_cast<void*>(locks.first)
           < static_cast<void*>(locks.second)) {
            locks.first->unlock();
            locks.second->unlock();
        } else {
            locks.second->unlock();
            locks.first->unlock();
        }

        locked = false;
    }

    /**
     * @brief Get stored locks without releasing
     *
     * @return Pair of lock's addresses.
     */
    std::pair<Lockable1 *, Lockable2 *> release() noexcept {
        std::pair<Lockable1*, Lockable2*> ret;
        std::swap(ret, locks);
        return ret;
    }

    /**
     * @brief Swaps locks with @a other
     */
    void swap(ordered_lock &other) noexcept {
        std::swap(locks, other.locks);
        std::swap(locked, other.locked);
    }

    /**
     * @return true, if all locks are acquired
     */
    bool owns_lock() const noexcept { return locked; }

    /// @copydoc owns_lock()
    explicit operator bool() const noexcept { return owns_lock(); }

}; // class ordered_lock

} // namespace concurrent_utils

#endif // CONCURRENT_UTILS_LOCKS_H
