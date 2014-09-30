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
    AVCodecID codecID; //*< codec id for example AV_CODEC_ID_MP3 encodes raw input to mp3
    AVSampleFormat av_smpl_fmt; //*< input samples format eg. AV_SAMPLE_FMT_S16(interleaved) AV_SAMPLE_FMT_S16P(non interleaved)*/
    int bitrate; //*< 64000 (64kbps) is ideal for voice; 128kbps and 320kbps for music
    int samplingFrequency; //*< 44100 samples/sec for cd quality
    union MediaManager::media::ferrybuffer input_buffer; //*< @see MediaManager::media::ferrybuffer union; handles both audio and video buffers which are treated as circular buffers
    int initptr; //*< audio_encode() considers this value as start point of valid data in input_buffer
    int termptr; //*< audio_encode() considers this value as end point of valid data in input_buffer
    char* output_buffer; //*< audio_encode() returns a pointer to encoded data here. so don't initiate it!
    int output_buffer_size; //*< audio_encode() assings the size of the encoded data here. so don't initiate it!
};

void audio_encode(libav_encode_args* a);

#endif	/* LIBAVCODEC_EXAMPLE_H */

