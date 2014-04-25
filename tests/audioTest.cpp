/* 
 * File:   audioTest.cpp
 * Author: gowtham
 *
 * Created on 16 Apr, 2014, 6:13:36 PM
 */

#include "audio.h"
#include "libavcodec_util.h"
#include "libavutil/samplefmt.h"
#include "libavcodec/avcodec.h"
#include "mypcm.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>

/*
 * Simple C++ Test Suite
 */

void test1() {
    std::cout << "audioTest test 1" << std::endl;
    setuid(1000);
    //audio::record(NULL);
    std::valarray<ferryperiod> fp(60000);
    int pbl = 60000;
    int pbf = 0;
    short state = 0;
    snd_record_return srr;
    srr.state = &state;
    snd_record_args sargs = {"default", 44100, 30, &fp, &pbl, &pbf, srr};
    ::snd_record((void*) & sargs);
    std::ofstream testrecord;
    testrecord.open("testarecord.swav", std::ios_base::out | std::ios_base::binary);
    for (int i = 0; i < fp.size(); i++) {
        testrecord.write(fp[i].period, fp[i].length);
    }
    testrecord.close();
    std::cout << "\nno. of period bufs: " << pbf << std::endl;
    std::cout << "\nlength of period: " << fp[0].length << std::endl;

    /*audio playback*/
    ::playback("testarecord.swav");

    /*mp3 encoding*/
    libav_encode_args lavea;
    lavea.codecID = AV_CODEC_ID_MP3;
    lavea.av_smpl_fmt = AV_SAMPLE_FMT_S16P;
    lavea.bitrate = 64000;
    lavea.samplingFrequency = 44100;
    lavea.input_buffer.periodbuffer = &fp;
    lavea.output_buffer = NULL;
    lavea.initptr = 0;
    lavea.termptr = pbf-1;
    audio_encode(&lavea);
    std::ofstream testrecordmp3;
    testrecordmp3.open("testarecord.mp3", std::ios_base::out | std::ios_base::binary);
    testrecordmp3.write(lavea.output_buffer, lavea.output_buffer_size);
    testrecordmp3.close();
}

void test2() {
    std::cout << "audioTest test 2" << std::endl;
    std::cout << "%TEST_FAILED% time=0 testname=test2 (audioTest) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% audioTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (audioTest)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (audioTest)" << std::endl;

    std::cout << "%TEST_STARTED% test2 (audioTest)\n" << std::endl;
    test2();
    std::cout << "%TEST_FINISHED% time=0 test2 (audioTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

