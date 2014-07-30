/* 
 * File:   MediaManager.cpp
 * Author: gowtham
 * 
 * Created on 23 October, 2013, 11:35 AM
 */

#include "MediaManager.h"
#include "capture.h"
#include "mypcm.h"
#include "libavcodec_util.h"
#include "ClientSocket.h"
#include "SocketException.h"
#include "myconverters.h"
#include "debug.h"
#include "mystdlib.h"
#include "logger.h"
#include <string>
#include <sys/socket.h>
#include <stdlib.h>
#include <iostream>
#include <valarray>
#include <fcntl.h>
#include <sys/stat.h>
#include <libavcodec/avcodec.h>
#include <unistd.h>
#include<time.h>
#include <fstream>

MediaManager::MediaManager() {

}

MediaManager::MediaManager(const MediaManager& orig) {

}

MediaManager::~MediaManager() {
}

MediaManager::capture_thread_iargs::~capture_thread_iargs() {
    delete this->inputs;
    delete this->outputs;
}

std::string MediaManager::MediaTypeString[] = {"AUDIO", "VIDEO"};
std::string MediaManager::EncodingString[] = {"mjpeg", "mp3"};

int MediaManager::capture(std::valarray<MediaManager::media>& inputs, std::valarray<MediaManager::media>& outputs) {
    capture_thread_iargs ctsa;
    ctsa.inputs = &inputs;
    ctsa.outputs = &outputs;
    capture_thread_rargs * crargs = (capture_thread_rargs*) capture(&ctsa);
    int ret = crargs->retvalue;
    delete crargs;
    return ret;
}

void MediaManager::pthreadCleanupCapture(void* args) {
    pthreadCleanupCaptureArgs& pcca = *(pthreadCleanupCaptureArgs*) args;
    ffl_debug(FPOL_MM, "terminating output threads");
    int i = 0;
    while (pcca.oupthreads.size() > 0) {
        pthread_t* thread = pcca.oupthreads[pcca.oupthreads.size() - 1];
        pthread_cancel(*thread);
        pthread_join(*thread, NULL);
        pcca.oupthreads.pop_back();
        delete thread;
    }
    ffl_debug(FPOL_MM, "terminating input threads");
    i = 0;
    timespec ts = {0, 0};
    while (pcca.inpthreads.size() > 0) {
        clock_gettime(CLOCK_REALTIME, &ts);
        pthread_cancel(*pcca.inpthreads[pcca.inpthreads.size() - 1]);
        pthread_join(*pcca.inpthreads[pcca.inpthreads.size() - 1], NULL);
        pcca.inpthreads.pop_back();
    }
    ffl_debug(FPOL_MM, "freeing up buffers");
    i = 0;
    while (i < (*pcca.inputs).size()) {
        if ((*pcca.inputs)[i].type == VIDEO)delete (*pcca.inputs)[i].buffer.framebuffer;
        else if ((*pcca.inputs)[i].type == AUDIO)delete (*pcca.inputs)[i].buffer.periodbuffer;
        i++;
    }
    i = 0;
    while (pcca.expired_objs.size() > 0) {
        delete pcca.expired_objs[pcca.expired_objs.size() - 1];
        pcca.expired_objs.pop_back();
    }
}

