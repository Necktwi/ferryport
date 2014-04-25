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
    std::string device;//*< name of device e.g. /dev/video0
    bool mmap;//*< use mapped memory access or not. prefer true if device supports it. //to be deprecated
    bool read;//*< prefer false //to be deprecated
    bool userp;//*< prefer false //to be deprecated
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
};

void deallocate_vcarg(void* buffer);
void* videocapture(void *voidargs);

#endif	/* CAPTURE_H */

