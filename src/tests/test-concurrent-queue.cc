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
#include <gtest/gtest.h>

#include "../concurrent-utils/concurrent-queue.h"
#include "mock-types.h"

using namespace concurrent_utils;

TEST(ConcurrentQueue, CtorAndDtor)
{
    concurrent_queue<copyable_movable_t<>, dummy_mutex> qq1;
    concurrent_queue<copyable_but_not_movable_t, dummy_mutex> qq2;
    concurrent_queue<not_copyable_but_movable_t, dummy_mutex> qq3;

//  should not be compiled
//  concurrent_queue<not_copyable_not_movable_t, dummy_mutex> qq4;
//  concurrent_queue<not_copyable_but_movable_t, dummy_mutex> qq5(std::move(qq1));
//  qq2 = std::move(qq1);

    concurrent_queue<std::size_t, dummy_mutex> q1;
    concurrent_queue<std::size_t, std::mutex> q2;
    EXPECT_TRUE(q1.empty());
    EXPECT_TRUE(q2.empty());
    EXPECT_FALSE(q1.closed());
    EXPECT_FALSE(q2.closed());
}

#include <boost/lexical_cast.hpp>

TEST(ConcurrentQueue, PushPullUnsafe)
{
    concurrent_queue<int, dummy_mutex> q_int_dummy;
    concurrent_queue<int, std::mutex> q_int_mutex;
    concurrent_queue<double, dummy_mutex> q_double_dummy;
    concurrent_queue<double, std::mutex> q_double_mutex;
    concurrent_queue<std::string, dummy_mutex> q_string_dummy;
    concurrent_queue<std::string, std::mutex> q_string_mutex;

    EXPECT_TRUE(q_int_dummy.empty());
    EXPECT_TRUE(q_int_mutex.empty());
    EXPECT_TRUE(q_double_dummy.empty());
    EXPECT_TRUE(q_double_mutex.empty());
    EXPECT_TRUE(q_string_dummy.empty());
    EXPECT_TRUE(q_string_mutex.empty());

    constexpr int num_tests = 3;
    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        ASSERT_TRUE(q_int_dummy.push_unsafe(d));
        ASSERT_TRUE(q_int_mutex.push_unsafe(d));
        ASSERT_TRUE(q_double_dummy.push_unsafe(d));
        ASSERT_TRUE(q_double_mutex.push_unsafe(d));
        ASSERT_TRUE(q_string_dummy.push_unsafe(boost::lexical_cast<std::string>(d)));
        ASSERT_TRUE(q_string_mutex.push_unsafe(boost::lexical_cast<std::string>(d)));
    }

    EXPECT_FALSE(q_int_dummy.empty());
    EXPECT_FALSE(q_int_mutex.empty());
    EXPECT_FALSE(q_double_dummy.empty());
    EXPECT_FALSE(q_double_mutex.empty());
    EXPECT_FALSE(q_string_dummy.empty());
    EXPECT_FALSE(q_string_mutex.empty());

    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        int ret_int = -99;
        double ret_double = -99.0;
        std::string ret_string = "-99";

        ASSERT_TRUE(q_int_dummy.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);
    }

    int ret_int = -99;
    double ret_double = -99.0;
    std::string ret_string = "-99";

    EXPECT_FALSE(q_int_dummy.pull_unsafe(ret_int));
    EXPECT_FALSE(q_int_mutex.pull_unsafe(ret_int));
    EXPECT_FALSE(q_double_dummy.pull_unsafe(ret_double));
    EXPECT_FALSE(q_double_mutex.pull_unsafe(ret_double));
    EXPECT_FALSE(q_string_dummy.pull_unsafe(ret_string));
    EXPECT_FALSE(q_string_mutex.pull_unsafe(ret_string));

    // If the queue is empty - value should not be changed
    EXPECT_EQ(-99, ret_int);
    EXPECT_EQ(-99.0, ret_double);
    EXPECT_EQ(std::string("-99"), ret_string);

    EXPECT_TRUE(q_int_dummy.empty());
    EXPECT_TRUE(q_int_mutex.empty());
    EXPECT_TRUE(q_double_dummy.empty());
    EXPECT_TRUE(q_double_mutex.empty());
    EXPECT_TRUE(q_string_dummy.empty());
    EXPECT_TRUE(q_string_mutex.empty());

    concurrent_queue<copyable_movable_t<>, dummy_mutex> qq1;
    concurrent_queue<copyable_movable_t<false>, dummy_mutex> qq2;
    concurrent_queue<copyable_but_not_movable_t, dummy_mutex> qq3;
    concurrent_queue<not_copyable_but_movable_t, dummy_mutex> qq4;
    concurrent_queue<throw_from_copying_t, dummy_mutex> qq5;

    for(int i = 0; i < num_tests; ++i)
    {
        ASSERT_TRUE(qq1.push_unsafe(i));
        ASSERT_TRUE(qq2.push_unsafe(i));
        ASSERT_TRUE(qq3.push_unsafe(i));
        ASSERT_TRUE(qq4.push_unsafe(i));
        ASSERT_TRUE(qq5.push_unsafe(i));
    }

    for(int i = 0; i < num_tests; ++i)
    {
        copyable_movable_t<> ret_cm(99);
        copyable_movable_t<false> ret_cm_except(99);
        copyable_but_not_movable_t ret_cnm(99);
        not_copyable_but_movable_t ret_ncm(99);
        throw_from_copying_t ret_tfc(99);

        ASSERT_TRUE(qq1.pull_unsafe(ret_cm));
        EXPECT_EQ(i, ret_cm.get());
        EXPECT_FALSE(ret_cm.was_copied());
        EXPECT_TRUE(ret_cm.was_moved());

        ASSERT_TRUE(qq2.pull_unsafe(ret_cm_except));
        EXPECT_EQ(i, ret_cm_except.get());
        EXPECT_TRUE(ret_cm_except.was_copied());
        EXPECT_FALSE(ret_cm_except.was_moved());

        ASSERT_TRUE(qq3.pull_unsafe(ret_cnm));
        EXPECT_EQ(i, ret_cnm.get());
        EXPECT_TRUE(ret_cnm.was_copied());
        EXPECT_FALSE(ret_cnm.was_moved());

        ASSERT_TRUE(qq4.pull_unsafe(ret_ncm));
        EXPECT_EQ(i, ret_ncm.get());
        EXPECT_FALSE(ret_ncm.was_copied());
        EXPECT_TRUE(ret_ncm.was_moved());

        ASSERT_THROW(qq5.pull_unsafe(ret_tfc), const char *);
    }
}

