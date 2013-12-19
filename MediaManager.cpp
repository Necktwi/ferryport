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
#include <string>
#include <sys/socket.h>
#include <stdlib.h>
#include <iostream>
#include <valarray>
#include <fcntl.h>
#include <sys/stat.h>
#include <libavcodec/avcodec.h>
#include <unistd.h>

MediaManager::MediaManager() {

}

MediaManager::MediaManager(const MediaManager& orig) {

}

MediaManager::~MediaManager() {
}

std::string MediaManager::MediaTypeString[] = {"AUDIO", "VIDEO"};
std::string MediaManager::EncodingString[] = {"mjpeg", "mp3"};

void* MediaManager::capture(void * args) {
    capture_thread_iargs * ctsa = (capture_thread_iargs*) args;
    capture_thread_rargs * ctsr = new capture_thread_rargs;
    ctsr->retvalue = capture(*ctsa->inputs, *ctsa->outputs);
    return ctsr;
}

int MediaManager::capture(std::valarray<MediaManager::media> inputs, std::valarray<MediaManager::media> outputs) {
    int i = 0;
    int audioTypeInputs[10];
    int videoTypeInputs[10];
    int atic = 0;
    int vtic = 0;
    int threadCount = 0;
    std::vector<pthread_t*> inpthreads;
    std::vector<pthread_t*> oupthreads;
    std::vector<void*> expired_objs;
    while (i < inputs.size()) {
        if (inputs[i].type == MediaManager::VIDEO) {
            videoTypeInputs[vtic++] = i;
            pthread_t *capture_thread = (pthread_t*) malloc(sizeof (pthread_t));
            inpthreads.push_back(capture_thread);
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
            //videocapture(cargs);
            inputs[i].videoframerate = cargs->framerate;
            expired_objs.push_back((void*) cargs);
            if (pthread_create(capture_thread, NULL, &videocapture, cargs) != 0)return -1;
        } else if (inputs[i].type == MediaManager::AUDIO) {
            audioTypeInputs[atic++] = i;
            pthread_t *snd_record_thread = (pthread_t*) malloc(sizeof (pthread_t));
            inpthreads.push_back(snd_record_thread);
            inputs[i].buffer.periodbuffer = new std::valarray<ferryperiod>(200);
            snd_record_args* srargs = new snd_record_args();
            //memset((void*) &srargs, 0, sizeof (snd_record_args));
            srargs->samplingFrequency = inputs[i].audioSamplingFrequency;
            srargs->duration = inputs[i].duration;
            srargs->dev_name = inputs[i].identifier;
            srargs->periodbuffer = inputs[i].buffer.periodbuffer; //(ferryperiod**) calloc(srargs.periodbufferlength, sizeof (ferryperiod*));
            srargs->periodbufferfloat = &inputs[i].bufferfloat;
            srargs->periodbufferlength = &inputs[i].buffersize;
            srargs->returnObj.state = &inputs[i].state;
            inputs[i].bufferfloat = 0;
            inputs[i].buffersize = 200;
            expired_objs.push_back((void*) srargs);
            if (pthread_create(snd_record_thread, NULL, &snd_record, srargs) != 0)return -1;
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
            fca->iaudiomedia = &inputs[audioTypeInputs[0]];
            fca->ivideomedia = &inputs[videoTypeInputs[0]];
            fca->omedia = &outputs[i];
            pthread_t * fmp_feeder_thread = (pthread_t*) malloc(sizeof (pthread_t));
            oupthreads.push_back(fmp_feeder_thread);
            //if (pthread_create(fmp_feeder_thread, NULL, &fmp_feeder, fca) != 0)return -1;
            fmp_feeder(fca);
            expired_objs.push_back(fca);
        } else if ((int) outputs[i].identifier.find(".mp3") == outputs[i].identifier.length() - 5) {

        } else if ((int) outputs[i].identifier[outputs[i].identifier.length() - 1] == '/') {
            raw_mjpeg_mp3_dump_args* rmmda = new raw_mjpeg_mp3_dump_args(); //(raw_mjpeg_mp3_dump_args*) malloc(sizeof (raw_mjpeg_mp3_dump_args));
            //memset((void*) rmmda, 0, sizeof (raw_mjpeg_mp3_dump_args));
            rmmda->iaudiomedia = &inputs[audioTypeInputs[0]];
            rmmda->ivideomedia = &inputs[videoTypeInputs[0]];
            rmmda->omedia = &outputs[i];
            pthread_t * rmmda_thread = (pthread_t*) malloc(sizeof (pthread_t));
            oupthreads.push_back(rmmda_thread);
            //raw_mjpeg_mp3_dump(rmmda);
            pthread_create(rmmda_thread, NULL, &raw_mjpeg_mp3_dump, rmmda);
            expired_objs.push_back(rmmda); //0xb6b01e980xb6b01e98
        }
        i++;
    }
    if ((debug & 1) == 1) {
        std::cout << "\n" << getTime() << " MediaManager::capture: waiting for output threads to join.\n";
        fflush(stdout);
    }
    i = 0;
    while (oupthreads.size() > 0) {
        pthread_t* thread = oupthreads[oupthreads.size() - 1];
        pthread_join(*thread, NULL);
        oupthreads.pop_back();
        delete thread;
    }
    if ((debug & 1) == 1) {
        std::cout << "\n" << getTime() << " MediaManager::capture: terminating input threads.\n";
        fflush(stdout);
    }
    i = 0;
    while (inpthreads.size() > 0) {
        pthread_cancel(*inpthreads[inpthreads.size() - 1]);
        pthread_join(*inpthreads[inpthreads.size() - 1], NULL);
        inpthreads.pop_back();
    }
    if ((debug & 1) == 1) {
        std::cout << "\n" << getTime() << " MediaManager::capture: freeing up buffers.\n";
        fflush(stdout);
    }
    i = 0;
    while (i < inputs.size()) {
        if (inputs[i].type == VIDEO)delete inputs[i].buffer.framebuffer;
        else if (inputs[i].type == AUDIO)delete inputs[i].buffer.periodbuffer;
        i++;
    }
    i = 0;
    while (expired_objs.size() > 0) {
        delete expired_objs[expired_objs.size() - 1];
        expired_objs.pop_back();
    }
    return 1;
}