void* MediaManager::capture(void * args) {
    capture_thread_iargs * ctsa = (capture_thread_iargs*) args;
    capture_thread_rargs * ctsr = new capture_thread_rargs;
    std::valarray<MediaManager::media>& inputs = *ctsa->inputs;
    std::valarray<MediaManager::media>& outputs = *ctsa->outputs;
    int i = 0;
    int audioTypeInputs[10];
    int videoTypeInputs[10];
    int atic = 0;
    int vtic = 0;
    int threadCount = 0;
    pthreadCleanupCaptureArgs pcca;
    pcca.inputs = &inputs;
    pthread_cleanup_push(pthreadCleanupCapture, (void*) &pcca);
    while (i < inputs.size()) {
        if (inputs[i].type == MediaManager::VIDEO) {
            videoTypeInputs[vtic++] = i;
            pthread_t * capture_thread = (pthread_t*) malloc(sizeof (pthread_t));
            pcca.inpthreads.push_back(capture_thread);
            capture_args *cargs = new capture_args();
            inputs[i].buffer.framebuffer = new std::valarray<ferryframe>(200);
            cargs->framebuffer = inputs[i].buffer.framebuffer;
            //memset((void*) cargs, 0, sizeof (capture_args));
            cargs->Height = inputs[i].height;
            cargs->Width = inputs[i].width;
            cargs->device = inputs[i].identifier;
            cargs->format = MediaManager::EncodingString[inputs[i].encoding];
            cargs->mmap = true;
            cargs->read = false;
            cargs->userp = false;
            cargs->readerscount = sizeof (outputs) / sizeof (outputs[0]);
            cargs->bufferfloat = &inputs[i].bufferfloat;
            cargs->buffersize = &inputs[i].buffersize;
            cargs->duration = inputs[i].duration;
            inputs[i].bufferfloat = 0;
            inputs[i].buffersize = 200;
            cargs->framerate = inputs[i].videoframerate;
            cargs->returnObj.state = &inputs[i].state;
            cargs->signalNewState = &inputs[i].signalNewState;
            inputs[i].signalNewState = 1;
            //videocapture(cargs);
            inputs[i].videoframerate = cargs->framerate;
            pcca.expired_objs.push_back((void*) cargs);
            if (pthread_create(capture_thread, NULL, &videocapture, cargs) != 0) {
                ctsr->retvalue = -1;
                return (void*) ctsr;
            }
        } else if (inputs[i].type == MediaManager::AUDIO) {
            audioTypeInputs[atic++] = i;
            pthread_t *snd_record_thread = (pthread_t*) malloc(sizeof (pthread_t));
            pcca.inpthreads.push_back(snd_record_thread);
            inputs[i].buffer.periodbuffer = new std::valarray<ferryperiod>(500);
            snd_record_args* srargs = new snd_record_args();
            //memset((void*) &srargs, 0, sizeof (snd_record_args));
            inputs[i].bufferfloat = 0;
            inputs[i].buffersize = 500;
            srargs->samplingFrequency = inputs[i].audioSamplingFrequency;
            srargs->duration = inputs[i].duration;
            srargs->dev_name = inputs[i].identifier;
            srargs->periodbuffer = inputs[i].buffer.periodbuffer; //(ferryperiod**) calloc(srargs.periodbufferlength, sizeof (ferryperiod*));
            srargs->periodbufferfloat = &inputs[i].bufferfloat;
            srargs->periodbufferlength = &inputs[i].buffersize;
            srargs->returnObj.state = &inputs[i].state;
            srargs->signalNewState = &inputs[i].signalNewState;
            inputs[i].signalNewState = 1;
            pcca.expired_objs.push_back((void*) srargs);
            if (pthread_create(snd_record_thread, NULL, &snd_record, srargs) != 0) {
                ctsr->retvalue = -1;
                return (void*) ctsr;
            }
            //snd_record(srargs);
        }
        i++;
    }
    usleep(100000);
    i = 0;
    while (i < outputs.size()) {
        if ((int) outputs[i].identifier.find("fmsp://") == 0) {
            struct fmp_feeder_args *fca = new fmp_feeder_args(); //(fmp_feeder_args*) malloc(sizeof (fmp_feeder_args));
            //memset((void*) fca, 0, sizeof (fmp_feeder_args));
            fca->iaudiomedia = (atic > 0) ? &inputs[audioTypeInputs[0]] : NULL;
            fca->ivideomedia = (vtic > 0) ? &inputs[videoTypeInputs[0]] : NULL;
            fca->omedia = &outputs[i];
            pthread_t * fmp_feeder_thread = (pthread_t*) malloc(sizeof (pthread_t));
            pcca.oupthreads.push_back(fmp_feeder_thread);
            if (pthread_create(fmp_feeder_thread, NULL, &fmp_feeder, fca) != 0) {
                ctsr->retvalue = -1;
                return (void*) ctsr;
            }
            //fmp_feeder(fca);
            pcca.expired_objs.push_back(fca);
        } else if ((int) outputs[i].identifier.find(".mp3") == outputs[i].identifier.length() - 4) {

        } else if ((int) outputs[i].identifier.find(".fp") == outputs[i].identifier.length() - 3) {
            MediaManager::fPRecorderArgs* rmmda = new MediaManager::fPRecorderArgs(); //(raw_mjpeg_mp3_dump_args*) malloc(sizeof (raw_mjpeg_mp3_dump_args));
            rmmda->iaudiomedia = (atic > 0) ? &inputs[audioTypeInputs[0]] : NULL;
            rmmda->ivideomedia = (vtic > 0) ? &inputs[videoTypeInputs[0]] : NULL;
            rmmda->omedia = &outputs[i];
            pthread_t * rmmda_thread = (pthread_t*) malloc(sizeof (pthread_t));
            pcca.oupthreads.push_back(rmmda_thread);
            pthread_create(rmmda_thread, NULL, &fPRecorder, rmmda);
            pcca.expired_objs.push_back(rmmda);
        } else if ((int) outputs[i].identifier[outputs[i].identifier.length() - 1] == '/') {
            raw_mjpeg_mp3_dump_args* rmmda = new raw_mjpeg_mp3_dump_args(); //(raw_mjpeg_mp3_dump_args*) malloc(sizeof (raw_mjpeg_mp3_dump_args));
            //memset((void*) rmmda, 0, sizeof (raw_mjpeg_mp3_dump_args));
            rmmda->iaudiomedia = (atic > 0) ? &inputs[audioTypeInputs[0]] : NULL;
            rmmda->ivideomedia = (vtic > 0) ? &inputs[videoTypeInputs[0]] : NULL;
            rmmda->omedia = &outputs[i];
            pthread_t * rmmda_thread = (pthread_t*) malloc(sizeof (pthread_t));
            pcca.oupthreads.push_back(rmmda_thread);
            //raw_mjpeg_mp3_dump(rmmda);
            pthread_create(rmmda_thread, NULL, &raw_mjpeg_mp3_dump, rmmda);
            pcca.expired_objs.push_back(rmmda); //0xb6b01e980xb6b01e98
        }
        i++;
    }
    ffl_debug(FPOL_MM, "waiting for output threads to join");
    i = 0;
    while (pcca.oupthreads.size() > 0) {
        pthread_t* thread = pcca.oupthreads[pcca.oupthreads.size() - 1];
        pthread_join(*thread, NULL);
        pcca.oupthreads.pop_back();
        delete thread;
    }
    ffl_debug(FPOL_MM, "terminating input threads");
    i = 0;
    timespec ts = {0, 0};
    while (pcca.inpthreads.size() > 0) {
        clock_gettime(CLOCK_REALTIME, &ts);
        pthread_cancel(*pcca.inpthreads[pcca.inpthreads.size() - 1]);
        pthread_join(*pcca.inpthreads[pcca.inpthreads.size() - 1], NULL);
        pcca.inpthreads.pop_back();
    }
    ffl_debug(FPOL_MM, "freeing up buffers");
    i = 0;
    while (i < inputs.size()) {
        if (inputs[i].type == VIDEO)delete inputs[i].buffer.framebuffer;
        else if (inputs[i].type == AUDIO)delete inputs[i].buffer.periodbuffer;
        i++;
    }
    i = 0;
    while (pcca.expired_objs.size() > 0) {
        delete pcca.expired_objs[pcca.expired_objs.size() - 1];
        pcca.expired_objs.pop_back();
    }
    pthread_cleanup_pop(0);
    ctsr->retvalue = 1;
    return (void*) ctsr;
}

