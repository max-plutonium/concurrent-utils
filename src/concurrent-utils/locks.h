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

#include <mutex>

namespace concurrent_utils {

/*!
 * \internal
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
 * @brief Class for ordered locks acquisition
 *
 * Some algorithms need to acquire just two locks that can
 * lead to a hang when two or more threads wait for each other.
 * ordered_lock acquires locks sequentially according to their
 * addresses, ie when called from different threads will
 * still acquire the lock in same manner.
 *
 * Common application - in copy or move constructors
 * in the classes containing the lock.
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
    /*!
     * \brief Конструирует пустой объект
     */
    constexpr ordered_lock() noexcept
        : locks(nullptr, nullptr), locked(false) { }

    /*!
     * \brief Конструирует объект и захватывает блокировки
     */
    ordered_lock(Lockable1 &l1, Lockable2 &l2)
        : ordered_lock(l1, l2, false) { lock(); }

    /*!
     * \brief Конструирует объект без захвата блокировок
     *
     * Необходим для того, чтобы захватить блокировки позже.
     *
     * \sa lock()
     */
    ordered_lock(Lockable1 &l1, Lockable2 &l2, std::defer_lock_t)
        noexcept : ordered_lock(l1, l2, false) { }

    /*!
     * \brief Конструирует объект без захвата блокировок
     *
     * Необходим для того, чтобы сконструировать объект из
     * уже захваченных блокировок.
     *
     * \sa unlock()
     */
    ordered_lock(Lockable1 &l1, Lockable2 &l2, std::adopt_lock_t)
        noexcept : ordered_lock(l1, l2, true) { }

    /*!
     * \brief Освобождает блокировки, если они были захвачены
     */
    ~ordered_lock()
    {
        if(locked)
            unlock();
    }

    ordered_lock(const ordered_lock&) = delete;
    ordered_lock &operator=(const ordered_lock&) = delete;

    /*!
     * \brief Конструирует объект из другого объекта перемещением
     */
    ordered_lock(ordered_lock &&other) noexcept
        : ordered_lock() { swap(other); }

    /*!
     * \brief Присваивает себе блокировки из \a other
     */
    ordered_lock &operator=(ordered_lock &&other) noexcept {
        ordered_lock(std::move(other)).swap(*this);
        return *this;
    }

    /*!
     * \brief Захватывает блокировки
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
           < static_cast<void *>(locks.second))
        {
            locks.first->lock();
            locks.second->lock();
        }
        else
        {
            locks.second->lock();
            locks.first->lock();
        }

        locked = true;
    }

    /*!
     * \brief Освобождает блокировки
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
           < static_cast<void*>(locks.second))
        {
            locks.first->unlock();
            locks.second->unlock();
        }
        else
        {
            locks.second->unlock();
            locks.first->unlock();
        }

        locked = false;
    }

    /*!
     * \brief Отдает блокировки без освобождения
     *
     * \return Пару из хранящихся адресов блокировок.
     */
    std::pair<Lockable1 *, Lockable2 *> release() noexcept
    {
        std::pair<Lockable1*, Lockable2*> ret;
        std::swap(ret, locks);
        return ret;
    }

    /*!
     * \brief Обменивает блокировки с \a other
     */
    void swap(ordered_lock &other) noexcept
    {
        std::swap(locks, other.locks);
        std::swap(locked, other.locked);
    }

    /*!
     * \brief Возвращает true, если блокировки захвачены
     */
    bool owns_lock() const noexcept
    { return locked; }

    /*!
     * \copydoc owns_lock()
     */
    explicit operator bool() const noexcept
    { return owns_lock(); }

}; // class ordered_lock

} // namespace concurrent_utils

#endif // CONCURRENT_UTILS_LOCKS_H