TEST(ConcurrentQueue, SwapUnsafe)
{
    concurrent_queue<std::size_t, dummy_mutex> q1;
    concurrent_queue<std::size_t, std::mutex> q2;

    constexpr int num_tests = 3;
    for(std::size_t i = 1; i < num_tests + 1; ++i)
        ASSERT_TRUE(q1.push_unsafe(i));

    EXPECT_FALSE(q1.empty());
    EXPECT_TRUE(q2.empty());

    q1.swap_unsafe(q2);

    EXPECT_TRUE(q1.empty());
    EXPECT_FALSE(q2.empty());

    std::size_t ret;
    for(std::size_t i = 1; i < num_tests + 1; ++i) {
        ASSERT_TRUE(q2.pull_unsafe(ret));
        EXPECT_EQ(i, ret);
    }

    EXPECT_TRUE(q1.empty());
    EXPECT_TRUE(q2.empty());

    for(std::size_t i = 1; i < num_tests + 1; ++i) {
        ASSERT_TRUE(q1.push_unsafe(i));
        ASSERT_TRUE(q2.push_unsafe(i + num_tests));
    }

    EXPECT_FALSE(q1.empty());
    EXPECT_FALSE(q2.empty());

    q1.swap_unsafe(q2);

    EXPECT_FALSE(q1.empty());
    EXPECT_FALSE(q2.empty());

    std::size_t ret2;
    for(std::size_t i = 1; i < num_tests + 1; ++i)
    {
        ASSERT_TRUE(q1.pull_unsafe(ret));
        EXPECT_EQ(i + num_tests, ret);
        ASSERT_TRUE(q2.pull_unsafe(ret2));
        EXPECT_EQ(i, ret2);
    }

    EXPECT_TRUE(q1.empty());
    EXPECT_TRUE(q2.empty());
}

TEST(ConcurrentQueue, CopyCtors)
{
    concurrent_queue<int, dummy_mutex> q_int_dummy;
    concurrent_queue<int, std::mutex> q_int_mutex;
    concurrent_queue<double, dummy_mutex> q_double_dummy;
    concurrent_queue<double, std::mutex> q_double_mutex;
    concurrent_queue<std::string, dummy_mutex> q_string_dummy;
    concurrent_queue<std::string, std::mutex> q_string_mutex;

    constexpr int num_tests = 3;
    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        ASSERT_TRUE(q_int_dummy.push_unsafe(d));
        ASSERT_TRUE(q_int_mutex.push_unsafe(d));
        ASSERT_TRUE(q_double_dummy.push_unsafe(d));
        ASSERT_TRUE(q_double_mutex.push_unsafe(d));
        ASSERT_TRUE(q_string_dummy.push_unsafe(boost::lexical_cast<std::string>(d)));
        ASSERT_TRUE(q_string_mutex.push_unsafe(boost::lexical_cast<std::string>(d)));
    }

    EXPECT_FALSE(q_int_dummy.empty());
    EXPECT_FALSE(q_int_mutex.empty());
    EXPECT_FALSE(q_double_dummy.empty());
    EXPECT_FALSE(q_double_mutex.empty());
    EXPECT_FALSE(q_string_dummy.empty());
    EXPECT_FALSE(q_string_mutex.empty());

    concurrent_queue<int, dummy_mutex> q_int_dummy2(q_int_dummy);
    concurrent_queue<int, std::mutex> q_int_mutex2(q_int_mutex);
    concurrent_queue<double, dummy_mutex> q_double_dummy2(q_double_dummy);
    concurrent_queue<double, std::mutex> q_double_mutex2(q_double_mutex);
    concurrent_queue<std::string, dummy_mutex> q_string_dummy2(q_string_dummy);
    concurrent_queue<std::string, std::mutex> q_string_mutex2(q_string_mutex);

    EXPECT_FALSE(q_int_dummy2.empty());
    EXPECT_FALSE(q_int_mutex2.empty());
    EXPECT_FALSE(q_double_dummy2.empty());
    EXPECT_FALSE(q_double_mutex2.empty());
    EXPECT_FALSE(q_string_dummy2.empty());
    EXPECT_FALSE(q_string_mutex2.empty());

    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        int ret_int = -99;
        double ret_double = -99.0;
        std::string ret_string = "-99";

        ASSERT_TRUE(q_int_dummy2.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex2.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy2.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex2.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy2.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex2.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);
    }

    EXPECT_TRUE(q_int_dummy2.empty());
    EXPECT_TRUE(q_int_mutex2.empty());
    EXPECT_TRUE(q_double_dummy2.empty());
    EXPECT_TRUE(q_double_mutex2.empty());
    EXPECT_TRUE(q_string_dummy2.empty());
    EXPECT_TRUE(q_string_mutex2.empty());

    concurrent_queue<float, dummy_mutex> q_float_dummy(q_int_mutex);
    concurrent_queue<float, std::mutex> q_float_mutex(q_int_dummy);
    concurrent_queue<unsigned, dummy_mutex> q_unsigned_dummy(q_double_mutex);
    concurrent_queue<unsigned, std::mutex> q_unsigned_mutex(q_double_dummy);

    EXPECT_FALSE(q_float_dummy.empty());
    EXPECT_FALSE(q_float_mutex.empty());
    EXPECT_FALSE(q_unsigned_dummy.empty());
    EXPECT_FALSE(q_unsigned_mutex.empty());

    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        float ret_float = -99.0f;
        unsigned ret_unsigned = 99;

        ASSERT_TRUE(q_float_dummy.pull_unsafe(ret_float));
        EXPECT_EQ(d, ret_float);

        ASSERT_TRUE(q_float_mutex.pull_unsafe(ret_float));
        EXPECT_EQ(d, ret_float);

        ASSERT_TRUE(q_unsigned_dummy.pull_unsafe(ret_unsigned));
        EXPECT_EQ(d, ret_unsigned);

        ASSERT_TRUE(q_unsigned_mutex.pull_unsafe(ret_unsigned));
        EXPECT_EQ(d, ret_unsigned);
    }

    EXPECT_TRUE(q_float_dummy.empty());
    EXPECT_TRUE(q_float_mutex.empty());
    EXPECT_TRUE(q_unsigned_dummy.empty());
    EXPECT_TRUE(q_unsigned_mutex.empty());
}

