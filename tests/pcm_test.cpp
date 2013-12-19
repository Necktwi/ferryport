/* 
 * File:   pcm_test.cpp
 * Author: gowtham
 *
 * Created on 18 Nov, 2013, 10:04:55 PM
 */

#include <stdlib.h>
#include <iostream>
#include "mypcm.h"
/*
 * Simple C++ Test Suite
 */
/*

This example reads from the default PCM device
and writes to standard output for 5 seconds of data.

 */

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

//int pcm_open_set_device() {
//    int rc;
//    snd_pcm_t *handle;
//    snd_pcm_hw_params_t *params;
//    snd_pcm_format_t val3;
//    unsigned int val, val2;
//    int dir;
//    snd_pcm_uframes_t frames;
//
//    /* Open PCM device for playback. */
//    //plughw:U0x46d0x825,0
//    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
//    if (rc < 0) {
//        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
//        exit(1);
//    }
//
//    /* Allocate a hardware parameters object. */
//    snd_pcm_hw_params_alloca(&params);
//
//    /* Fill it in with default values. */
//    snd_pcm_hw_params_any(handle, params);
//
//    /* Set the desired hardware parameters. */
//
//    /* Interleaved mode */
//    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
//
//    /* Signed 16-bit little-endian format */
//    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
//
//    /* Two channels (stereo) */
//    snd_pcm_hw_params_set_channels(handle, params, 2);
//
//    /* 44100 bits/second sampling rate (CD quality) */
//    val = 44100;
//    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
//
//    /* Write the parameters to the driver */
//    rc = snd_pcm_hw_params(handle, params);
//    if (rc < 0) {
//        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
//        exit(1);
//    }
//
//    /* Display information about the PCM interface */
//
//    printf("PCM handle name = '%s'\n", snd_pcm_name(handle));
//
//    printf("PCM state = %s\n", snd_pcm_state_name(snd_pcm_state(handle)));
//
//    snd_pcm_hw_params_get_access(params, (snd_pcm_access_t *) & val);
//    printf("access type = %s\n", snd_pcm_access_name((snd_pcm_access_t) val));
//
//    snd_pcm_hw_params_get_format(params, (snd_pcm_format_t*) & val);
//    printf("format = '%s' (%s)\n", snd_pcm_format_name((snd_pcm_format_t) val), snd_pcm_format_description((snd_pcm_format_t) val));
//
//    snd_pcm_hw_params_get_subformat(params, (snd_pcm_subformat_t *) & val);
//    printf("subformat = '%s' (%s)\n", snd_pcm_subformat_name((snd_pcm_subformat_t) val), snd_pcm_subformat_description((snd_pcm_subformat_t) val));
//
//    snd_pcm_hw_params_get_channels(params, &val);
//    printf("channels = %d\n", val);
//
//    snd_pcm_hw_params_get_rate(params, &val, &dir);
//    printf("rate = %d bps\n", val);
//
//    snd_pcm_hw_params_get_period_time(params,
//            &val, &dir);
//    printf("period time = %d us\n", val);
//
//    snd_pcm_hw_params_get_period_size(params,
//            &frames, &dir);
//    printf("period size = %d frames\n", (int) frames);
//
//    snd_pcm_hw_params_get_buffer_time(params,
//            &val, &dir);
//    printf("buffer time = %d us\n", val);
//
//    snd_pcm_hw_params_get_buffer_size(params,
//            (snd_pcm_uframes_t *) & val);
//    printf("buffer size = %d frames\n", val);
//
//    snd_pcm_hw_params_get_periods(params, &val, &dir);
//    printf("periods per buffer = %d frames\n", val);
//
//    snd_pcm_hw_params_get_rate_numden(params,
//            &val, &val2);
//    printf("exact rate = %d/%d bps\n", val, val2);
//
//    val = snd_pcm_hw_params_get_sbits(params);
//    printf("significant bits = %d\n", val);
//
//    snd_pcm_hw_params_get_tick_time(params, &val, &dir);
//    printf("tick time = %d us\n", val);
//
//    val = snd_pcm_hw_params_is_batch(params);
//    printf("is batch = %d\n", val);
//
//    val = snd_pcm_hw_params_is_block_transfer(params);
//    printf("is block transfer = %d\n", val);
//
//    val = snd_pcm_hw_params_is_double(params);
//    printf("is double = %d\n", val);
//
//    val = snd_pcm_hw_params_is_half_duplex(params);
//    printf("is half duplex = %d\n", val);
//
//    val = snd_pcm_hw_params_is_joint_duplex(params);
//    printf("is joint duplex = %d\n", val);
//
//    val = snd_pcm_hw_params_can_overrange(params);
//    printf("can overrange = %d\n", val);
//
//    val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
//    printf("can mmap = %d\n", val);
//
//    val = snd_pcm_hw_params_can_pause(params);
//    printf("can pause = %d\n", val);
//
//    val = snd_pcm_hw_params_can_resume(params);
//    printf("can resume = %d\n", val);
//
//    val = snd_pcm_hw_params_can_sync_start(params);
//    printf("can sync start = %d\n", val);
//
//    snd_pcm_close(handle);
//
//    return 0;
//}

int pcm_record() {
    long loops;
    int rc;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;

    /* Open PCM device for recording (capture). */
    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(handle, params, 2);

    /* 44100 bits/second sampling rate (CD quality) */
    val = 44100;
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

    /* Set period size to 32 frames. */
    frames = 1152; //32;
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(1);
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * 4; /* 2 bytes/sample, 2 channels */
    buffer = (char *) malloc(size);

    /* We want to loop for 5 seconds */
    snd_pcm_hw_params_get_period_time(params, &val, &dir);
    loops = 5000000 / val;

    while (loops > 0) {
        loops--;
        rc = snd_pcm_readi(handle, buffer, frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != (int) frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
        rc = write(1, buffer, size);
        if (rc != size)
            fprintf(stderr, "short write: wrote %d bytes\n", rc);
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);

    return 0;
}

void test1() {
    std::cout << "pcm_test test 1" << std::endl;
    //pcm_record();
    pcm_open_set_device();
}

void test2() {
    std::cout << "pcm_test test 2" << std::endl;
    std::cout << "%TEST_FAILED% time=0 testname=test2 (pcm_test) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% pcm_test" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (pcm_test)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (pcm_test)" << std::endl;

    std::cout << "%TEST_STARTED% test2 (pcm_test)\n" << std::endl;
    test2();
    std::cout << "%TEST_FINISHED% time=0 test2 (pcm_test)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

