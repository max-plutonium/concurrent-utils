#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <omp.h>

#include "benchmark.h"

namespace details {

benchmark_cpuclock_timer::benchmark_cpuclock_timer()
    : start_time(0)
{
#ifdef __unix__
    if(clock_getcpuclockid(getpid(), &clockid) != 0)
        perror("benchmark_cpuclock_timer: clock_getcpuclockid");
#elif defined __MINGW32__
    clockid = CLOCK_PROCESS_CPUTIME_ID;
#else
#   error "platform not supported"
#endif
}

void benchmark_cpuclock_timer::start()
{
    timespec ts;
    if(clock_gettime(clockid, &ts) == -1)
        perror("benchmark_cpuclock_timer: clock_gettime");
    start_time = double(ts.tv_sec) + double(ts.tv_nsec) / 1e9;
}

double benchmark_cpuclock_timer::elapsed()
{
    timespec ts;
    if(clock_gettime(clockid, &ts) == -1)
        perror("benchmark_timer: clock_gettime");
    auto time = double(ts.tv_sec) + double(ts.tv_nsec) / 1e9;
    return start_time ? time - start_time : 0.0;
}


benchmark_omp_timer::benchmark_omp_timer()
    : start_time(0)
{
}

void benchmark_omp_timer::start()
{
    start_time = omp_get_wtime();
}

double benchmark_omp_timer::elapsed()
{
    return start_time ? omp_get_wtime() - start_time : 0.0;
}


benchmark_controller::benchmark_controller(
        const char *aname, std::uint32_t aiterations)
    : name(aname), iteration(0), iterations(aiterations)
{
    fprintf(stderr,
            "\n************************************\n"
            "start  benchmark \"%s\" for %u iterations\n",
            name, iterations);
    omp_timer.start();
    cpu_timer.start();
}

benchmark_controller::~benchmark_controller()
{
    const auto omp_elapsed = omp_timer.elapsed();
    const auto cpu_elapsed = cpu_timer.elapsed();
    fprintf(stderr,
        "finish benchmark \"%s\" for %u iterations\n"
        "cpu  time %0.9f (%0.9f per iteration)\n"
        "full time %0.9f (%0.9f per iteration)\n"
        "*************************************\n",
        name, iterations,
        cpu_elapsed, cpu_elapsed / iterations,
        omp_elapsed, omp_elapsed / iterations);
}

bool benchmark_controller::is_done()
{
    return (iteration++) == iterations;
}

} // namespace details