TEST(ConcurrentQueue, Append)
{
    concurrent_queue<int, dummy_mutex> q_int_dummy;
    concurrent_queue<int, std::mutex> q_int_mutex;
    concurrent_queue<double, dummy_mutex> q_double_dummy;
    concurrent_queue<double, std::mutex> q_double_mutex;
    concurrent_queue<std::string, dummy_mutex> q_string_dummy;
    concurrent_queue<std::string, std::mutex> q_string_mutex;

    constexpr int num_tests = 3;
    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        ASSERT_TRUE(q_int_dummy.push_unsafe(d));
        ASSERT_TRUE(q_int_mutex.push_unsafe(d));
        ASSERT_TRUE(q_double_dummy.push_unsafe(d));
        ASSERT_TRUE(q_double_mutex.push_unsafe(d));
        ASSERT_TRUE(q_string_dummy.push_unsafe(boost::lexical_cast<std::string>(d)));
        ASSERT_TRUE(q_string_mutex.push_unsafe(boost::lexical_cast<std::string>(d)));
    }

    concurrent_queue<int, dummy_mutex> q_int_dummy2;
    concurrent_queue<int, std::mutex> q_int_mutex2;
    concurrent_queue<double, dummy_mutex> q_double_dummy2;
    concurrent_queue<double, std::mutex> q_double_mutex2;
    concurrent_queue<std::string, dummy_mutex> q_string_dummy2;
    concurrent_queue<std::string, std::mutex> q_string_mutex2;

    q_int_dummy2.append(q_int_mutex);
    q_int_mutex2.append(q_int_dummy);
    q_double_dummy2.append(q_double_mutex);
    q_double_mutex2.append(q_double_dummy);
    q_string_dummy2.append(q_string_mutex);
    q_string_mutex2.append(q_string_dummy);

    EXPECT_FALSE(q_int_dummy.empty());
    EXPECT_FALSE(q_int_mutex.empty());
    EXPECT_FALSE(q_double_dummy.empty());
    EXPECT_FALSE(q_double_mutex.empty());
    EXPECT_FALSE(q_string_dummy.empty());
    EXPECT_FALSE(q_string_mutex.empty());

    EXPECT_FALSE(q_int_dummy2.empty());
    EXPECT_FALSE(q_int_mutex2.empty());
    EXPECT_FALSE(q_double_dummy2.empty());
    EXPECT_FALSE(q_double_mutex2.empty());
    EXPECT_FALSE(q_string_dummy2.empty());
    EXPECT_FALSE(q_string_mutex2.empty());

    q_int_dummy2.append(std::move(q_int_mutex));
    q_int_mutex2.append(std::move(q_int_dummy));
    q_double_dummy2.append(std::move(q_double_mutex));
    q_double_mutex2.append(std::move(q_double_dummy));
    q_string_dummy2.append(std::move(q_string_mutex));
    q_string_mutex2.append(std::move(q_string_dummy));

    EXPECT_TRUE(q_int_dummy.empty());
    EXPECT_TRUE(q_int_mutex.empty());
    EXPECT_TRUE(q_double_dummy.empty());
    EXPECT_TRUE(q_double_mutex.empty());
    EXPECT_TRUE(q_string_dummy.empty());
    EXPECT_TRUE(q_string_mutex.empty());

    EXPECT_FALSE(q_int_dummy2.empty());
    EXPECT_FALSE(q_int_mutex2.empty());
    EXPECT_FALSE(q_double_dummy2.empty());
    EXPECT_FALSE(q_double_mutex2.empty());
    EXPECT_FALSE(q_string_dummy2.empty());
    EXPECT_FALSE(q_string_mutex2.empty());
}