void* MediaManager::fmp_feeder_aftermath(void* args, bool isSuccess) {
    
}

void MediaManager::pthreadCleanupFMPFeeder(void* args) {
    pthreadCleanupFMPFeederArgs* pcffa = (pthreadCleanupFMPFeederArgs*) args;
    if (pcffa->isSuccess) {
        time(&pcffaa->lastConnectFTS->t);
    } else {
        time_t ct;
        time(&ct);
        if (ct - ffaa->lastConnectFTS->t > ffaa->reconnectInterval) {
            ffaa->csm->lock();
            ffaa->cs->reconnect();
            ffaa->csm->unlock();
        }
    }
    delete ffaa->payload;
}

void *MediaManager::fmp_feeder(void* args) {

    /**
     * It contains length of initial bytes common in frames captured by the
     * video source.
     */
    int headerLength = 0;
    fmp_feeder_args * ffargs = (fmp_feeder_args *) args;
    pthreadCleanupFMPFeederArgs pcffa;
    pcffa.ffargs = ffargs;
    pthread_cleanup_push(pthreadCleanupFMPFeeder, (void*) &pcffa);
    int index = 0;
    struct timespec tstart = {0, 0}, tend = {0, 0};
    int ret = 0;
    int cpos;
    int ppos;
    int port;
    std::string path;
    std::string url;
    bool init = false;
connect:
    if (ffargs->omedia->signalNewState < 2 || !init) {
        if ((ffargs->ivideomedia == NULL || ffargs->ivideomedia->state < 0)&&(ffargs->iaudiomedia == NULL || ffargs->iaudiomedia->state < 0)) return NULL;
        if (ffargs->omedia->signalNewState < 1) {
            if (ffargs->omedia->signalNewState < 0) {
                ffargs->omedia->state = -1;
                return NULL;
            } else {
                ffl_debug(FPOL_MM, "waiting for start_stop set");
                sleep(ffargs->omedia->splMediaProps.fmpFeederSplProps.reconnectIntervalSec);
                goto connect;
            }
        } else {
            cpos = ffargs->omedia->identifier.find(":", 10);
            ppos = ffargs->omedia->identifier.find("/", 10);
            ffl_notice(FPOL_MM, "Stream address: %s", ffargs->omedia->identifier.c_str());
            port = cpos > 0 ? atoi(ffargs->omedia->identifier.substr(cpos + 1, ffargs->omedia->identifier.length() - (ppos > 0 ? ppos : 0) - cpos - 1).c_str()) : 927101;
            path = ppos > 0 ? ffargs->omedia->identifier.substr(ppos, ffargs->omedia->identifier.length()) : "";
            url = ffargs->omedia->identifier.substr(7, cpos > 0 ? cpos - 7 : (ppos > 0 ? ppos - 7 : ffargs->omedia->identifier.length() - 7));
            init = true;
        }
    }
    ffl_notice(FPOL_MM, "MediaManager: connecting to %s:%d", url.c_str(), port);
    try {
        ffargs->cs = new ClientSocket(url, port);
    } catch (SocketException&) {
        if (ffargs->omedia->splMediaProps.fmpFeederSplProps.reconnect) {
            ffl_err(FPOL_MM, "MediaManager: connection to %s:%d failed. sleeping for %d sec", url.c_str(), port, ffargs->omedia->splMediaProps.fmpFeederSplProps.reconnectIntervalSec);
            if ((ffargs->ivideomedia == NULL || ffargs->ivideomedia->state < 0)&&(ffargs->iaudiomedia == NULL || ffargs->iaudiomedia->state < 0)) return NULL;
            sleep(ffargs->omedia->splMediaProps.fmpFeederSplProps.reconnectIntervalSec);
            goto connect;
        }
        ffargs->ffr.error = "Unable to connect streaming server.";
        ffargs->ffr.errorcode = 1;
        return (void*) &ffargs->ffr;
    }
    time(&ffargs->lastconnectFTS.t);
    std::string beacon = ("{path:\"" + path + "\",MAXPACKSIZE:20000}");
    try {
        *ffargs->cs << beacon;
    } catch (SocketException e) {
        ffl_err(FPOL_MM, "MediaManager: unable to send initpack to %s:%d", url.c_str(), port);
        if (ffargs->omedia->splMediaProps.fmpFeederSplProps.reconnect) {
            delete ffargs->cs;
            sleep(ffargs->omedia->splMediaProps.fmpFeederSplProps.reconnectIntervalSec);
            goto connect;
        } else {
            pthread_exit(NULL);
        }
    }
    ffargs->cs->set_non_blocking(true);
    int videoframesPfloat;
    int videoframesCfloat = ffargs->ivideomedia != NULL ? ffargs->ivideomedia->bufferfloat : 0;
    int audioperiodPfloat;
    int audioperiodCfloat = ffargs->iaudiomedia != NULL ? ffargs->iaudiomedia->bufferfloat : 0;
    int received_frames_per_segment_duration;
    std::string * packet;
    std::string buf;
    int transmitted_frames_per_segment_duration = ffargs->omedia->videoframerate * ffargs->omedia->segmentDuration;
    if (transmitted_frames_per_segment_duration < 1) {
        std::cout << "\n" << getTime() << " MediaManager::raw_mjpeg_mp3_dump: error: your frame rate should give atleast one frame in segment duration.\n";
        pthread_exit(NULL);
    }
    int pf;
    int cf;
    libav_encode_args lavea;
    if (ffargs->iaudiomedia != NULL) {
        lavea.codecID = AV_CODEC_ID_MP3;
        lavea.av_smpl_fmt = AV_SAMPLE_FMT_S16P;
        lavea.bitrate = ffargs->omedia->audioBitrate;
        lavea.samplingFrequency = ffargs->iaudiomedia->audioSamplingFrequency;
        lavea.input_buffer.periodbuffer = ffargs->iaudiomedia->buffer.periodbuffer;
        lavea.output_buffer = NULL;
    }
    sleep(ffargs->omedia->segmentDuration);
    bool initPck = true;

    /**
     * It holds the last byte of the video head to check its same for all frames. 
     */
    char headerLastByte;
    while ((ffargs->iaudiomedia != NULL && ffargs->iaudiomedia->state > 0) || (ffargs->ivideomedia != NULL && ffargs->ivideomedia->state > 0)) {
        clock_gettime(CLOCK_REALTIME, &tstart);
        tstart.tv_sec += ffargs->omedia->segmentDuration;
        if (ffargs->ivideomedia != NULL && ffargs->ivideomedia->state != -1) {
            videoframesPfloat = videoframesCfloat + 1;
            videoframesCfloat = ffargs->ivideomedia->bufferfloat;
        }
        if (ffargs->iaudiomedia != NULL && ffargs->iaudiomedia->state != -1) {
            audioperiodPfloat = audioperiodCfloat;
            audioperiodCfloat = ffargs->iaudiomedia->bufferfloat;
        }
        index++;
        packet = new std::string("{index:" + itoa(index));
        *packet += ",ferryframes:[";
        std::vector<int> framesizes;
        if (ffargs->ivideomedia != NULL && ffargs->ivideomedia->state > 0) {
            received_frames_per_segment_duration = videoframesCfloat > videoframesPfloat ? (videoframesCfloat - videoframesPfloat + 1) : (videoframesCfloat + 200 - videoframesPfloat + 1);
            if (transmitted_frames_per_segment_duration == 0 || transmitted_frames_per_segment_duration > received_frames_per_segment_duration)transmitted_frames_per_segment_duration = received_frames_per_segment_duration;
            cf = videoframesPfloat;
            for (int i = 0; i < transmitted_frames_per_segment_duration; i++) {
                pf = cf;
                cf = (videoframesPfloat + (i * received_frames_per_segment_duration / transmitted_frames_per_segment_duration)) % ffargs->ivideomedia->buffer.framebuffer->size();
                *packet += "FFESCSTR";
                buf.assign((char*) (((char*) (*ffargs->ivideomedia->buffer.framebuffer)[cf].frame)), (*ffargs->ivideomedia->buffer.framebuffer)[cf].length);
                *packet += buf;
                framesizes.push_back((*ffargs->ivideomedia->buffer.framebuffer)[cf].length);
                *packet += buf = "FFESCSTR";
                if (i < (transmitted_frames_per_segment_duration - 1)) {
                    *packet += buf = ",";
                }
            }
        }
        *packet += buf = "],framesizes:[";
        for (int i = 0; i < framesizes.size(); i++) {
            packet->append(itoa(framesizes[i]));
            if (i < framesizes.size() - 1) {
                packet->append(",");
            }
        }
        *packet += "],ferrymp3:FFESCSTR";
        if (ffargs->iaudiomedia != NULL && ffargs->iaudiomedia->state > 0) {
            lavea.initptr = audioperiodPfloat;
            lavea.termptr = audioperiodCfloat - 1;
            ffl_debug(FPOL_MM, "collected sound samples at %d to %d", audioperiodPfloat, audioperiodCfloat);
            audio_encode(&lavea);
            if (lavea.output_buffer != NULL) {
                packet->append(lavea.output_buffer, lavea.output_buffer_size);
                ffl_debug(FPOL_MM, "mp3 encoded packet size: %d", lavea.output_buffer_size);
            }
            free(lavea.output_buffer);
        }
        *packet += buf = "FFESCSTR,time:\"" + itoa((int) (tstart.tv_sec * 1000 + tstart.tv_nsec / 1000)) + "\",duration:" + itoa(ffargs->omedia->segmentDuration) + ",endex:" + itoa(index) + "}";
        ffargs->csm.lock();
        try {
            ffargs->cs->send(packet, MSG_DONTWAIT | MSG_NOSIGNAL);
        } catch (SocketException e) {
            ffargs->csm.unlock();
            delete ffargs->cs;
            ffl_notice(FPOL_MM, "MediaManager: unable to send packet to %s:%d", url.c_str(), port);
            goto connect;
        }
        ffargs->csm.unlock();
        if (ffargs->omedia->signalNewState < 2) {
            ffargs->cs->disconnect();
            goto connect;
        }
nsleep:
        ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &tstart, &tend);
        if (ret) {
            ffl_debug(FPOL_MM, "MediaManager: clock_nanosleep interrupted! remaining %d.%dsec. gonna sleep again zzzz");
            goto nsleep;
        } else {
            //ffl_debug(FPOL_MM, "MediaManager: clock_nanosleep successfully slept till %ul.%ul", tstart.tv_sec, tstart.tv_nsec);
        }

    }
    pthread_cleanup_pop(1);
}

