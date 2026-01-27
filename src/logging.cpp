#include "logging.hpp"

#include <iomanip>
#include <iostream>

#include "dictionary.hpp"

Logger::Logger(const Dict &answers, const Dict &guesses)
{
    total_iterations = (long long)answers.getLength() * guesses.getLength() * (guesses.getLength() - 1) / 2;

    std::cout << answers.getLength() << " answers\t" << guesses.getLength() << " guesses" << std::endl;
    std::cout << "Total iterations: " << total_iterations << std::endl;
}

void Logger::logCompletedIteration()
{
    std::lock_guard<std::mutex> lock(mutex);
    completed_iterations++;
}

void Logger::displayProgress()
{
    std::lock_guard<std::mutex> lock(mutex);

    if (difftime(time(NULL), last_print_time) < 1)
        return;

    last_print_time = time(NULL);

    std::cout << std::fixed << "\r"
              << completed_iterations << "/" << total_iterations << "\t"
              << std::setprecision(2) << "[" << 100.0 * completed_iterations / total_iterations << "% completed]\t"
              << std::flush;
}