TEST(ConcurrentQueue, CopyAssign)
{
    concurrent_queue<int, dummy_mutex> q_int_dummy;
    concurrent_queue<int, std::mutex> q_int_mutex;
    concurrent_queue<double, dummy_mutex> q_double_dummy;
    concurrent_queue<double, std::mutex> q_double_mutex;
    concurrent_queue<std::string, dummy_mutex> q_string_dummy;
    concurrent_queue<std::string, std::mutex> q_string_mutex;

    constexpr int num_tests = 3;
    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        ASSERT_TRUE(q_int_dummy.push_unsafe(d));
        ASSERT_TRUE(q_int_mutex.push_unsafe(d));
        ASSERT_TRUE(q_double_dummy.push_unsafe(d));
        ASSERT_TRUE(q_double_mutex.push_unsafe(d));
        ASSERT_TRUE(q_string_dummy.push_unsafe(boost::lexical_cast<std::string>(d)));
        ASSERT_TRUE(q_string_mutex.push_unsafe(boost::lexical_cast<std::string>(d)));
    }

    EXPECT_FALSE(q_int_dummy.empty());
    EXPECT_FALSE(q_int_mutex.empty());
    EXPECT_FALSE(q_double_dummy.empty());
    EXPECT_FALSE(q_double_mutex.empty());
    EXPECT_FALSE(q_string_dummy.empty());
    EXPECT_FALSE(q_string_mutex.empty());

    concurrent_queue<int, dummy_mutex> q_int_dummy2;
    concurrent_queue<int, std::mutex> q_int_mutex2;
    concurrent_queue<double, dummy_mutex> q_double_dummy2;
    concurrent_queue<double, std::mutex> q_double_mutex2;
    concurrent_queue<std::string, dummy_mutex> q_string_dummy2;
    concurrent_queue<std::string, std::mutex> q_string_mutex2;

    EXPECT_TRUE(q_int_dummy2.empty());
    EXPECT_TRUE(q_int_mutex2.empty());
    EXPECT_TRUE(q_double_dummy2.empty());
    EXPECT_TRUE(q_double_mutex2.empty());
    EXPECT_TRUE(q_string_dummy2.empty());
    EXPECT_TRUE(q_string_mutex2.empty());

    q_int_dummy2 = q_int_dummy;
    q_int_mutex2 = q_int_mutex;
    q_double_dummy2 = q_double_dummy;
    q_double_mutex2 = q_double_mutex;
    q_string_dummy2 = q_string_dummy;
    q_string_mutex2 = q_string_mutex;

    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        int ret_int = -99;
        double ret_double = -99.0;
        std::string ret_string = "-99";

        ASSERT_TRUE(q_int_dummy2.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex2.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy2.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex2.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy2.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex2.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);
    }

    EXPECT_TRUE(q_int_dummy2.empty());
    EXPECT_TRUE(q_int_mutex2.empty());
    EXPECT_TRUE(q_double_dummy2.empty());
    EXPECT_TRUE(q_double_mutex2.empty());
    EXPECT_TRUE(q_string_dummy2.empty());
    EXPECT_TRUE(q_string_mutex2.empty());

    concurrent_queue<float, dummy_mutex> q_float_dummy;
    concurrent_queue<float, std::mutex> q_float_mutex;
    concurrent_queue<unsigned, dummy_mutex> q_unsigned_dummy;
    concurrent_queue<unsigned, std::mutex> q_unsigned_mutex;

    EXPECT_TRUE(q_float_dummy.empty());
    EXPECT_TRUE(q_float_mutex.empty());
    EXPECT_TRUE(q_unsigned_dummy.empty());
    EXPECT_TRUE(q_unsigned_mutex.empty());

    q_float_dummy = q_int_mutex;
    q_float_mutex = q_int_dummy;
    q_unsigned_dummy = q_double_mutex;
    q_unsigned_mutex = q_double_dummy;

    EXPECT_FALSE(q_float_dummy.empty());
    EXPECT_FALSE(q_float_mutex.empty());
    EXPECT_FALSE(q_unsigned_dummy.empty());
    EXPECT_FALSE(q_unsigned_mutex.empty());

    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        float ret_float = -99.0f;
        unsigned ret_unsigned = 99;

        ASSERT_TRUE(q_float_dummy.pull_unsafe(ret_float));
        EXPECT_EQ(d, ret_float);

        ASSERT_TRUE(q_float_mutex.pull_unsafe(ret_float));
        EXPECT_EQ(d, ret_float);

        ASSERT_TRUE(q_unsigned_dummy.pull_unsafe(ret_unsigned));
        EXPECT_EQ(d, ret_unsigned);

        ASSERT_TRUE(q_unsigned_mutex.pull_unsafe(ret_unsigned));
        EXPECT_EQ(d, ret_unsigned);
    }

    EXPECT_TRUE(q_float_dummy.empty());
    EXPECT_TRUE(q_float_mutex.empty());
    EXPECT_TRUE(q_unsigned_dummy.empty());
    EXPECT_TRUE(q_unsigned_mutex.empty());
}