void* MediaManager::fPRecorderAftermath(void* args, bool isSuccess) {
    MediaManager::fPRecorderAftermathArgs * ffaa = (MediaManager::fPRecorderAftermathArgs*)args;
    if (isSuccess) {

    } else {

    }
    delete ffaa->payload;
}

void deallocateFMPFeederArgs(void* args) {
    //free(args);
}

void *MediaManager::fPRecorder(void* args) {
    /**
     * It contains length of initial bytes common in frames captured by the video source.
     */
    int headerLength = 0;
    ffl_debug(FPOL_MM, "fPRecorder started");
    fPRecorderArgs * ffargs = (fPRecorderArgs *) args;
    pthread_cleanup_push(deallocateFMPFeederArgs, args);
    int index = 0;
    struct timespec tstart = {0, 0}, tend = {0, 0};
    time_t max_fileduration_tracker_init;
    time_t max_fileduration_tracker_term;
    int ret = 0;
    std::string bare_fn;
    std::string ext;
    time(&max_fileduration_tracker_init);
    time(&max_fileduration_tracker_term);
    bool init = false;
start_stop:
    if (ffargs->omedia->signalNewState < 2 || !init) {
        if ((ffargs->ivideomedia == NULL || ffargs->ivideomedia->state < 0)&&(ffargs->iaudiomedia == NULL || ffargs->iaudiomedia->state < 0)) return NULL;
        if (ffargs->omedia->signalNewState < 1) {
            if (ffargs->omedia->signalNewState < 0) {
                ffargs->omedia->state = -1;
                return NULL;
            } else {
                ffl_debug(FPOL_MM, "waiting for start_stop set");
                sleep(1);
                goto start_stop;
            }
        } else {
            bare_fn = ffargs->omedia->identifier.substr(0, ffargs->omedia->identifier.find(".fp")) + std::string("+");
            ext = "";
            ffargs->omedia->signalNewState = 2;
            init = true;
        }
    };
    ext = std::string(itoa(max_fileduration_tracker_term - max_fileduration_tracker_init));
    time(&max_fileduration_tracker_init);
    std::string fn = bare_fn + ext + ".fp";
    ffargs->fd = open(fn.c_str(), O_WRONLY | O_CREAT);
    if (ffargs->fd < 1) {
        ffargs->omedia->state = -1;
        ffl_err(FPOL_MM, "unable to create file %s", fn.c_str());
        return NULL;
    }
    ffl_debug(FPOL_MM, "file %s is created", fn.c_str());
    int videoframesPfloat;
    int videoframesCfloat = ffargs->ivideomedia != NULL ? ffargs->ivideomedia->bufferfloat : 0;
    int audioperiodPfloat;
    int audioperiodCfloat = ffargs->iaudiomedia != NULL ? ffargs->iaudiomedia->bufferfloat : 0;
    int received_frames_per_segment_duration;
    std::string * packet;
    std::string buf;
    int transmitted_frames_per_segment_duration = ffargs->omedia->videoframerate * ffargs->omedia->segmentDuration;
    if (transmitted_frames_per_segment_duration < 1) {
        ffl_err(FPOL_MM, "MediaManager::fPRecorder: your frame rate should give atleast one frame in segment duration");
        return NULL;
    }
    int pf;
    int cf;
    libav_encode_args lavea;
    if (ffargs->iaudiomedia != NULL) {
        lavea.codecID = AV_CODEC_ID_MP3;
        lavea.av_smpl_fmt = AV_SAMPLE_FMT_S16P;
        lavea.bitrate = ffargs->omedia->audioBitrate;
        lavea.samplingFrequency = ffargs->iaudiomedia->audioSamplingFrequency;
        lavea.input_buffer.periodbuffer = ffargs->iaudiomedia->buffer.periodbuffer;
        lavea.output_buffer = NULL;
    }
    sleep(ffargs->omedia->segmentDuration);

    /**
     * It holds the last byte of the video head to check its same for all frames. 
     */
    char headerLastByte;
    while ((ffargs->iaudiomedia != NULL && ffargs->iaudiomedia->state > 0) || (ffargs->ivideomedia != NULL && ffargs->ivideomedia->state > 0)) {
        clock_gettime(CLOCK_REALTIME, &tstart);
        time(&max_fileduration_tracker_term);
        if (max_fileduration_tracker_term - max_fileduration_tracker_init >= ffargs->omedia->splMediaProps.fPRecorderExclProps.perFileDurationSec) {
            close(ffargs->fd);
            ffl_debug(FPOL_MM, "max file duration hit!");
            goto start_stop;
        }
        tstart.tv_sec += ffargs->omedia->segmentDuration;
        if (ffargs->ivideomedia != NULL && ffargs->ivideomedia->state > 0) {
            videoframesPfloat = videoframesCfloat + 1;
            videoframesCfloat = ffargs->ivideomedia->bufferfloat;
        }
        if (ffargs->iaudiomedia != NULL && ffargs->iaudiomedia->state > 0) {
            audioperiodPfloat = audioperiodCfloat;
            audioperiodCfloat = ffargs->iaudiomedia->bufferfloat;
        }
        index++;
        packet = new std::string("{index:" + std::to_string(index));
        *packet += ",ferryframes:[";
        if (ffargs->ivideomedia != NULL && ffargs->ivideomedia->state > 0) {
            received_frames_per_segment_duration = videoframesCfloat > videoframesPfloat ? (videoframesCfloat - videoframesPfloat + 1) : (videoframesCfloat + 200 - videoframesPfloat + 1);
            if (transmitted_frames_per_segment_duration == 0 || transmitted_frames_per_segment_duration > received_frames_per_segment_duration)transmitted_frames_per_segment_duration = received_frames_per_segment_duration;
            cf = videoframesPfloat;
            for (int i = 0; i < transmitted_frames_per_segment_duration; i++) {
                pf = cf;
                cf = (videoframesPfloat + (i * received_frames_per_segment_duration / transmitted_frames_per_segment_duration)) % ffargs->ivideomedia->buffer.framebuffer->size();
                *packet += "FFESCSTR";
                buf.assign((char*) (((char*) (*ffargs->ivideomedia->buffer.framebuffer)[cf].frame)), (*ffargs->ivideomedia->buffer.framebuffer)[cf].length);
                *packet += buf;
                *packet += buf = "FFESCSTR";
                if (i < (transmitted_frames_per_segment_duration - 1)) {
                    *packet += buf = ",";
                }
            }
        }
        *packet += "],ferrymp3:FFESCSTR";
        if (ffargs->iaudiomedia != NULL && ffargs->iaudiomedia->state > 0) {
            lavea.initptr = audioperiodPfloat;
            lavea.termptr = audioperiodCfloat - 1;
            ffl_debug(FPOL_MM, "collected sound samples at %d to %d", audioperiodPfloat, audioperiodCfloat);
            audio_encode(&lavea);
            if (lavea.output_buffer != NULL) {
                packet->append(lavea.output_buffer, lavea.output_buffer_size);
                ffl_debug(FPOL_MM, "mp3 encoded packet size: %d", lavea.output_buffer_size);
            }
            free(lavea.output_buffer);
        }
        *packet += "FFESCSTR,time:\"" + itoa((int) (tstart.tv_sec * 1000 + tstart.tv_nsec / 1000)) + "\",duration:" + itoa(ffargs->omedia->segmentDuration) + "},";
        write(ffargs->fd, (void*) packet->c_str(), packet->size());
        if (ffargs->omedia->signalNewState < 2) {
            close(ffargs->fd);
            time(&max_fileduration_tracker_term);
            ffl_debug(FPOL_MM, "external signalNewState set to stop");
            goto start_stop;
        }
nsleep:
        ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &tstart, &tend);
        if (ret) {
            ffl_debug(FPOL_MM, "MediaManager: clock_nanosleep interrupted! remaining %d.%dsec");
            //perror("clock_nanosleep");
            //std::cout << "; remaining " << tend.tv_sec << "sec " << tend.tv_nsec << "nsec\n";
            goto nsleep;
        } else {
            //ffl_debug(FPOL_MM, "MediaManager: clock_nanosleep successfully slept till %ul.%ul", tstart.tv_sec, tstart.tv_nsec);
        }
    }
    pthread_cleanup_pop(1);
    return NULL;
}

