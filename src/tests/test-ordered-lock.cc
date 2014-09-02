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
#include <gmock/gmock.h>

#include "../concurrent-utils/locks.h"

using testing::InSequence;

using namespace concurrent_utils;

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


TEST(OrderedLock, Ctors)
{
    ordered_lock<mock_mutex, mock_mutex> lock1;

// not lockable - should not be compiled
// ordered_lock<mock_mutex, std::string> lock2;
// ordered_lock<float, mock_mutex> lock3;

    EXPECT_FALSE(lock1.owns_lock());
    EXPECT_FALSE(!!lock1);

    auto pair = lock1.release();
    EXPECT_EQ(nullptr, pair.first);
    EXPECT_EQ(nullptr, pair.second);

    {
        InSequence seq;
        mock_mutex mtx1, mtx2;
        EXPECT_CALL(mtx1, lock()).Times(1);
        EXPECT_CALL(mtx2, lock()).Times(1);
        EXPECT_CALL(mtx1, unlock()).Times(1);
        EXPECT_CALL(mtx2, unlock()).Times(1);

        ordered_lock<mock_mutex, mock_mutex> lock2 { mtx1, mtx2 };

        EXPECT_TRUE(lock2.owns_lock());
        EXPECT_TRUE(!!lock2);
    }

    {
        InSequence seq;
        mock_mutex mtx1, mtx2;
        EXPECT_CALL(mtx1, lock()).Times(0);
        EXPECT_CALL(mtx2, lock()).Times(0);
        EXPECT_CALL(mtx1, unlock()).Times(0);
        EXPECT_CALL(mtx2, unlock()).Times(0);

        ordered_lock<mock_mutex, mock_mutex> lock2 { mtx1, mtx2, std::defer_lock };

        EXPECT_FALSE(lock2.owns_lock());
        EXPECT_FALSE(!!lock2);
    }

    {
        InSequence seq;
        mock_mutex mtx1, mtx2;
        EXPECT_CALL(mtx1, lock()).Times(0);
        EXPECT_CALL(mtx2, lock()).Times(0);
        EXPECT_CALL(mtx1, unlock()).Times(1);
        EXPECT_CALL(mtx2, unlock()).Times(1);

        ordered_lock<mock_mutex, mock_mutex> lock2 { mtx1, mtx2, std::adopt_lock };

        EXPECT_TRUE(lock2.owns_lock());
        EXPECT_TRUE(!!lock2);
    }

    InSequence seq;
    mock_mutex mtx1, mtx2;
    EXPECT_CALL(mtx1, lock()).Times(1);
    EXPECT_CALL(mtx2, lock()).Times(1);
    EXPECT_CALL(mtx1, unlock()).Times(0);
    EXPECT_CALL(mtx2, unlock()).Times(0);

    ordered_lock<mock_mutex, mock_mutex> lock2 { mtx1, mtx2 };

    EXPECT_TRUE(lock2.owns_lock());
    EXPECT_TRUE(!!lock2);

    lock1 = std::move(lock2);

    EXPECT_TRUE(lock1.owns_lock());
    EXPECT_TRUE(!!lock1);
    EXPECT_FALSE(lock2.owns_lock());
    EXPECT_FALSE(!!lock2);

    pair = lock1.release();
    EXPECT_EQ(&mtx1, pair.first);
    EXPECT_EQ(&mtx2, pair.second);

    pair = lock2.release();
    EXPECT_EQ(nullptr, pair.first);
    EXPECT_EQ(nullptr, pair.second);
}

TEST(OrderedLock, Lock)
{
    InSequence seq;
    mock_mutex mtx1, mtx2;
    EXPECT_CALL(mtx2, lock()).Times(1);
    EXPECT_CALL(mtx1, lock()).Times(1);
    EXPECT_CALL(mtx1, unlock()).Times(0);
    EXPECT_CALL(mtx2, unlock()).Times(0);

    ordered_lock<mock_mutex, mock_mutex> lock { mtx1, mtx2, std::defer_lock };

    lock.lock();

    EXPECT_TRUE(lock.owns_lock());
    EXPECT_TRUE(!!lock);

    EXPECT_THROW(lock.lock(), std::system_error);

    auto pair = lock.release();
    EXPECT_EQ(&mtx1, pair.first);
    EXPECT_EQ(&mtx2, pair.second);

    EXPECT_TRUE(lock.owns_lock());
    EXPECT_TRUE(!!lock);

    EXPECT_THROW(lock.lock(), std::system_error);

    ordered_lock<mock_mutex, mock_mutex> lock2;
    lock2.lock();

    EXPECT_TRUE(lock2.owns_lock());
    EXPECT_TRUE(!!lock2);

    EXPECT_THROW(lock2.lock(), std::system_error);
}

TEST(OrderedLock, Unlock)
{
    InSequence seq;
    mock_mutex mtx1, mtx2;
    EXPECT_CALL(mtx1, lock()).Times(0);
    EXPECT_CALL(mtx2, lock()).Times(0);
    EXPECT_CALL(mtx2, unlock()).Times(1);
    EXPECT_CALL(mtx1, unlock()).Times(1);

    ordered_lock<mock_mutex, mock_mutex> lock { mtx1, mtx2, std::adopt_lock };

    lock.unlock();

    EXPECT_FALSE(lock.owns_lock());
    EXPECT_FALSE(!!lock);

    EXPECT_THROW(lock.unlock(), std::system_error);

    auto pair = lock.release();
    EXPECT_EQ(&mtx1, pair.first);
    EXPECT_EQ(&mtx2, pair.second);

    EXPECT_FALSE(lock.owns_lock());
    EXPECT_FALSE(!!lock);

    EXPECT_THROW(lock.unlock(), std::system_error);

    ordered_lock<mock_mutex, mock_mutex> lock2;
    lock2.lock();
    lock2.unlock();

    EXPECT_FALSE(lock2.owns_lock());
    EXPECT_FALSE(!!lock2);

    EXPECT_THROW(lock2.unlock(), std::system_error);
}

TEST(OrderedLock, Swap)
{
    InSequence seq;
    mock_mutex mtx1, mtx2;
    EXPECT_CALL(mtx2, lock()).Times(1);
    EXPECT_CALL(mtx1, lock()).Times(1);
    EXPECT_CALL(mtx2, unlock()).Times(1);
    EXPECT_CALL(mtx1, unlock()).Times(1);

    ordered_lock<mock_mutex, mock_mutex> lock { mtx1, mtx2 };
    ordered_lock<mock_mutex, mock_mutex> lock2;

    EXPECT_TRUE(lock.owns_lock());
    EXPECT_TRUE(!!lock);
    EXPECT_FALSE(lock2.owns_lock());
    EXPECT_FALSE(!!lock2);

    lock.swap(lock2);

    EXPECT_FALSE(lock.owns_lock());
    EXPECT_FALSE(!!lock);
    EXPECT_TRUE(lock2.owns_lock());
    EXPECT_TRUE(!!lock2);
}