void* MediaManager::fmp_feeder_aftermath(void* args, bool isSuccess) {
    MediaManager::fmp_feeder_aftermath_args * ffaa = (MediaManager::fmp_feeder_aftermath_args*)args;
    if (isSuccess) {
        time(&ffaa->lastConnectFTS->t);
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

void deallocate_fmp_feeder_args(void* args) {
    //free(args);
}

void *MediaManager::fmp_feeder(void* args) {
    fmp_feeder_args * ffargs = (fmp_feeder_args *) args;
    pthread_cleanup_push(deallocate_fmp_feeder_args, args);
    int index = 0;
    struct timespec tstart = {0, 0}, tend = {0, 0};
    int cpos = ffargs->omedia->identifier.find(":", 10);
    int ppos = ffargs->omedia->identifier.find("/", 10);
    int port = cpos > 0 ? atoi(ffargs->omedia->identifier.substr(cpos + 1, ffargs->omedia->identifier.length() - (ppos > 0 ? ppos : 0) - cpos - 1).c_str()) : 927101;
    std::string path = ppos > 0 ? ffargs->omedia->identifier.substr(ppos, ffargs->omedia->identifier.length()) : "";
    std::string url = ffargs->omedia->identifier.substr(7, cpos > 0 ? cpos - 7 : (ppos > 0 ? ppos - 7 : ffargs->omedia->identifier.length() - 7));
connect:
    try {
        ffargs->cs = new ClientSocket(url, port);
    } catch (SocketException&) {
        if (ffargs->omedia->splMediaProps.fmpFeederSplProps.reconnect) {
            sleep(ffargs->omedia->splMediaProps.fmpFeederSplProps.reconnectIntervalSec);
            delete ffargs->cs;
            goto connect;
        }
        ffargs->ffr.error = "Unable to connect streaming server.";
        ffargs->ffr.errorcode = 1;
        return (void*) &ffargs->ffr;
    }
    time(&ffargs->lastconnectFTS.t);
    std::string beacon = ("{path:\"" + path + "\",MAXPACKSIZE:8192}");
    try {
        *ffargs->cs << beacon;
    } catch (SocketException e) {
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
    int videoframesCfloat = ffargs->ivideomedia->bufferfloat;
    int audioperiodPfloat;
    int audioperiodCfloat = ffargs->iaudiomedia->bufferfloat;
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
    lavea.codecID = AV_CODEC_ID_MP3;
    lavea.av_smpl_fmt = AV_SAMPLE_FMT_S16P;
    lavea.bitrate = ffargs->omedia->audioBitrate;
    lavea.samplingFrequency = ffargs->iaudiomedia->audioSamplingFrequency;
    lavea.input_buffer.periodbuffer = ffargs->iaudiomedia->buffer.periodbuffer;
    lavea.output_buffer = NULL;
    sleep(ffargs->omedia->segmentDuration);
    while (ffargs->iaudiomedia->state>-1 || ffargs->ivideomedia->state>-1) {
        clock_gettime(CLOCK_MONOTONIC, &tstart);
        if (ffargs->ivideomedia->state != -1) {
            videoframesPfloat = videoframesCfloat;
            videoframesCfloat = ffargs->ivideomedia->bufferfloat;
        }
        if (ffargs->iaudiomedia->state != -1) {
            audioperiodPfloat = audioperiodCfloat;
            audioperiodCfloat = ffargs->iaudiomedia->bufferfloat;
        }
        index++;
        packet = new std::string("{index:" + std::string((char*) itoa(index)) + ",ferryframes:[");
        //cs->send(*packet, MSG_MORE);
        std::vector<int> framesizes;
        if (ffargs->ivideomedia->state != -1) {
            received_frames_per_segment_duration = videoframesCfloat > videoframesPfloat ? (videoframesCfloat - videoframesPfloat + 1) : (videoframesCfloat + 200 - videoframesPfloat);
            if (transmitted_frames_per_segment_duration == 0)transmitted_frames_per_segment_duration = received_frames_per_segment_duration;
            cf = videoframesPfloat;
//            std::string frame_fn;
//            int frame_fd;
            for (int i = 0; i < transmitted_frames_per_segment_duration; i++) {
                pf = cf;
                cf = (videoframesPfloat + (i * received_frames_per_segment_duration / transmitted_frames_per_segment_duration)) % ffargs->ivideomedia->buffer.framebuffer->size();
                *packet += "FFESCSTR";
                //cs->send(buf, MSG_MORE);
                buf.assign((char*) (*ffargs->ivideomedia->buffer.framebuffer)[cf].frame, (*ffargs->ivideomedia->buffer.framebuffer)[cf].length);
                *packet += buf;
                framesizes.push_back((*ffargs->ivideomedia->buffer.framebuffer)[cf].length);
//                frame_fn = ffargs->omedia->identifier + std::string(itoa(i)) + ".jpeg";
//                frame_fd = open(frame_fn.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
//                write(frame_fd, (*ffargs->ivideomedia->buffer.framebuffer)[cf].frame, (*ffargs->ivideomedia->buffer.framebuffer)[cf].length);
//                close(frame_fd);
//                //cs->send(buf, MSG_MORE);
                *packet += buf = "FFESCSTR";
                //cs->send(buf, MSG_MORE);
                if (i < (transmitted_frames_per_segment_duration - 1)) {
                    *packet += buf = ",";
                    //cs->send(buf, MSG_MORE);
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
        //cs->send(buf, MSG_MORE);
        if (ffargs->iaudiomedia->state != -1) {
            lavea.initptr = audioperiodPfloat;
            lavea.termptr = audioperiodCfloat;
            audio_encode(lavea);
            *packet += buf = std::string(lavea.output_buffer);
            //cs->send(buf, MSG_MORE);
            free(lavea.output_buffer);
        }
        *packet += buf = "FFESCSTR,time:\"" + std::string((char*) itoa((int) tstart.tv_sec)) + "\",duration:" + std::string((char*) itoa(ffargs->omedia->segmentDuration)) + ",endex:" + std::string((char*) itoa(index)) + "}";
        //cs->send(buf); //, MSG_MORE);
//        ClientSocket::AftermathObj* afmo = new ClientSocket::AftermathObj();
//        afmo->aftermath = &MediaManager::fmp_feeder_aftermath;
//        MediaManager::fmp_feeder_aftermath_args * ffaa = new MediaManager::fmp_feeder_aftermath_args();
//        ffaa->cs = ffargs->cs;
//        ffaa->csm = &ffargs->csm;
//        afmo->aftermathDS = ffaa;
        ffargs->csm.lock();
        //ffargs->cs->asyncsend(packet, afmo);
        try {
            ffargs->cs->send(packet, MSG_DONTWAIT | MSG_NOSIGNAL);
        } catch (SocketException e) {
            ffargs->csm.unlock();
            goto connect;
        }
        ffargs->csm.unlock();
        clock_gettime(CLOCK_MONOTONIC, &tend);
        if ((tend.tv_sec - tstart.tv_sec) < ffargs->omedia->segmentDuration) {
            tend.tv_nsec /= 1000;
            tstart.tv_nsec /= 1000;
            tend.tv_nsec += (tend.tv_sec * (1000000));
            tstart.tv_nsec += (tstart.tv_sec * (1000000));
            usleep((ffargs->omedia->segmentDuration * 1000000)-(tend.tv_nsec - tstart.tv_nsec) - 100);
        }
    }
    pthread_cleanup_pop(1);
}

void* MediaManager::raw_mjpeg_mp3_dump(void* args) { //#rmmd
    raw_mjpeg_mp3_dump_args * rmmdargs = (raw_mjpeg_mp3_dump_args *) args;
    pthread_cleanup_push(deallocate_fmp_feeder_args, args);
    int index = 0;
    struct timespec tstart = {0, 0}, tend = {0, 0};
    struct stat ptr;
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
    std::string audio_fn = rmmdargs->omedia->identifier + "audio.mp2";
    int audio_fd = open(audio_fn.c_str(), O_WRONLY | O_CREAT);
    sleep(rmmdargs->omedia->segmentDuration);
    libav_encode_args lavea;
    lavea.codecID = AV_CODEC_ID_MP3;
    lavea.av_smpl_fmt = AV_SAMPLE_FMT_S16P;
    lavea.bitrate = rmmdargs->omedia->audioBitrate;
    lavea.samplingFrequency = rmmdargs->iaudiomedia->audioSamplingFrequency;
    lavea.input_buffer.periodbuffer = rmmdargs->iaudiomedia->buffer.periodbuffer;
    lavea.output_buffer = NULL;
    while (rmmdargs->iaudiomedia->state>-1 || rmmdargs->ivideomedia->state>-1) {
        clock_gettime(CLOCK_MONOTONIC, &tstart);
        if (rmmdargs->ivideomedia->state != -1) {
            videoframesPfloat = videoframesCfloat;
            videoframesCfloat = rmmdargs->ivideomedia->bufferfloat;
        }
        if (rmmdargs->iaudiomedia->state != -1) {
            audioperiodPfloat = audioperiodCfloat;
            audioperiodCfloat = rmmdargs->iaudiomedia->bufferfloat;
        }
        if (rmmdargs->ivideomedia->state != -1) {
            received_frames_per_segment_duration = videoframesCfloat > videoframesPfloat ? (videoframesCfloat - videoframesPfloat + 1) : (videoframesCfloat + 200 - videoframesPfloat);
            if (transmitted_frames_per_segment_duration == 0)transmitted_frames_per_segment_duration = received_frames_per_segment_duration;
            cf = videoframesPfloat;
            for (int i = 0; i < transmitted_frames_per_segment_duration; i++) {
                pf = cf;
                cf = (videoframesPfloat + (i * received_frames_per_segment_duration / transmitted_frames_per_segment_duration)) % rmmdargs->ivideomedia->buffer.framebuffer->size();
                ++frame_seq_no;
                frame_fn = rmmdargs->omedia->identifier + std::string(itoa(frame_seq_no)) + ".jpeg";
                frame_fd = open(frame_fn.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
                write(frame_fd, (*rmmdargs->ivideomedia->buffer.framebuffer)[cf].frame, (*rmmdargs->ivideomedia->buffer.framebuffer)[cf].length);
                close(frame_fd);
            }
        }
        if (rmmdargs->iaudiomedia->state != -1) {
            lavea.initptr = audioperiodPfloat;
            lavea.termptr = audioperiodCfloat;
            audio_encode(lavea);
            write(audio_fd, lavea.output_buffer, lavea.output_buffer_size);
            free(lavea.output_buffer);
        }
        clock_gettime(CLOCK_MONOTONIC, &tend);
        if ((tend.tv_sec - tstart.tv_sec) < rmmdargs->omedia->segmentDuration) {
            tend.tv_nsec /= 1000;
            tstart.tv_nsec /= 1000;
            tend.tv_nsec += (tend.tv_sec * (1000000));
            tstart.tv_nsec += (tstart.tv_sec * (1000000));
            usleep((rmmdargs->omedia->segmentDuration * 1000000)-(tend.tv_nsec - tstart.tv_nsec) - 100);
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


