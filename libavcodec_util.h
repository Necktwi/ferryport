/* 
 * File:   libavcodec-example.h
 * Author: gowtham
 *
 * Created on 4 October, 2013, 3:12 PM
 */

#ifndef LIBAVCODEC_UTIL_H
#define	LIBAVCODEC_UTIL_H

#include "MediaManager.h"
#include "mypcm.h"
#include "capture.h"
#include <string>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/samplefmt.h>
}

int libavcodec_example(int argc, char **argv);

struct libav_encode_args {
    AVCodecID codecID;
    AVSampleFormat av_smpl_fmt;
    int bitrate;
    int samplingFrequency;
    MediaManager::media::ferrybuffer input_buffer;
    int initptr;
    int termptr;
    char* output_buffer;
    int output_buffer_size;
};

void audio_encode(libav_encode_args args);

#endif	/* LIBAVCODEC_EXAMPLE_H */

