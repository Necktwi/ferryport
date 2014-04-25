/* 
 * File:   rawPlayer.cpp
 * Author: gowtham
 *
 * Created on 23 Apr, 2014, 2:31:33 PM
 */

#include "mypcm.h"
#include <stdlib.h>
#include <iostream>

/*
 * Simple C++ Test Suite
 */

void test1() {
    std::cout << "rawPlayer test 1" << std::endl;
    ::playback("ferrymediacapture1/audio.swav");
}

void test2() {
    std::cout << "rawPlayer test 2" << std::endl;
    std::cout << "%TEST_FAILED% time=0 testname=test2 (rawPlayer) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% rawPlayer" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (rawPlayer)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (rawPlayer)" << std::endl;

    std::cout << "%TEST_STARTED% test2 (rawPlayer)\n" << std::endl;
    test2();
    std::cout << "%TEST_FINISHED% time=0 test2 (rawPlayer)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

