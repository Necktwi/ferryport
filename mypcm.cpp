#define ALSA_PCM_NEW_HW_PARAMS_API

#include "mypcm.h"
#include "debug.h"
#include "mystdlib.h"
#include "logger.h"
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <stdio.h>
#include <valarray>
#include <stdint.h>
#include <iostream>

void ferryperiod::initferryperiod(int length, short wordsize) {
    free(period);
    this->period = (char*) malloc(length);
    this->length = length;
    this->wordsize = wordsize;
}

ferryperiod::ferryperiod() {
    period = NULL;
    wordsize = 1;
    length = 0;
}

ferryperiod::~ferryperiod() {
    free(period);
}

void VFerryPeriod::initferryperiod(int length, short wordsize) {
    free(period.iov_base);
    this->period.iov_base = (char*) malloc(length);
    this->period.iov_len = length;
    this->wordsize = wordsize;
}

VFerryPeriod::VFerryPeriod() {
    period.iov_base = NULL;
    period.iov_len = 0;
    wordsize = 1;
}

VFerryPeriod::~VFerryPeriod() {
    free(period.iov_base);
}

int pcm_types_n_formats() {
    int val;

    printf("ALSA library version: %s\n", SND_LIB_VERSION_STR);

    printf("\nPCM stream types:\n");
    for (val = 0; val <= SND_PCM_STREAM_LAST; val++)
        printf("  %s\n",
            snd_pcm_stream_name((snd_pcm_stream_t) val));

    printf("\nPCM access types:\n");
    for (val = 0; val <= SND_PCM_ACCESS_LAST; val++)
        printf("  %s\n",
            snd_pcm_access_name((snd_pcm_access_t) val));

    printf("\nPCM formats:\n");
    for (val = 0; val <= SND_PCM_FORMAT_LAST; val++)
        if (snd_pcm_format_name((snd_pcm_format_t) val)
                != NULL)
            printf("  %s (%s)\n",
                snd_pcm_format_name((snd_pcm_format_t) val),
                snd_pcm_format_description(
                (snd_pcm_format_t) val));

    printf("\nPCM subformats:\n");
    for (val = 0; val <= SND_PCM_SUBFORMAT_LAST;
            val++)
        printf("  %s (%s)\n",
            snd_pcm_subformat_name((
            snd_pcm_subformat_t) val),
            snd_pcm_subformat_description((
            snd_pcm_subformat_t) val));

    printf("\nPCM states:\n");
    for (val = 0; val <= SND_PCM_STATE_LAST; val++) {
        printf("  %s\n", snd_pcm_state_name((snd_pcm_state_t) val));
    }

    return 0;
}

int pcm_open_set_device() {
    int rc;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_format_t val3;
    unsigned int val, val2;
    int dir;
    snd_pcm_uframes_t frames;

    /* Open PCM device for playback. */
    //plughw:U0x46d0x825,0
    //hw:1,0
    rc = snd_pcm_open(&handle, "plughw:1,0", SND_PCM_STREAM_PLAYBACK, 0);
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
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_NONINTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(handle, params, 2);

    /* 44100 bits/second sampling rate (CD quality) */
    val = 44100;
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(1);
    }

    /* Display information about the PCM interface */

    printf("PCM handle name = '%s'\n", snd_pcm_name(handle));

    printf("PCM state = %s\n", snd_pcm_state_name(snd_pcm_state(handle)));

    snd_pcm_hw_params_get_access(params, (snd_pcm_access_t *) & val);
    printf("access type = %s\n", snd_pcm_access_name((snd_pcm_access_t) val));

    snd_pcm_hw_params_get_format(params, (snd_pcm_format_t*) & val);
    printf("format = '%s' (%s)\n", snd_pcm_format_name((snd_pcm_format_t) val), snd_pcm_format_description((snd_pcm_format_t) val));

    snd_pcm_hw_params_get_subformat(params, (snd_pcm_subformat_t *) & val);
    printf("subformat = '%s' (%s)\n", snd_pcm_subformat_name((snd_pcm_subformat_t) val), snd_pcm_subformat_description((snd_pcm_subformat_t) val));

    snd_pcm_hw_params_get_channels(params, &val);
    printf("channels = %d\n", val);

    snd_pcm_hw_params_get_rate(params, &val, &dir);
    printf("rate = %d sps\n", val);

    snd_pcm_hw_params_get_period_time(params,
            &val, &dir);
    printf("period time = %d us\n", val);

    snd_pcm_hw_params_get_period_size(params,
            &frames, &dir);
    printf("period size = %d frames\n", (int) frames);

    snd_pcm_hw_params_get_buffer_time(params,
            &val, &dir);
    printf("buffer time = %d us\n", val);

    snd_pcm_hw_params_get_buffer_size(params,
            (snd_pcm_uframes_t *) & val);
    printf("buffer size = %d frames\n", val);

    snd_pcm_hw_params_get_periods(params, &val, &dir);
    printf("periods per buffer = %d frames\n", val);

    snd_pcm_hw_params_get_rate_numden(params,
            &val, &val2);
    printf("exact rate = %d/%d bps\n", val, val2);

    val = snd_pcm_hw_params_get_sbits(params);
    printf("significant bits = %d\n", val);

    snd_pcm_hw_params_get_tick_time(params, &val, &dir);
    printf("tick time = %d us\n", val);

    val = snd_pcm_hw_params_is_batch(params);
    printf("is batch = %d\n", val);

    val = snd_pcm_hw_params_is_block_transfer(params);
    printf("is block transfer = %d\n", val);

    val = snd_pcm_hw_params_is_double(params);
    printf("is double = %d\n", val);

    val = snd_pcm_hw_params_is_half_duplex(params);
    printf("is half duplex = %d\n", val);

    val = snd_pcm_hw_params_is_joint_duplex(params);
    printf("is joint duplex = %d\n", val);

    val = snd_pcm_hw_params_can_overrange(params);
    printf("can overrange = %d\n", val);

    val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
    printf("can mmap = %d\n", val);

    val = snd_pcm_hw_params_can_pause(params);
    printf("can pause = %d\n", val);

    val = snd_pcm_hw_params_can_resume(params);
    printf("can resume = %d\n", val);

    val = snd_pcm_hw_params_can_sync_start(params);
    printf("can sync start = %d\n", val);

    snd_pcm_close(handle);

    return 0;
}

