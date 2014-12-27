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
#ifndef CONCURRENT_UTILS_BENCHMARK_H
#define CONCURRENT_UTILS_BENCHMARK_H

#include <cstdint>

namespace details {

#include <stdlib.h>
#include <time.h>

class benchmark_cpuclock_timer
{
    clockid_t clockid;
    double start_time;

public:
    benchmark_cpuclock_timer();
    void start();
    double elapsed();
};


class benchmark_omp_timer
{
    double start_time;

public:
    benchmark_omp_timer();
    void start();
    double elapsed();
};


class benchmark_controller
{
    const char *name;
    std::uint32_t iteration;
    const std::uint32_t iterations;
    benchmark_cpuclock_timer cpu_timer;
    benchmark_omp_timer omp_timer;

public:
    benchmark_controller(const char *aname, std::uint32_t aiterations);
    ~benchmark_controller();
    bool is_done();
};

} // namespace details

#define benchmark(name, iterations) \
        for(details::benchmark_controller __benchmark(name, iterations); \
            !__benchmark.is_done();)

#endif // CONCURRENT_UTILS_BENCHMARK_H
