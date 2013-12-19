/* 
 * File:   libavproc_test.cpp
 * Author: gowtham
 *
 * Created on 7 Oct, 2013, 4:41:14 PM
 */

#include <stdlib.h>
#include <iostream>

/*
 * Simple C++ Test Suite
 */

void test1() {
    std::cout << "libavproc_test test 1" << std::endl;
}

void test2() {
    std::cout << "libavproc_test test 2" << std::endl;
    std::cout << "%TEST_FAILED% time=0 testname=test2 (libavproc_test) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% libavproc_test" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (libavproc_test)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (libavproc_test)" << std::endl;

    std::cout << "%TEST_STARTED% test2 (libavproc_test)\n" << std::endl;
    test2();
    std::cout << "%TEST_FINISHED% time=0 test2 (libavproc_test)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

