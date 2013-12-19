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
    void* frame;
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
    std::string device;
    bool mmap;
    bool read;
    bool userp;
    std::string format;
    int duration;
    int Width;
    int Height;
    std::valarray<ferryframe> *framebuffer;
    int* buffersize;
    int* bufferfloat;
    int readerscount;
    int framerate;
    capture_return returnObj;
};

void deallocate_vcarg(void* buffer);
void* videocapture(void *voidargs);

#endif	/* CAPTURE_H */