TEST(ConcurrentQueue, MoveCtors)
{
    concurrent_queue<int, dummy_mutex> q_int_dummy;
    concurrent_queue<int, std::mutex> q_int_mutex;
    concurrent_queue<double, dummy_mutex> q_double_dummy;
    concurrent_queue<double, std::mutex> q_double_mutex;
    concurrent_queue<std::string, dummy_mutex> q_string_dummy;
    concurrent_queue<std::string, std::mutex> q_string_mutex;

    constexpr int num_tests = 3;
    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        ASSERT_TRUE(q_int_dummy.push_unsafe(d));
        ASSERT_TRUE(q_int_mutex.push_unsafe(d));
        ASSERT_TRUE(q_double_dummy.push_unsafe(d));
        ASSERT_TRUE(q_double_mutex.push_unsafe(d));
        ASSERT_TRUE(q_string_dummy.push_unsafe(boost::lexical_cast<std::string>(d)));
        ASSERT_TRUE(q_string_mutex.push_unsafe(boost::lexical_cast<std::string>(d)));
    }

    EXPECT_FALSE(q_int_dummy.empty());
    EXPECT_FALSE(q_int_mutex.empty());
    EXPECT_FALSE(q_double_dummy.empty());
    EXPECT_FALSE(q_double_mutex.empty());
    EXPECT_FALSE(q_string_dummy.empty());
    EXPECT_FALSE(q_string_mutex.empty());

    concurrent_queue<int, dummy_mutex> q_int_dummy2(std::move(q_int_dummy));
    concurrent_queue<int, std::mutex> q_int_mutex2(std::move(q_int_mutex));
    concurrent_queue<double, dummy_mutex> q_double_dummy2(std::move(q_double_dummy));
    concurrent_queue<double, std::mutex> q_double_mutex2(std::move(q_double_mutex));
    concurrent_queue<std::string, dummy_mutex> q_string_dummy2(std::move(q_string_dummy));
    concurrent_queue<std::string, std::mutex> q_string_mutex2(std::move(q_string_mutex));

    EXPECT_TRUE(q_int_dummy.empty());
    EXPECT_TRUE(q_int_mutex.empty());
    EXPECT_TRUE(q_double_dummy.empty());
    EXPECT_TRUE(q_double_mutex.empty());
    EXPECT_TRUE(q_string_dummy.empty());
    EXPECT_TRUE(q_string_mutex.empty());

    EXPECT_FALSE(q_int_dummy2.empty());
    EXPECT_FALSE(q_int_mutex2.empty());
    EXPECT_FALSE(q_double_dummy2.empty());
    EXPECT_FALSE(q_double_mutex2.empty());
    EXPECT_FALSE(q_string_dummy2.empty());
    EXPECT_FALSE(q_string_mutex2.empty());

    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        int ret_int = -99;
        double ret_double = -99.0;
        std::string ret_string = "-99";

        ASSERT_TRUE(q_int_dummy2.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex2.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy2.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex2.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy2.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex2.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);
    }

    EXPECT_TRUE(q_int_dummy2.empty());
    EXPECT_TRUE(q_int_mutex2.empty());
    EXPECT_TRUE(q_double_dummy2.empty());
    EXPECT_TRUE(q_double_mutex2.empty());
    EXPECT_TRUE(q_string_dummy2.empty());
    EXPECT_TRUE(q_string_mutex2.empty());

    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        ASSERT_TRUE(q_int_dummy.push_unsafe(d));
        ASSERT_TRUE(q_int_mutex.push_unsafe(d));
        ASSERT_TRUE(q_double_dummy.push_unsafe(d));
        ASSERT_TRUE(q_double_mutex.push_unsafe(d));
        ASSERT_TRUE(q_string_dummy.push_unsafe(boost::lexical_cast<std::string>(d)));
        ASSERT_TRUE(q_string_mutex.push_unsafe(boost::lexical_cast<std::string>(d)));
    }

    concurrent_queue<int, dummy_mutex> q_int_dummy3(std::move(q_int_mutex));
    concurrent_queue<int, std::mutex> q_int_mutex3(std::move(q_int_dummy));
    concurrent_queue<double, dummy_mutex> q_double_dummy3(std::move(q_double_mutex));
    concurrent_queue<double, std::mutex> q_double_mutex3(std::move(q_double_dummy));
    concurrent_queue<std::string, dummy_mutex> q_string_dummy3(std::move(q_string_mutex));
    concurrent_queue<std::string, std::mutex> q_string_mutex3(std::move(q_string_dummy));

    EXPECT_TRUE(q_int_dummy.empty());
    EXPECT_TRUE(q_int_mutex.empty());
    EXPECT_TRUE(q_double_dummy.empty());
    EXPECT_TRUE(q_double_mutex.empty());
    EXPECT_TRUE(q_string_dummy.empty());
    EXPECT_TRUE(q_string_mutex.empty());

    EXPECT_FALSE(q_int_dummy3.empty());
    EXPECT_FALSE(q_int_mutex3.empty());
    EXPECT_FALSE(q_double_dummy3.empty());
    EXPECT_FALSE(q_double_mutex3.empty());
    EXPECT_FALSE(q_string_dummy3.empty());
    EXPECT_FALSE(q_string_mutex3.empty());

    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        int ret_int = -99;
        double ret_double = -99.0;
        std::string ret_string = "-99";

        ASSERT_TRUE(q_int_dummy3.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex3.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy3.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex3.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy3.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex3.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);
    }

    EXPECT_TRUE(q_int_dummy3.empty());
    EXPECT_TRUE(q_int_mutex3.empty());
    EXPECT_TRUE(q_double_dummy3.empty());
    EXPECT_TRUE(q_double_mutex3.empty());
    EXPECT_TRUE(q_string_dummy3.empty());
    EXPECT_TRUE(q_string_mutex3.empty());
}

