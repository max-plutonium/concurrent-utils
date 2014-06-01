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
#ifndef MOCK_TYPES_H
#define MOCK_TYPES_H

#include <cstdint>
#include <gmock/gmock.h>

template <bool NoexceptMovable = true>
class copyable_movable_t
{
    int i;
    bool copied = false, moved = false;

public:
    copyable_movable_t(int _i) : i(_i) { }

    copyable_movable_t(const copyable_movable_t &o)
        : copyable_movable_t(o.i) { copied = true; }
    copyable_movable_t &operator=(const copyable_movable_t &o)
    { i = o.i; copied = true; return *this; }

    copyable_movable_t(copyable_movable_t &&o) noexcept(NoexceptMovable)
        : copyable_movable_t(o.i) { o.i = 0; moved = true; }
    copyable_movable_t &operator=(copyable_movable_t &&o) noexcept(NoexceptMovable)
    { i = o.i; o.i = 0; moved = true; return *this; }

    ~copyable_movable_t() { i = -1; }

    int get() const { return i; }
    bool was_copied() const { return copied; }
    bool was_moved() const { return moved; }
};

class copyable_but_not_movable_t
{
    int i;
    bool copied = false, moved = false;

public:
    copyable_but_not_movable_t(int _i) : i(_i) { }

    copyable_but_not_movable_t(const copyable_but_not_movable_t &o)
        : copyable_but_not_movable_t(o.i) { copied = true; }
    copyable_but_not_movable_t &operator=(const copyable_but_not_movable_t &o)
    { i = o.i; copied = true; return *this; }

    copyable_but_not_movable_t(copyable_but_not_movable_t &&o) = delete;
    copyable_but_not_movable_t &operator=(copyable_but_not_movable_t &&o) = delete;

    ~copyable_but_not_movable_t() { i = -1; }

    int get() const { return i; }
    bool was_copied() const { return copied; }
    bool was_moved() const { return moved; }
};

class not_copyable_but_movable_t
{
    int i;
    bool copied = false, moved = false;

public:
    not_copyable_but_movable_t(int _i) : i(_i) { }

    not_copyable_but_movable_t(const not_copyable_but_movable_t &o) = delete;
    not_copyable_but_movable_t &operator=(const not_copyable_but_movable_t &o) = delete;

    not_copyable_but_movable_t(not_copyable_but_movable_t &&o) noexcept
        : not_copyable_but_movable_t(o.i) { o.i = 0; moved = true; }
    not_copyable_but_movable_t &operator=(not_copyable_but_movable_t &&o) noexcept
    { i = o.i; o.i = 0; moved = true; return *this; }

    ~not_copyable_but_movable_t() { i = -1; }

    int get() const { return i; }
    bool was_copied() const { return copied; }
    bool was_moved() const { return moved; }
};

class not_copyable_not_movable_t
{
    int i;
    bool copied = false, moved = false;

public:
    not_copyable_not_movable_t(int _i) : i(_i) { }

    not_copyable_not_movable_t(const not_copyable_not_movable_t &o) = delete;
    not_copyable_not_movable_t &operator=(const not_copyable_not_movable_t &o) = delete;

    not_copyable_not_movable_t(not_copyable_not_movable_t &&o) = delete;
    not_copyable_not_movable_t &operator=(not_copyable_not_movable_t &&o) = delete;

    ~not_copyable_not_movable_t() { i = -1; }

    int get() const { return i; }
    bool was_copied() const { return copied; }
    bool was_moved() const { return moved; }
};

class throw_from_copying_t
{
public:
    throw_from_copying_t(int) { }

    throw_from_copying_t(const throw_from_copying_t &) { throw "copy ctor"; }
    throw_from_copying_t &operator=(const throw_from_copying_t &)
    { throw "copy operator"; }
};

struct dummy_mutex
{
    bool locked = false;
    void lock() { locked = true; }
    void unlock() { locked = false; }
};

class mock_mutex
{
public:
    mock_mutex() = default;
    mock_mutex(const mock_mutex&) = delete;
    mock_mutex &operator=(const mock_mutex&) = delete;
    mock_mutex(mock_mutex&&) noexcept = default;
    mock_mutex &operator=(mock_mutex&&) noexcept = default;
    MOCK_METHOD0(lock, void());
    MOCK_METHOD0(unlock, void());
};

#endif // MOCK_TYPES_H
