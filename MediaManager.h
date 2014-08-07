/* 
 * File:   MediaManager.h
 * Author: gowtham
 *
 * Created on 23 October, 2013, 11:35 AM
 */

#ifndef MEDIAMANAGER_H
#define	MEDIAMANAGER_H

#include "mypcm.h"
#include "capture.h"
#include "ClientSocket.h"
#include "mystdlib.h"
#include <string>
#include <valarray>
#include <mutex>

class MediaManager {
public:
    MediaManager();
    MediaManager(const MediaManager& orig);
    virtual ~MediaManager();

    enum MediaType {
        AUDIO, VIDEO, FILE
    };
    static std::string MediaTypeString[];

    enum Encoding {
        MJPEG, MP3, MP2
    };
    static std::string EncodingString[];

    enum PixelFormat {
        YUYV422, YUY422P
    };

    struct FmpFeederSplProps {
        bool reconnect;
        int reconnectIntervalSec;
    };

    /**
     * exclusive properties for FairPlay recorder
     */
    struct FPRecorderExclProps {
        int perFileDurationSec;
    };

    class media {
    public:
        std::string identifier;
        MediaType type;
        int width = 320;
        int height = 240;
        /**
         * give in fps i.e. number of frames per second
         */
        float videoframerate = 5;
        int audioBitrate = 64000;
        int audioSamplingFrequency = 44100;
        int duration = 0;
        int segmentDuration = 2;
        Encoding encoding;
        MediaManager::media* follower;

        union SplMediaProps {
            MediaManager::FmpFeederSplProps fmpFeederSplProps;
            MediaManager::FPRecorderExclProps fPRecorderExclProps;
        } splMediaProps;

        union ferrybuffer {
            std::valarray<ferryframe>* framebuffer = NULL;
            std::valarray<ferryperiod>* periodbuffer;
        } buffer;
        int bufferfloat;
        int buffersize;
        int priority;
        int signalNewState; //*< set 1 to restart and 0 to stop the media and 2 to continue.
        short state = -1; //*< it reflects the state of medias
        media();
        ~media();
        media& operator=(media& m);
    };

    static int capture(std::valarray<MediaManager::media>& inputs,
            std::valarray<MediaManager::media>& outputs);

    static void * fmp_feeder(void *);
    static void * fPRecorder(void *);

    struct fmp_feeder_return {
        std::string error;
        int errorcode;
    };

    struct fmp_feeder_args {
        media *iaudiomedia;
        media *ivideomedia;
        media *omedia;
        fmp_feeder_return ffr;
        ClientSocket * cs;
        std::mutex csm; //#csm mutex for creating deleting ClientSocket cs;
        FerryTimeStamp lastconnectFTS; //#fts FerryTimeStamep to store last connect time;
    };

    struct fPRecorderArgs {
        media *iaudiomedia;
        media *ivideomedia;
        media *omedia;
        int fd;
    };

    static void* capture(void*);

    struct capture_thread_iargs {
        std::valarray<media> * inputs;
        std::valarray<media> * outputs;
        ~capture_thread_iargs();
    };

    struct capture_thread_rargs {
        int retvalue;
    };

    struct raw_mjpeg_mp3_dump_return {
        std::string error;
        int errorcode;
    };

    struct raw_mjpeg_mp3_dump_args {
        media *iaudiomedia;
        media *ivideomedia;
        media *omedia;
        raw_mjpeg_mp3_dump_return rmmdr;
        bool* start_stop;
    };
    static void* raw_mjpeg_mp3_dump(void* args);
private:

    static void pthreadCleanupCapture(void*);

    class pthreadCleanupCaptureArgs {
    public:
        std::vector<pthread_t*> inpthreads;
        std::vector<pthread_t*> oupthreads;
        std::vector<void*> expired_objs;
        std::valarray<MediaManager::media>* inputs;
    };

    static void pthreadCleanupFMPFeeder(void*);

    class pthreadCleanupFMPFeederArgs {
    public:
        std::string * payload;
        ClientSocket * cs;
        std::mutex * csm;
        FerryTimeStamp * lastConnectFTS;
        int reconnectInterval;
        void* args = NULL;
        bool isSuccess;
        fmp_feeder_args* ffargs;
    };

    static void pthreadCleanupFPRecorder(void*);

    class pthreadCleanupFPRecorderArgs {
    public:
        std::string * payload;
        int fd;
        bool isSuccess;
        fPRecorderArgs* fprargs;
    };

    static void pthreadCleanupRawMJPEGMP3Dump(void*);

    class pthreadCleanupRawMJPEGMP3Dump {
    public:

    };

};

#endif	/* MEDIAMANAGER_H */

