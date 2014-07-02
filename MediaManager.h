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
    static void* fmp_feeder_aftermath(void* args, bool isSucess);

    enum MediaType {
        AUDIO, VIDEO
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
        } splMediaProps;

        union ferrybuffer {
            std::valarray<ferryframe>* framebuffer = NULL;
            std::valarray<ferryperiod>* periodbuffer;
        } buffer;
        int bufferfloat;
        int buffersize;
        int priority;
        short state = -1;
        media();
        ~media();
        media& operator=(media& m);
    };

    static int capture(std::valarray<MediaManager::media> inputs, std::valarray<MediaManager::media> outputs);

    static void * fmp_feeder(void *);

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

    struct fmp_feeder_aftermath_args {
        std::string * payload;
        ClientSocket * cs;
        std::mutex * csm;
        FerryTimeStamp * lastConnectFTS;
        int reconnectInterval;
    };

    static void* capture(void*);

    struct capture_thread_iargs {
        std::valarray<media> * inputs;
        std::valarray<media> * outputs;
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
    };
    static void* raw_mjpeg_mp3_dump(void* args);
private:

};

#endif	/* MEDIAMANAGER_H */