TEST(ConcurrentQueue, MoveAssign)
{
    concurrent_queue<int, dummy_mutex> q_int_dummy;
    concurrent_queue<int, std::mutex> q_int_mutex;
    concurrent_queue<double, dummy_mutex> q_double_dummy;
    concurrent_queue<double, std::mutex> q_double_mutex;
    concurrent_queue<std::string, dummy_mutex> q_string_dummy;
    concurrent_queue<std::string, std::mutex> q_string_mutex;

    constexpr int num_tests = 3;
    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        ASSERT_TRUE(q_int_dummy.push_unsafe(d));
        ASSERT_TRUE(q_int_mutex.push_unsafe(d));
        ASSERT_TRUE(q_double_dummy.push_unsafe(d));
        ASSERT_TRUE(q_double_mutex.push_unsafe(d));
        ASSERT_TRUE(q_string_dummy.push_unsafe(boost::lexical_cast<std::string>(d)));
        ASSERT_TRUE(q_string_mutex.push_unsafe(boost::lexical_cast<std::string>(d)));
    }

    EXPECT_FALSE(q_int_dummy.empty());
    EXPECT_FALSE(q_int_mutex.empty());
    EXPECT_FALSE(q_double_dummy.empty());
    EXPECT_FALSE(q_double_mutex.empty());
    EXPECT_FALSE(q_string_dummy.empty());
    EXPECT_FALSE(q_string_mutex.empty());

    concurrent_queue<int, dummy_mutex> q_int_dummy2;
    concurrent_queue<int, std::mutex> q_int_mutex2;
    concurrent_queue<double, dummy_mutex> q_double_dummy2;
    concurrent_queue<double, std::mutex> q_double_mutex2;
    concurrent_queue<std::string, dummy_mutex> q_string_dummy2;
    concurrent_queue<std::string, std::mutex> q_string_mutex2;

    EXPECT_TRUE(q_int_dummy2.empty());
    EXPECT_TRUE(q_int_mutex2.empty());
    EXPECT_TRUE(q_double_dummy2.empty());
    EXPECT_TRUE(q_double_mutex2.empty());
    EXPECT_TRUE(q_string_dummy2.empty());
    EXPECT_TRUE(q_string_mutex2.empty());

    q_int_dummy2 = std::move(q_int_dummy);
    q_int_mutex2 = std::move(q_int_mutex);
    q_double_dummy2 = std::move(q_double_dummy);
    q_double_mutex2 = std::move(q_double_mutex);
    q_string_dummy2 = std::move(q_string_dummy);
    q_string_mutex2 = std::move(q_string_mutex);

    EXPECT_TRUE(q_int_dummy.empty());
    EXPECT_TRUE(q_int_mutex.empty());
    EXPECT_TRUE(q_double_dummy.empty());
    EXPECT_TRUE(q_double_mutex.empty());
    EXPECT_TRUE(q_string_dummy.empty());
    EXPECT_TRUE(q_string_mutex.empty());

    EXPECT_FALSE(q_int_dummy2.empty());
    EXPECT_FALSE(q_int_mutex2.empty());
    EXPECT_FALSE(q_double_dummy2.empty());
    EXPECT_FALSE(q_double_mutex2.empty());
    EXPECT_FALSE(q_string_dummy2.empty());
    EXPECT_FALSE(q_string_mutex2.empty());

    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        int ret_int = -99;
        double ret_double = -99.0;
        std::string ret_string = "-99";

        ASSERT_TRUE(q_int_dummy2.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex2.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy2.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex2.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy2.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex2.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);
    }

    EXPECT_TRUE(q_int_dummy2.empty());
    EXPECT_TRUE(q_int_mutex2.empty());
    EXPECT_TRUE(q_double_dummy2.empty());
    EXPECT_TRUE(q_double_mutex2.empty());
    EXPECT_TRUE(q_string_dummy2.empty());
    EXPECT_TRUE(q_string_mutex2.empty());

    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        ASSERT_TRUE(q_int_dummy.push_unsafe(d));
        ASSERT_TRUE(q_int_mutex.push_unsafe(d));
        ASSERT_TRUE(q_double_dummy.push_unsafe(d));
        ASSERT_TRUE(q_double_mutex.push_unsafe(d));
        ASSERT_TRUE(q_string_dummy.push_unsafe(boost::lexical_cast<std::string>(d)));
        ASSERT_TRUE(q_string_mutex.push_unsafe(boost::lexical_cast<std::string>(d)));
    }

    concurrent_queue<int, dummy_mutex> q_int_dummy3;
    concurrent_queue<int, std::mutex> q_int_mutex3;
    concurrent_queue<double, dummy_mutex> q_double_dummy3;
    concurrent_queue<double, std::mutex> q_double_mutex3;
    concurrent_queue<std::string, dummy_mutex> q_string_dummy3;
    concurrent_queue<std::string, std::mutex> q_string_mutex3;

    q_int_dummy3 = std::move(q_int_mutex);
    q_int_mutex3 = std::move(q_int_dummy);
    q_double_dummy3 = std::move(q_double_mutex);
    q_double_mutex3 = std::move(q_double_dummy);
    q_string_dummy3 = std::move(q_string_mutex);
    q_string_mutex3 = std::move(q_string_dummy);

    EXPECT_TRUE(q_int_dummy.empty());
    EXPECT_TRUE(q_int_mutex.empty());
    EXPECT_TRUE(q_double_dummy.empty());
    EXPECT_TRUE(q_double_mutex.empty());
    EXPECT_TRUE(q_string_dummy.empty());
    EXPECT_TRUE(q_string_mutex.empty());

    EXPECT_FALSE(q_int_dummy3.empty());
    EXPECT_FALSE(q_int_mutex3.empty());
    EXPECT_FALSE(q_double_dummy3.empty());
    EXPECT_FALSE(q_double_mutex3.empty());
    EXPECT_FALSE(q_string_dummy3.empty());
    EXPECT_FALSE(q_string_mutex3.empty());

    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        int ret_int = -99;
        double ret_double = -99.0;
        std::string ret_string = "-99";

        ASSERT_TRUE(q_int_dummy3.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex3.pull_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy3.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex3.pull_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy3.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex3.pull_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);
    }

    EXPECT_TRUE(q_int_dummy3.empty());
    EXPECT_TRUE(q_int_mutex3.empty());
    EXPECT_TRUE(q_double_dummy3.empty());
    EXPECT_TRUE(q_double_mutex3.empty());
    EXPECT_TRUE(q_string_dummy3.empty());
    EXPECT_TRUE(q_string_mutex3.empty());
}

