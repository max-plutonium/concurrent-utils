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
#include <mutex>

#include "../concurrent-helper/concurrent-queue.h"
#include "mock-types.h"

using namespace concurrent_helper;

TEST(ConcurrentQueue, CtorAndDtor)
{
    concurrent_queue<copyable_movable_t<>, dummy_mutex> qq1;
    concurrent_queue<copyable_but_not_movable_t, dummy_mutex> qq2;
    concurrent_queue<not_copyable_but_movable_t, dummy_mutex> qq3;

//  should not be compiled
//  concurrent_queue<not_copyable_not_movable_t, dummy_mutex> qq4;

    concurrent_queue<std::size_t, dummy_mutex> q1;
    concurrent_queue<std::size_t, std::mutex> q2;
    EXPECT_TRUE(q1.empty());
    EXPECT_TRUE(q2.empty());
}

#include <boost/lexical_cast.hpp>

TEST(ConcurrentQueue, PushPopUnsafe)
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

        ASSERT_TRUE(q_int_dummy.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy.pop_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex.pop_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);
    }

    int ret_int = -99;
    double ret_double = -99.0;
    std::string ret_string = "-99";

    EXPECT_FALSE(q_int_dummy.pop_unsafe(ret_int));
    EXPECT_FALSE(q_int_mutex.pop_unsafe(ret_int));
    EXPECT_FALSE(q_double_dummy.pop_unsafe(ret_double));
    EXPECT_FALSE(q_double_mutex.pop_unsafe(ret_double));
    EXPECT_FALSE(q_string_dummy.pop_unsafe(ret_string));
    EXPECT_FALSE(q_string_mutex.pop_unsafe(ret_string));

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

    for(int i = 0; i < num_tests; ++i)
    {
        ASSERT_TRUE(qq1.push_unsafe(i));
        ASSERT_TRUE(qq2.push_unsafe(i));
        ASSERT_TRUE(qq3.push_unsafe(i));
        ASSERT_TRUE(qq4.push_unsafe(i));
    }

    for(int i = 0; i < num_tests; ++i)
    {
        copyable_movable_t<> ret_cm(99);
        copyable_movable_t<false> ret_cm_except(99);
        copyable_but_not_movable_t ret_cnm(99);
        not_copyable_but_movable_t ret_ncm(99);

        ASSERT_TRUE(qq1.pop_unsafe(ret_cm));
        EXPECT_EQ(i, ret_cm.get());
        EXPECT_FALSE(ret_cm.was_copied());
        EXPECT_TRUE(ret_cm.was_moved());

        ASSERT_TRUE(qq2.pop_unsafe(ret_cm_except));
        EXPECT_EQ(i, ret_cm_except.get());
        EXPECT_TRUE(ret_cm_except.was_copied());
        EXPECT_FALSE(ret_cm_except.was_moved());

        ASSERT_TRUE(qq3.pop_unsafe(ret_cnm));
        EXPECT_EQ(i, ret_cnm.get());
        EXPECT_TRUE(ret_cnm.was_copied());
        EXPECT_FALSE(ret_cnm.was_moved());

        ASSERT_TRUE(qq4.pop_unsafe(ret_ncm));
        EXPECT_EQ(i, ret_ncm.get());
        EXPECT_FALSE(ret_ncm.was_copied());
        EXPECT_TRUE(ret_ncm.was_moved());
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
        ASSERT_TRUE(q2.pop_unsafe(ret));
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
        ASSERT_TRUE(q1.pop_unsafe(ret));
        EXPECT_EQ(i + num_tests, ret);
        ASSERT_TRUE(q2.pop_unsafe(ret2));
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

        ASSERT_TRUE(q_int_dummy2.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex2.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy2.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex2.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy2.pop_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex2.pop_unsafe(ret_string));
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
        float ret_float = -99.0;
        unsigned ret_unsigned = 99;

        ASSERT_TRUE(q_float_dummy.pop_unsafe(ret_float));
        EXPECT_EQ(d, ret_float);

        ASSERT_TRUE(q_float_mutex.pop_unsafe(ret_float));
        EXPECT_EQ(d, ret_float);

        ASSERT_TRUE(q_unsigned_dummy.pop_unsafe(ret_unsigned));
        EXPECT_EQ(d, ret_unsigned);

        ASSERT_TRUE(q_unsigned_mutex.pop_unsafe(ret_unsigned));
        EXPECT_EQ(d, ret_unsigned);
    }

    EXPECT_TRUE(q_float_dummy.empty());
    EXPECT_TRUE(q_float_mutex.empty());
    EXPECT_TRUE(q_unsigned_dummy.empty());
    EXPECT_TRUE(q_unsigned_mutex.empty());
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

        ASSERT_TRUE(q_int_dummy2.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex2.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy2.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex2.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy2.pop_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex2.pop_unsafe(ret_string));
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
        float ret_float = -99.0;
        unsigned ret_unsigned = 99;

        ASSERT_TRUE(q_float_dummy.pop_unsafe(ret_float));
        EXPECT_EQ(d, ret_float);

        ASSERT_TRUE(q_float_mutex.pop_unsafe(ret_float));
        EXPECT_EQ(d, ret_float);

        ASSERT_TRUE(q_unsigned_dummy.pop_unsafe(ret_unsigned));
        EXPECT_EQ(d, ret_unsigned);

        ASSERT_TRUE(q_unsigned_mutex.pop_unsafe(ret_unsigned));
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

        ASSERT_TRUE(q_int_dummy2.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex2.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy2.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex2.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy2.pop_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex2.pop_unsafe(ret_string));
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

        ASSERT_TRUE(q_int_dummy3.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex3.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy3.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex3.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy3.pop_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex3.pop_unsafe(ret_string));
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

        ASSERT_TRUE(q_int_dummy2.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex2.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy2.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex2.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy2.pop_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex2.pop_unsafe(ret_string));
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

        ASSERT_TRUE(q_int_dummy3.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_int_mutex3.pop_unsafe(ret_int));
        EXPECT_EQ(d, ret_int);

        ASSERT_TRUE(q_double_dummy3.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_double_mutex3.pop_unsafe(ret_double));
        EXPECT_EQ(d, ret_double);

        ASSERT_TRUE(q_string_dummy3.pop_unsafe(ret_string));
        EXPECT_EQ(boost::lexical_cast<std::string>(d), ret_string);

        ASSERT_TRUE(q_string_mutex3.pop_unsafe(ret_string));
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
    EXPECT_TRUE(q1.pop_unsafe(ret));
    EXPECT_TRUE(q2.pop_unsafe(ret));

    q1.clear();
    EXPECT_TRUE(q1.empty());
    EXPECT_FALSE(q1.pop_unsafe(ret));

    q2.clear();
    EXPECT_TRUE(q2.empty());
    EXPECT_FALSE(q2.pop_unsafe(ret));
}

