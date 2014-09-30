/* 
 * File:   capture.h
 * Author: gowtham
 *
 * Created on 6 October, 2013, 4:03 PM
 */

#ifndef CAPTURE_H
#define	CAPTURE_H

#include <string>
#include <malloc.h>
#include <valarray>

class ferryframe {
public:
    int length;
    void* frame = NULL;
    void* intframeptr = NULL;
    int wordsize;
    int readerscount;

    void initferryframe(int length);

    ferryframe();

    ~ferryframe();
};

class capture_return {
public:
    short errorcode = 0;
    std::string error = "";
    short * state = NULL;
};

class capture_args {
public:
    std::string device; //*< name of device e.g. /dev/video0
    bool mmap; //*< use mapped memory access or not. prefer true if device supports it. //to be deprecated
    bool read; //*< prefer false //to be deprecated
    bool userp; //*< prefer false //to be deprecated
    std::string format;
    int duration; //*< duration of recording
    int Width; //*< width of the image
    int Height; //*< height of the image
    std::valarray<ferryframe> *framebuffer; //*< pointer to array that holds frames
    int* buffersize; //*< pointer to buffer size. It should be less than or eaqual to frame buffer true size
    int* bufferfloat; //*< pointer to frame index above the newly written frame
    int readerscount; //*< count of consumers grabbing a copy of produced count
    int framerate; //*< frames per second
    capture_return returnObj; //*< output data
    int* signalNewState = NULL;
};

void deallocate_vcarg(void* buffer);
void* videocapture(void *voidargs);

enum io_method {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

class VideoCapture {
public:
    void processImage(const void *p, int size);
    int readFrame(void);
    void mainloop(void);
    void stopCapturing(void* args);
    void startCapturing(void);
    void uninitDevice(void* args);
    void initRead(unsigned int buffer_size);
    void initMmap(void);
    void initUserp(unsigned int buffer_size);
    void initDevice(void);
    void closeDevice(void* args);
    void openDevice(void);
    void printVideoparams();
    void errnoExit(const char *s);
    char devName[20];
    enum io_method io = IO_METHOD_MMAP;
    int fd = -1;
    struct buffer *buffers;
    unsigned int nBuffers;
    int outBuf;
    char* forceFormat;
    int frameCount = 100;
    int width = 640;
    int height = 480;
    int finalFrameCount = 0;
    time_t startTime;
    time_t stopTime;
    int readersCount;
    int framerate;
    int duration;
    int* signalNewState = NULL;

    int *bufferSize;
    int *bufFloat;
    std::valarray<ferryframe> *frameBuffer;

};

#endif	/* CAPTURE_H */