TEST(ConcurrentQueue, Clear)
{
    concurrent_queue<std::size_t, dummy_mutex> q1;
    concurrent_queue<std::size_t, std::mutex> q2;

    EXPECT_TRUE(q1.empty());
    EXPECT_TRUE(q2.empty());

    constexpr int num_tests = 3;
    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        ASSERT_TRUE(q1.push_unsafe(d));
        ASSERT_TRUE(q2.push_unsafe(d));
    }

    EXPECT_FALSE(q1.empty());
    EXPECT_FALSE(q2.empty());

    std::size_t ret = 99;
    EXPECT_TRUE(q1.pull_unsafe(ret));
    EXPECT_TRUE(q2.pull_unsafe(ret));

    q1.clear();
    EXPECT_TRUE(q1.empty());
    EXPECT_FALSE(q1.pull_unsafe(ret));

    q2.clear();
    EXPECT_TRUE(q2.empty());
    EXPECT_FALSE(q2.pull_unsafe(ret));
}

TEST(ConcurrentQueue, Close)
{
    concurrent_queue<std::size_t, dummy_mutex> q1;
    concurrent_queue<std::size_t, std::mutex> q2;

    constexpr int num_tests = 3;
    for(double d = 1.0; d < num_tests + 1; ++d)
    {
        ASSERT_TRUE(q1.push_unsafe(d));
        ASSERT_TRUE(q2.push_unsafe(d));
    }

    EXPECT_FALSE(q1.closed());
    EXPECT_FALSE(q2.closed());

    std::size_t ret = 99;
    EXPECT_TRUE(q1.pull_unsafe(ret));

    q1.close();
    EXPECT_FALSE(q1.push_unsafe(123));
    EXPECT_FALSE(q1.empty());
    EXPECT_TRUE(q1.closed());

    // We can pull from the queue yet
    EXPECT_TRUE(q1.pull_unsafe(ret));

    // If the queue is closed - value should not be changed
    EXPECT_EQ(std::size_t(2), ret);

    EXPECT_TRUE(q2.pull_unsafe(ret));
    EXPECT_TRUE(q2.pull_unsafe(ret));
    EXPECT_TRUE(q2.pull_unsafe(ret));

    q2.close();
    EXPECT_FALSE(q2.push_unsafe(123));
    EXPECT_TRUE(q2.empty());
    EXPECT_TRUE(q2.closed());
    EXPECT_FALSE(q2.pull_unsafe(ret));

    // If the queue is closed - value should not be changed
    EXPECT_EQ(std::size_t(3), ret);
}

TEST(ConcurrentQueue, PushPull)
{
    concurrent_queue<int, std::mutex> q_int_mutex;
    concurrent_queue<double, std::mutex> q_double_mutex;
    concurrent_queue<std::string, std::mutex> q_string_mutex;

    constexpr std::size_t num_tests = 1000000;

    auto producer_task = [&]() {
        for(double d = 0; d < num_tests; ++d) {
            ASSERT_TRUE(q_int_mutex.push(d));
            ASSERT_TRUE(q_double_mutex.push(d));
            ASSERT_TRUE(q_string_mutex.push(boost::lexical_cast<std::string>(d)));
        }
    };

    auto consumer_task = [&]() {
        for(double d = 0; d < num_tests; ++d) {
            int ret_int = 0;
            double ret_double = 0;
            std::string ret_string;

            while(!q_int_mutex.pull(ret_int));
            EXPECT_EQ(d, ret_int);

            while(!q_double_mutex.pull(ret_double));
            EXPECT_EQ(d, ret_double);

            while(!q_string_mutex.pull(ret_string));
            EXPECT_EQ(d, boost::lexical_cast<std::size_t>(ret_string));
        }

        ASSERT_TRUE(q_int_mutex.empty());
        ASSERT_TRUE(q_double_mutex.empty());
        ASSERT_TRUE(q_string_mutex.empty());

        int ret_int = -99;
        double ret_double = -99.0;
        std::string ret_string = "-99";

        EXPECT_FALSE(q_int_mutex.pull(ret_int));
        EXPECT_FALSE(q_double_mutex.pull(ret_double));
        EXPECT_FALSE(q_string_mutex.pull(ret_string));

        // If the queue is closed - value should not be changed
        EXPECT_EQ(-99, ret_int);
        EXPECT_EQ(-99.0, ret_double);
        EXPECT_EQ(std::string("-99"), ret_string);
    };

    std::thread(producer_task).detach();
    consumer_task();
}

#include <future>