#include <thread>
#include <atomic>

TEST(ConcurrentQueue, PushPop)
{
    constexpr std::size_t num_producers = 4, num_consumers = 4;
    constexpr std::size_t iterations = 1000000;

    std::vector<std::thread> producer_threads(num_producers),
                             consumer_threads(num_consumers);

    std::size_t result_single = 0;
    std::atomic_size_t result_int(0);
    std::atomic_size_t result_double(0);
    std::atomic_size_t result_string(0);

    concurrent_queue<int, std::mutex> q_int_mutex;
    concurrent_queue<double, std::mutex> q_double_mutex;
    concurrent_queue<std::string, std::mutex> q_string_mutex;

    auto sum_single = [&]() {
        for(std::size_t i = 0; i < iterations; ++i)
            result_single += i;
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

            while(!q_int_mutex.pop(ret_int));
            local_int_sum += ret_int;

            while(!q_double_mutex.pop(ret_double));
            local_double_sum += ret_double;

            while(!q_string_mutex.pop(ret_string));
            local_string_sum += boost::lexical_cast<std::size_t>(ret_string);
        }

        result_int.fetch_add(local_int_sum, std::memory_order_acq_rel);
        result_double.fetch_add(local_double_sum, std::memory_order_acq_rel);
        result_string.fetch_add(local_string_sum, std::memory_order_acq_rel);
    };

    for(std::size_t idx = 0; idx < producer_threads.size(); ++idx) {
        constexpr std::size_t chunk_size = iterations / num_producers;
        producer_threads[idx] = std::thread { std::bind(producer_function, chunk_size * idx, chunk_size * (idx + 1)) };
    }

    for(std::size_t idx = 0; idx < consumer_threads.size(); ++idx) {
        constexpr std::size_t chunk_size = iterations / num_consumers;
        consumer_threads[idx] = std::thread { std::bind(consumer_function, chunk_size * idx, chunk_size * (idx + 1)) };
    }

    sum_single();

    for(std::thread &thr : producer_threads) thr.join();
    for(std::thread &thr : consumer_threads) thr.join();

    EXPECT_EQ(result_single, result_int.load(std::memory_order_relaxed));
    EXPECT_EQ(result_single, result_double.load(std::memory_order_relaxed));
    EXPECT_EQ(result_single, result_string.load(std::memory_order_relaxed));

    std::cerr << "result_single = " << result_single << std::endl;
    std::cerr << "result_int = " << result_int.load(std::memory_order_relaxed) << std::endl;
    std::cerr << "result_double = " << result_double.load(std::memory_order_relaxed) << std::endl;
    std::cerr << "result_string = " << result_string.load(std::memory_order_relaxed) << std::endl;

    int ret_int = -99;
    double ret_double = -99.0;
    std::string ret_string = "-99";

    EXPECT_FALSE(q_int_mutex.pop(ret_int));
    EXPECT_FALSE(q_double_mutex.pop(ret_double));
    EXPECT_FALSE(q_string_mutex.pop(ret_string));

    // If the queue is empty - value should not be changed
    EXPECT_EQ(-99, ret_int);
    EXPECT_EQ(-99.0, ret_double);
    EXPECT_EQ(std::string("-99"), ret_string);

    EXPECT_TRUE(q_int_mutex.empty());
    EXPECT_TRUE(q_double_mutex.empty());
    EXPECT_TRUE(q_string_mutex.empty());
}