void* MediaManager::raw_mjpeg_mp3_dump(void* args) { //#rmmd
    raw_mjpeg_mp3_dump_args * rmmdargs = (raw_mjpeg_mp3_dump_args *) args;
    pthread_cleanup_push(deallocate_fmp_feeder_args, args);
    int index = 0;
    struct timespec tstart = {0, 0}, tend = {0, 0};
    struct stat ptr;
    int ret = 0;
    if (stat(rmmdargs->omedia->identifier.c_str(), &ptr) == -1) {
        rmmdargs->rmmdr.error = "Destination folder not found";
        rmmdargs->rmmdr.errorcode = -1;
        return &rmmdargs->rmmdr;
    }
    int videoframesPfloat;
    int videoframesCfloat = rmmdargs->ivideomedia->bufferfloat;
    int audioperiodPfloat;
    int audioperiodCfloat = rmmdargs->iaudiomedia->bufferfloat;
    int received_frames_per_segment_duration;
    int transmitted_frames_per_segment_duration = rmmdargs->omedia->videoframerate * rmmdargs->omedia->segmentDuration;
    if (transmitted_frames_per_segment_duration < 1) {
        std::cout << "\n" << getTime() << " MediaManager::raw_mjpeg_mp3_dump: error: your frame rate should give atleast one frame in segment duration.\n";
        pthread_exit(NULL);
    }
    int pf;
    int cf = videoframesPfloat;
    int frame_fd;
    int frame_seq_no = 0;
    std::string frame_fn;
    std::string audio_fn = rmmdargs->omedia->identifier + "audio.mp3";
    std::string rawaudio_fn = rmmdargs->omedia->identifier + "audio.swav";
    int audio_fd = open(audio_fn.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    //    int rawaudio_fd = open(rawaudio_fn.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    sleep(rmmdargs->omedia->segmentDuration);
    libav_encode_args lavea;
    lavea.codecID = AV_CODEC_ID_MP3;
    lavea.av_smpl_fmt = AV_SAMPLE_FMT_S16P;
    lavea.bitrate = rmmdargs->omedia->audioBitrate;
    lavea.samplingFrequency = rmmdargs->iaudiomedia->audioSamplingFrequency;
    lavea.input_buffer.periodbuffer = rmmdargs->iaudiomedia->buffer.periodbuffer;
    lavea.output_buffer = NULL;
    while (rmmdargs->iaudiomedia->state>-1 || rmmdargs->ivideomedia->state>-1) {
        clock_gettime(CLOCK_REALTIME, &tstart);
        tstart.tv_sec += rmmdargs->omedia->segmentDuration;
        if (rmmdargs->ivideomedia->state > 0) {
            videoframesPfloat = videoframesCfloat;
            videoframesCfloat = rmmdargs->ivideomedia->bufferfloat;
        }
        if (rmmdargs->iaudiomedia->state > 0) {
            audioperiodPfloat = audioperiodCfloat;
            audioperiodCfloat = rmmdargs->iaudiomedia->bufferfloat;
        }
        if (rmmdargs->ivideomedia->state > 0) {
            received_frames_per_segment_duration = videoframesCfloat > videoframesPfloat ? (videoframesCfloat - videoframesPfloat) : (videoframesCfloat + 200 - videoframesPfloat);
            if (transmitted_frames_per_segment_duration == 0)transmitted_frames_per_segment_duration = received_frames_per_segment_duration;
            int frameSkipLength = received_frames_per_segment_duration / transmitted_frames_per_segment_duration;
            cf = videoframesPfloat;
            for (int i = 0; i < transmitted_frames_per_segment_duration; i++) {
                pf = cf;
                cf = (videoframesPfloat + (i * frameSkipLength)) % rmmdargs->ivideomedia->buffer.framebuffer->size();
                ++frame_seq_no;
                frame_fn = rmmdargs->omedia->identifier + std::string(itoa(frame_seq_no)) + ".jpeg";
                frame_fd = open(frame_fn.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                write(frame_fd, (*rmmdargs->ivideomedia->buffer.framebuffer)[cf].frame, (*rmmdargs->ivideomedia->buffer.framebuffer)[cf].length);
                close(frame_fd);
            }
        }
        if (rmmdargs->iaudiomedia->state > 0) {
            lavea.initptr = audioperiodPfloat;
            lavea.termptr = audioperiodCfloat - 1;
            ffl_debug(FPOL_MM, "collected sound samples at %d to %d", lavea.initptr, lavea.termptr);
            audio_encode(&lavea);
            if (lavea.output_buffer != NULL) {
                write(audio_fd, lavea.output_buffer, lavea.output_buffer_size);
                ffl_debug(FPOL_MM, "mp3 encoded packet size: %d", lavea.output_buffer_size);
            }
            free(lavea.output_buffer);
        }
nsleep:
        ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &tstart, &tend);
        if (ret) {
            ffl_warn(FPOL_MM | NO_NEW_LINE, "");
            perror("clock_nanosleep");
            std::cout << "; remaining " << tend.tv_sec << "sec " << tend.tv_nsec << "nsec\n";
            goto nsleep;
        } else {
            //ffl_debug(FPOL_MM, "Clock_nanosleep successfully slept for %ul.%ulsec", tstart.tv_sec, tstart.tv_nsec);

        }
    }
    close(audio_fd);
    pthread_cleanup_pop(1);
}

//MediaManager::fmp_feeder_args::fmp_feeder_args() {
//};
//
//MediaManager::fmp_feeder_args::fmp_feeder_args(media& iaudiomedia, media& ivideomedia, media& omedia) {
//    this->iaudiomedia = iaudiomedia;
//    this->ivideomedia = ivideomedia;
//    this->omedia = omedia;
//}

MediaManager::media::media() {

}

MediaManager::media::~media() {

}

MediaManager::media& MediaManager::media::operator=(MediaManager::media& m) {
    return m;
}