TEST(ConcurrentQueue, PushPullMultithreaded)
{
    constexpr std::size_t num_producers = 4, num_consumers = 4;
    constexpr std::size_t iterations = 1000000;

    using result_type = std::tuple<std::size_t, std::size_t, std::size_t>;
    std::vector<std::future<result_type>> results(num_consumers);

    concurrent_queue<int, std::mutex> q_int_mutex;
    concurrent_queue<double, std::mutex> q_double_mutex;
    concurrent_queue<std::string, std::mutex> q_string_mutex;

    auto sum_single = [iterations]() -> std::size_t {
        std::size_t res = 0;
        for(std::size_t i = 0; i < iterations; ++i)
            res += i;
        return res;
    };

    auto producer_function = [&](std::size_t start, std::size_t finish) {
        for(double d = start; d < finish; ++d)
        {
            ASSERT_TRUE(q_int_mutex.push(d));
            ASSERT_TRUE(q_double_mutex.push(d));
            ASSERT_TRUE(q_string_mutex.push(boost::lexical_cast<std::string>(d)));
        }
    };

    auto consumer_function = [&](std::size_t start, std::size_t finish) {

        std::size_t local_int_sum = 0;
        std::size_t local_double_sum = 0;
        std::size_t local_string_sum = 0;

        for(double d = start; d < finish; ++d)
        {
            int ret_int = 0;
            double ret_double = 0;
            std::string ret_string;

            while(!q_int_mutex.pull(ret_int));
            local_int_sum += ret_int;

            while(!q_double_mutex.pull(ret_double));
            local_double_sum += ret_double;

            while(!q_string_mutex.pull(ret_string));
            local_string_sum += boost::lexical_cast<std::size_t>(ret_string);
        }

        return std::make_tuple(local_int_sum, local_double_sum, local_string_sum);
    };

    for(std::size_t idx = 0; idx < num_producers; ++idx) {
        constexpr std::size_t chunk_size = iterations / num_producers;
        std::thread(producer_function, chunk_size * idx,
            chunk_size * (idx + 1)).detach();
    }

    for(std::size_t idx = 0; idx < num_consumers; ++idx) {
        constexpr std::size_t chunk_size = iterations / num_consumers;
        results[idx] = std::async(std::launch::async, consumer_function,
            chunk_size * idx, chunk_size * (idx + 1));
    }

    const auto sum = sum_single();
    result_type result_sums;

    for(std::future<result_type> &f : results) {
        result_type res = f.get();
        std::get<0>(result_sums) += std::get<0>(res);
        std::get<1>(result_sums) += std::get<1>(res);
        std::get<2>(result_sums) += std::get<2>(res);
    }

    EXPECT_EQ(sum, std::get<0>(result_sums));
    EXPECT_EQ(sum, std::get<1>(result_sums));
    EXPECT_EQ(sum, std::get<2>(result_sums));

    int ret_int = -99;
    double ret_double = -99.0;
    std::string ret_string = "-99";

    EXPECT_FALSE(q_int_mutex.pull(ret_int));
    EXPECT_FALSE(q_double_mutex.pull(ret_double));
    EXPECT_FALSE(q_string_mutex.pull(ret_string));

    // If the queue is empty - value should not be changed
    EXPECT_EQ(-99, ret_int);
    EXPECT_EQ(-99.0, ret_double);
    EXPECT_EQ(std::string("-99"), ret_string);

    EXPECT_TRUE(q_int_mutex.empty());
    EXPECT_TRUE(q_double_mutex.empty());
    EXPECT_TRUE(q_string_mutex.empty());
}

TEST(ConcurrentQueue, PushPullSwap)
{
    concurrent_queue<std::size_t, std::mutex> queue1, queue2;
    constexpr std::size_t num_tests = 1000000, num_transformations = 10000;

    auto producer_task = [&]() {
        for(std::size_t d = 0; d < num_tests; ++d)
            ASSERT_TRUE(queue1.push(d));
    };

    auto transformer_task = [&]() {
        for(std::size_t d = 0; d < num_transformations; ++d)
            queue1.swap(queue2);
    };

    auto consumer_task = [&]() {
        std::size_t res = 0;

        for(std::size_t d = 0; d < num_tests; ++d) {
            std::size_t ret = 0;
            while(!queue1.pull(ret) && !queue2.pull(ret));
            res += ret;
        }

        EXPECT_TRUE(queue1.empty());
        EXPECT_TRUE(queue2.empty());
        return res;
    };

    auto future = std::async(std::launch::async, consumer_task);
    std::thread(transformer_task).detach();
    producer_task();

    EXPECT_EQ(std::size_t(499999500000), future.get());
}

TEST(ConcurrentQueue, WaitPull)
{
    concurrent_queue<std::size_t, std::mutex> queue1, queue2;
    constexpr std::size_t num_tests = 1000000;

    auto producer_task = [&]() {
        for(std::size_t d = 0; d < num_tests; ++d)
            ASSERT_TRUE(queue1.push(d));
        queue1.close();
    };

    auto transformer_task = [&]() {
        std::size_t res = 0;

        while(queue1.wait_pull(res))
            queue2.push(res);

        EXPECT_FALSE(queue1.empty());
        EXPECT_TRUE(queue1.closed());

        while(queue1.pull(res))
            queue2.push(res);
        queue2.close();

        EXPECT_EQ(num_tests - 1, res);
        EXPECT_TRUE(queue1.empty());
        EXPECT_TRUE(queue1.closed());
    };

    auto consumer_task = [&]() {
        std::size_t res = 0, sum = 0;

        while(queue2.wait_pull(res))
            sum += res;

        //EXPECT_FALSE(queue2.empty());
        EXPECT_TRUE(queue2.closed());

        while(queue2.pull(res))
            sum += res;

        EXPECT_TRUE(queue2.empty());
        EXPECT_TRUE(queue2.closed());
        return sum;
    };

    auto future = std::async(std::launch::async, consumer_task);
    std::thread(transformer_task).detach();
    producer_task();

    EXPECT_EQ(std::size_t(499999500000), future.get());
}

TEST(ConcurrentQueue, WaitPullAbsTime)
{
    auto producer_task = [&](concurrent_queue<std::size_t, std::mutex> &queue) {
        auto now = std::chrono::steady_clock::now();
        queue.push(1);
        std::this_thread::sleep_until(now + std::chrono::milliseconds(50));
        queue.push(2);
        std::this_thread::sleep_until(now + std::chrono::milliseconds(150));
        queue.push(3);
    };

    for(std::size_t delay = 0, idx = 1; idx <= 3; delay += 100, ++idx)
    {
        concurrent_queue<std::size_t, std::mutex> queue;
        std::thread(producer_task, std::ref(queue)).detach();
        auto time_point = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);

        std::size_t res = 0;
        while(queue.wait_pull(time_point, res));
        EXPECT_EQ(idx, res);
    }
}

TEST(ConcurrentQueue, WaitPullRelaTime)
{
    auto producer_task = [&](concurrent_queue<std::size_t, std::mutex> &queue) {
        queue.push(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.push(2);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        queue.push(3);
    };

    for(std::size_t delay = 0, idx = 1; idx <= 3; delay += 100, ++idx)
    {
        concurrent_queue<std::size_t, std::mutex> queue;
        std::thread(producer_task, std::ref(queue)).detach();

        std::size_t res = 0;
        while(queue.wait_pull(std::chrono::milliseconds(delay), res));
        EXPECT_EQ(idx, res);
    }
}