/*

This example reads standard from input and writes
to the default PCM device for 5 seconds of data.

 */

/* Use the newer ALSA API */


int playback(char* inputFileName) {
    long loops;
    int rc;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;
    int f = 0;
    if (strlen(inputFileName)) {
        f = open(inputFileName, O_RDONLY);
    }
    /* Open PCM device for playback. */
    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
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
    frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        pthread_exit(NULL);
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * 4; /* 2 bytes/sample, 2 channels */
    buffer = (char *) malloc(size);

    /* We want to loop for 5 seconds */
    snd_pcm_hw_params_get_period_time(params, &val, &dir);
    /* 5 seconds in microseconds divided by
     * period time */
    loops = 10 * 30000000 / val;

    while (loops > 0) {
        loops--;
        rc = read(f, buffer, size);
        if (rc == 0) {
            fprintf(stderr, "end of file on input\n");
            break;
        } else if (rc != size) {
            fprintf(stderr, "short read: read %d bytes\n", rc);
        }
        rc = snd_pcm_writei(handle, buffer, frames);
        if (rc == -EPIPE) {
            /* EPIPE means underrun */
            fprintf(stderr, "underrun occurred\n");
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            fprintf(stderr,
                    "error from writei: %s\n",
                    snd_strerror(rc));
        } else if (rc != (int) frames) {
            fprintf(stderr, "short write, write %d frames\n", rc);
        }
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
    if (f)close(f);
    return 0;
}

void deallocate_srarg(void* buffer) {
    struct snd_record_args* args = (snd_record_args*) buffer;
    *args->returnObj.state = -1;
};

void * snd_record(void* voidargs) {
    pthread_cleanup_push(deallocate_srarg, voidargs);
    struct snd_record_args* args = (snd_record_args*) voidargs;
    *args->returnObj.state = 0;
    long loops;
    int rc;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    uint16_t *buffer;
    int* signalNewState = args->signalNewState;
    bool continuous_capture = args->duration <= 0 ? true : false;
    /* Open PCM device for recording (capture). */
    rc = snd_pcm_open(&handle, (const char*) args->dev_name.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        ffl_err(FPOL_PCM, "unable to open pcm device: %s", snd_strerror(rc));
        ffl_debug(FPOL_PCM, "dev_name: %s", (char*) args->dev_name.c_str());
        args->returnObj.errorcode = 1;
        //pthread_cleanup_pop(1);
        //return NULL;
        pthread_exit(NULL);
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

    /* 44100 samples/second sampling rate (CD quality) */
    val = args->samplingFrequency;
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

    /* Set period size to 32 frames. */
    frames = 1152; //32; //
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        std::cout << "\n" << getTime() << " snd_record: unable to set hw parameters: " << snd_strerror(rc) << "\n";
        pthread_exit(NULL);
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * 4; /* 2 bytes/sample, 2 channels */
    buffer = (uint16_t*) malloc(size);

    /* We want to loop for 5 seconds */
    snd_pcm_hw_params_get_period_time(params, &val, &dir);
    loops = args->duration * 1000000 / val;
    *args->returnObj.state = 1;
    while (*signalNewState >= 0 && (loops > 0 || continuous_capture)) {
        loops--;
        rc = snd_pcm_readi(handle, (void**) buffer, frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            args->returnObj.error = std::string("error from read: ") + std::string(snd_strerror(rc));
            std::cout << "\n" << getTime() << " snd_record(): error from read: " << snd_strerror(rc) << "\n";
            break;
        } else if (rc != (int) frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
        (*(args->periodbuffer))[*args->periodbufferfloat].initferryperiod(size, 2);
        memcpy((*args->periodbuffer)[*args->periodbufferfloat].period, buffer, (*args->periodbuffer)[*args->periodbufferfloat].length);
        (*args->periodbufferfloat)++;
        (*args->periodbufferfloat) %= *args->periodbufferlength;
        //        if (rc != size) {
        //            fprintf(stderr, "short write: wrote %d bytes\n", rc);
        //        }
    }
    *args->returnObj.state = -1;
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
    pthread_cleanup_pop(1);
    return NULL;
}