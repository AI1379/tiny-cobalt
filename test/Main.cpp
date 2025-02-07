#include <cpptrace/basic.hpp>
#include <cpptrace/cpptrace.hpp>
#include <cpptrace/from_current.hpp>
#include <csignal>
#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char **argv) {
    std::signal(
            SIGSEGV, +[](int) {
                std::cerr << "Segmentation fault" << std::endl;
                cpptrace::stacktrace::current().print_with_snippets();
                std::exit(SIGSEGV);
            });
    ::testing::InitGoogleTest(&argc, argv);
    int res = 0;
    CPPTRACE_TRY { res = RUN_ALL_TESTS(); }
    CPPTRACE_CATCH(const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        cpptrace::from_current_exception().print();
        res = -1;
    }
    return res;
}