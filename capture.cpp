/*
 *  V4L2 video capture example, modified by Derek Molloy for the Logitech C920 camera
 *  Modifications, added the -F mode for H264 capture and associated help detail
 *  www.derekmolloy.ie
 *
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see http://linuxtv.org/docs.php for more information
 */

#include <base/JPEGImage.h>
#include "capture.h"
#include "debug.h"
#include <base/mystdlib.h>
#include <base/logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <valarray>

#include <time.h>
#include <iostream>
#include <pthread.h>
#include <malloc.h>

extern "C" {
#include <linux/videodev2.h>
}
#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer {
    void *start;
    size_t length;
};

//bool analyzedInitImage = false;
//bool analyzed2ndImage = false;


static std::map<v4l2_field, std::string> v4l2_field_str;

static void v4l2_field_str_init() {
    if (v4l2_field_str.size() > 0)return;
    v4l2_field_str[V4L2_FIELD_ANY] = "V4L2_FIELD_ANY";
    v4l2_field_str[V4L2_FIELD_NONE] = "V4L2_FIELD_NONE";
    v4l2_field_str[V4L2_FIELD_TOP] = "V4L2_FIELD_TOP";
    v4l2_field_str[V4L2_FIELD_BOTTOM] = "V4L2_FIELD_BOTTOM";
    v4l2_field_str[V4L2_FIELD_INTERLACED] = "V4L2_FIELD_INTERLACED";
    v4l2_field_str[V4L2_FIELD_SEQ_TB] = "V4L2_FIELD_SEQ_TB";
    v4l2_field_str[V4L2_FIELD_SEQ_BT] = "V4L2_FIELD_SEQ_BT";
    v4l2_field_str[V4L2_FIELD_ALTERNATE] = "V4L2_FIELD_ALTERNATE";
    v4l2_field_str[V4L2_FIELD_INTERLACED_TB] = "V4L2_FIELD_INTERLACED_TB";
    v4l2_field_str[V4L2_FIELD_INTERLACED_BT] = "V4L2_FIELD_INTERLACED_BT";
}

void ferryframe::initferryframe(int length) {
    free(this->frame);
    this->intframeptr = this->frame = malloc(length);
    this->length = length;
}

ferryframe::ferryframe() {
    this->frame = NULL;
    this->length = 0;
    this->readerscount = 0;
    this->wordsize = 1;
};

ferryframe::~ferryframe() {
    free(frame);
}

void VideoCapture::errnoExit(const char *s) {
    fp_err(FPOL_CAP, "%s: %s error %d: %s", devName, s, errno, strerror(errno));
    //exit(EXIT_FAILURE);
    pthread_exit(NULL);
}

static int xioctl(int fh, int request, void *arg) {
    int r;

    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

void VideoCapture::processImage(const void *p, int size) {
    if (outBuf) {
        (*frameBuffer)[*bufFloat].initferryframe(size);
        memcpy((*frameBuffer)[*bufFloat].frame, p, size);
        (*frameBuffer)[*bufFloat].readerscount = readersCount;
        *bufFloat = (*bufFloat + 1) % (*bufferSize);
    }
    fflush(stderr);
    finalFrameCount++;
#ifdef _DEBUG
    if ((debug & 16) == 16) {
        //        if (!analyzedInitImage) {
        //            analyzedInitImage = true;
        //            JPEGImage i((char*) p, size);
        //            std::string fn = "initialFrame.jpg";
        //            int s=i.saveImage(fn);
        //            if(s>0){
        //                fprintf(stdout,"successfully written %d bytes to initialFrame.jpg\n",s);
        //            }else{
        //                fprintf(stdout,"failed to save init image\n");
        //            };
        //        } else if(!analyzed2ndImage){
        //            analyzed2ndImage = true;
        //            JPEGImage i((char*) p, size);
        //            std::string fn = "secondFrame.jpg";
        //            int s=i.saveImage(fn);
        //            if(s>0){
        //                fprintf(stdout,"successfully written %d bytes to secondFrame.jpg\n",s);
        //            }else{
        //                fprintf(stdout,"failed to save 2nd image\n");
        //            };
        //        }
        if (finalFrameCount % framerate == 0) {
            fp_debug(FPOL_CAP_L, "%s", std::string(framerate, '.').c_str());
        }
        //fprintf(stdout, ".");
        //fflush(stdout);
    }
#endif
}

int VideoCapture::readFrame(void) {
    struct v4l2_buffer buf;
    unsigned int i;

    switch (io) {
        case IO_METHOD_READ:
            if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
                switch (errno) {
                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                    default:
                        errnoExit("read");
                }
            }

            processImage(buffers[0].start, buffers[0].length);
            break;

        case IO_METHOD_MMAP:
            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                    default:
                        errnoExit("VIDIOC_DQBUF");
                }
            }

            assert(buf.index < nBuffers);

            processImage(buffers[buf.index].start, buf.bytesused);

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errnoExit("VIDIOC_QBUF");
            break;

        case IO_METHOD_USERPTR:
            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;

            if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                    default:
                        errnoExit("VIDIOC_DQBUF");
                }
            }

            for (i = 0; i < nBuffers; ++i)
                if (buf.m.userptr == (unsigned long) buffers[i].start && buf.length == buffers[i].length)
                    break;

            assert(i < nBuffers);

            processImage((void *) buf.m.userptr, buf.bytesused);

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errnoExit("VIDIOC_QBUF");
            break;
    }

    return 1;
}

void VideoCapture::mainloop(void) {
    unsigned int count;
    unsigned int loopIsInfinite = 0;

    if (frameCount == 0) loopIsInfinite = 1; //infinite loop
    count = frameCount;
    time(&startTime);
    while (*signalNewState >= 0 && ((count-- > 0) || loopIsInfinite)) {
        for (;;) {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) {
                if (EINTR == errno)
                    continue;
                errnoExit("select");
            }

            if (0 == r) {
                //fprintf(stderr, "select timeout\n");
                errnoExit("select timeout");
            }

            if (readFrame())
                break;
            /* EAGAIN - continue select loop. */
        }
    }
    time(&stopTime);
}

void VideoCapture::stopCapturing(void* args) {
    enum v4l2_buf_type type;

    switch (io) {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type) && args == NULL)
                errnoExit("VIDIOC_STREAMOFF");
            break;
    }
}

void VideoCapture::startCapturing(void) {
    unsigned int i;
    enum v4l2_buf_type type;

    switch (io) {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
            for (i = 0; i < nBuffers; ++i) {
                struct v4l2_buffer buf;

                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    errnoExit("VIDIOC_QBUF");
            }
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                errnoExit("VIDIOC_STREAMON");
            break;

        case IO_METHOD_USERPTR:
            for (i = 0; i < nBuffers; ++i) {
                struct v4l2_buffer buf;

                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;
                buf.index = i;
                buf.m.userptr = (unsigned long) buffers[i].start;
                buf.length = buffers[i].length;

                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    errnoExit("VIDIOC_QBUF");
            }
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                errnoExit("VIDIOC_STREAMON");
            break;
    }
}

void VideoCapture::uninitDevice(void* args) {
    unsigned int i;

    switch (io) {
        case IO_METHOD_READ:
            free(buffers[0].start);
            break;

        case IO_METHOD_MMAP:
            for (i = 0; i < nBuffers; ++i)
                if (-1 == munmap(buffers[i].start, buffers[i].length) && args == NULL)
                    errnoExit("munmap");
            break;

        case IO_METHOD_USERPTR:
            for (i = 0; i < nBuffers; ++i)
                free(buffers[i].start);
            break;
    }

    free(buffers);
}

void VideoCapture::initRead(unsigned int buffer_size) {
    buffers = (buffer*) calloc(1, sizeof (*buffers));

    if (!buffers) {
        //fprintf(stderr, "Out of memory\n");
        errnoExit("Out of memory");
    }

    buffers[0].length = buffer_size;
    buffers[0].start = malloc(buffer_size);

    if (!buffers[0].start) {
        //fprintf(stderr, "Out of memory\n");
        errnoExit("Out of memory");
    }
}

void VideoCapture::initMmap(void) {
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            //fprintf(stderr, "%s does not support memory mapping\n", devName);
            errnoExit("device does not support memory mapping");
        } else {
            errnoExit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        //fprintf(stderr, "Insufficient buffer memory on %s\n",devName);
        errnoExit("Insufficient buffer memory on device");
    }

    buffers = (buffer*) calloc(req.count, sizeof (*buffers));

    if (!buffers) {
        //fprintf(stderr, "Out of memory\n");
        errnoExit("Out of memory\n");
    }

    for (nBuffers = 0; nBuffers < req.count; ++nBuffers) {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = nBuffers;

        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
            errnoExit("VIDIOC_QUERYBUF");

        buffers[nBuffers].length = buf.length;
        buffers[nBuffers].start =
                mmap(NULL /* start anywhere */,
                buf.length,
                PROT_READ | PROT_WRITE /* required */,
                MAP_SHARED /* recommended */,
                fd, buf.m.offset);

        if (MAP_FAILED == buffers[nBuffers].start)
            errnoExit("mmap");
    }
}

void VideoCapture::initUserp(unsigned int buffer_size) {
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            //fprintf(stderr, "%s does not support user pointer i/o\n", devName);
            errnoExit("device does not support user pointer i/o");
        } else {
            errnoExit("VIDIOC_REQBUFS");
        }
    }

    buffers = (buffer*) calloc(4, sizeof (*buffers));

    if (!buffers) {
        //fprintf(stderr, "Out of memory\n");
        //exit(EXIT_FAILURE);
        errnoExit("Out of memory");
    }

    for (nBuffers = 0; nBuffers < 4; ++nBuffers) {
        buffers[nBuffers].length = buffer_size;
        buffers[nBuffers].start = malloc(buffer_size);

        if (!buffers[nBuffers].start) {
            //fprintf(stderr, "Out of memory\n");
            //exit(EXIT_FAILURE);
            errnoExit("Out of memory");
        }
    }
}

void VideoCapture::initDevice(void) {
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    struct v4l2_frmivalenum argp;
    unsigned int min;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            //fprintf(stderr, "%s is no V4L2 device\n", devName);
            //exit(EXIT_FAILURE);
            errnoExit("device is no v4l2 device");
        } else {
            errnoExit("VIDIOC_QUERYCAP");
        }
    }
    fp_debug(FPOL_CAP, "capabilities of %s:\n\tndriver: %s\n\tcard: %s\n\tbus_info: %s\n\tversion: %u\n\tcapabilities: %u\n\tdevice_caps: %u\n\treserved: %u,%u,%u", devName, cap.driver, cap.card, cap.bus_info, cap.version, cap.capabilities, cap.device_caps, cap.reserved[0], cap.reserved[1], cap.reserved[2]);
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        //        fprintf(stderr, "%s is no video capture device\n", devName);
        //        exit(EXIT_FAILURE);
        errnoExit("device is no video capture device");
    }

    switch (io) {
        case IO_METHOD_READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                //                fprintf(stderr, "%s does not support read i/o\n",devName);
                //                exit(EXIT_FAILURE);
                errnoExit("device does not support read i/o");
            }
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                //                fprintf(stderr, "%s does not support streaming i/o\n", devName);
                //                exit(EXIT_FAILURE);
                errnoExit("device does not support streaming i/o");
            }
            break;
    }


    /* Select video input, video standard and tune here. */


    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
            switch (errno) {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    break;
            }
        }
    } else {
        /* Errors ignored. */
    }
    fp_debug(FPOL_CAP, "%s crop capabilities:\n\tbounds:\n\t\tleft: %d\n\t\ttop: %d\n\t\twidth: %d\n\t\theight: %d\n\tdefrect:\n\t\tleft: %d\n\t\ttop: %d\n\t\twidth: %d\n\t\theight: %d\n\tpixelaspect: %u/%u\n\ttype:%u", devName, cropcap.bounds.left, cropcap.bounds.top, cropcap.bounds.width, cropcap.bounds.height, cropcap.defrect.left, cropcap.defrect.top, cropcap.defrect.width, cropcap.defrect.height, cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator, cropcap.type);
    argp.index = 0;
    argp.width = width;
    argp.height = height;
    argp.pixel_format = V4L2_PIX_FMT_MJPEG;
    if (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &argp) < 0) {
        //exit(EXIT_FAILURE);
        errnoExit("VIDIOC_ENUM_FRAMEINTERVALS");
    }

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fp_debug(FPOL_CAP, "input_format=%s", forceFormat);
    if (forceFormat) {
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;

        if (strcmp(forceFormat, "h264") == 0) {
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
            fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        } else if (strcmp(forceFormat, "mjpeg") == 0) {
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
            fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        }

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
            if (-1 < xioctl(fd, VIDIOC_G_FMT, &fmt)) {
                fp_debug(FPOL_CAP, "%s is set to pixel format:\n\tpixelformat:\t%s\n\tfield:\t\t%s\n\twidth:\t\t%d\n\theight:\t\t%d", devName, std::string((char*) &fmt.fmt.pix.pixelformat, 4).c_str(), v4l2_field_str[(v4l2_field) fmt.fmt.pix.field].c_str(), fmt.fmt.pix.width, fmt.fmt.pix.height);
            }
            errnoExit("VIDIOC_S_FMT");
        }

        /* Note VIDIOC_S_FMT may change width and height. */
    } else {
        /* Preserve original settings as set by v4l2-ctl for example */
        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt)) {
            errnoExit("VIDIOC_G_FMT");
        } else {

        }
    }

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    struct v4l2_streamparm parm;
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.denominator = framerate;
    parm.parm.capture.timeperframe.numerator = 1;
    if (ioctl(fd, VIDIOC_S_PARM, &parm) < 0) {
        errnoExit("VIDIOC_S_PARM");
    };
    if (ioctl(fd, VIDIOC_G_PARM, &parm) < 0) {
        errnoExit("VIDIOC_G_PARM");
    };
    framerate = parm.parm.capture.timeperframe.denominator;
    frameCount = framerate*duration;
    switch (io) {
        case IO_METHOD_READ:
            initRead(fmt.fmt.pix.sizeimage);
            break;

        case IO_METHOD_MMAP:
            initMmap();
            break;

        case IO_METHOD_USERPTR:
            initUserp(fmt.fmt.pix.sizeimage);
            break;
    }
}

void VideoCapture::closeDevice(void* args) {
    if (-1 == close(fd) && args == NULL)
        errnoExit("close");

    fd = -1;
}

void VideoCapture::openDevice(void) {
    struct stat st;

    if (-1 == stat(devName, &st)) {
        //        fprintf(stderr, "Cannot identify '%s': %d, %s\n",devName, errno, strerror(errno));
        //        exit(EXIT_FAILURE);
        errnoExit("Cannot identify device");
    }

    if (!S_ISCHR(st.st_mode)) {
        //        fprintf(stderr, "%s is no device\n", devName);
        //        exit(EXIT_FAILURE);
        errnoExit("is not a device");
    }

    fd = open(devName, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == fd) {
        //        fprintf(stderr, "Cannot open '%s': %d, %s\n",devName, errno, strerror(errno));
        //        exit(EXIT_FAILURE);
        errnoExit("Cannot open device");
    }
}

static void usage(FILE *fp, int argc, char **argv) {
    fprintf(fp,
            "Usage:  [options]\n\n"
            "Version 1.3\n"
            "Options:\n"
            "-d | --device name   Video device name [%s]\n"
            "-h | --help          Print this message\n"
            "-m | --mmap          Use memory mapped buffers [default]\n"
            "-r | --read          Use read() calls\n"
            "-u | --userp         Use application allocated buffers\n"
            "-o | --output        Outputs stream to stdout\n"
            "-f | --format        Force format to 640x480 YUYV\n"
            "-F | --formatH264    Force format to 1920x1080 H264\n"
            "-c | --count         Number of frames to grab [] - use 0 for infinite\n"
            "\n"
            "Example usage: capture -F -o -c 300 > output.raw\n"
            "Captures 300 frames of H264 at 1920x1080 - use raw2mpg4 script to convert to mpg4\n",
            argv[0]);
}

void VideoCapture::printVideoparams() {
    std::cerr << "\nsize : " << width << "x" << height << "\n";
    //std::cerr << "\nformat : " << std::string(forceFormat) << "\n";
    std::cerr << "\nframe count :" << frameCount << "\n";
    std::cerr << "\nfinal_frame count :" << finalFrameCount << "\n";
    std::cerr << "\nduration :" << stopTime - startTime << "\n";
    float framerate = (float) ((float) finalFrameCount / (stopTime - startTime));
    std::cerr << "\nframe_rate :" << framerate << "\n";
    fflush(stdout);
}

static const char short_options[] = "d:hmruof:c:i:s:O:";

static const struct option
long_options[] = {
    { "device", required_argument, NULL, 'd'},
    { "help", no_argument, NULL, 'h'},
    { "mmap", no_argument, NULL, 'm'},
    { "read", no_argument, NULL, 'r'},
    { "userp", no_argument, NULL, 'u'},
    { "output", no_argument, NULL, 'o'},
    { "format", 1, NULL, 'f'},
    { "count", required_argument, NULL, 'c'},
    { "input", 1, NULL, 'i'},
    { "size", 1, NULL, 's'},
    { "outfile", 1, NULL, 'O'},
    { 0, 0, 0, 0}
};

void pthreadCancelVideoCapture(void* buffer) {
    struct capture_args* arg = (capture_args*) buffer;
    *arg->returnObj.state = -1;
};

class objMemberCallerArg {
public:
    void(VideoCapture::*memfunc)(void*);
    VideoCapture* vc;
    void* arg;
};

static void objMemberCaller(void* arg) {
    objMemberCallerArg* omca = (objMemberCallerArg*) arg;
    ((*omca->vc).*(omca->memfunc))(omca->arg);
}

void* videocapture(void * voidarg) {
    VideoCapture vc;
    char buf[20];
    int i = 0;
    int j = 0;
    struct capture_args* arg = (capture_args*) voidarg;
    *arg->returnObj.state = 0;
    v4l2_field_str_init();
    fp_debug(FPOL_CAP, "arg->device=%s", arg->device.c_str());
    //arg->framebuffer->resize(arg->buffersize); //(ferryframe*) calloc(arg->buffersize, sizeof (ferryframe));
    pthread_cleanup_push(pthreadCancelVideoCapture, arg);
    vc.readersCount = arg->readerscount;
    if (arg->device.length() > 0) {
        i = arg->device.length();
        memcpy(vc.devName, arg->device.c_str(), i);
        vc.devName[i + 1] = 0;
    }
    if (arg->mmap) {
        vc.io = IO_METHOD_MMAP;
    }
    if (arg->read) {
        vc.io = IO_METHOD_READ;
    }
    if (arg->userp) {
        vc.io = IO_METHOD_USERPTR;
    }
    vc.outBuf++;
    if (arg->format.length() > 0) {
        vc.forceFormat = (char*) arg->format.c_str();
    }
    //frameCount = arg->count;
    vc.width = arg->Width > 0 ? arg->Width : vc.width;
    vc.height = arg->Height > 0 ? arg->Height : vc.height;
    vc.frameBuffer = arg->framebuffer;
    vc.bufferSize = arg->buffersize;
    vc.bufFloat = arg->bufferfloat;
    vc.framerate = arg->framerate;
    vc.duration = arg->duration;
    vc.signalNewState = arg->signalNewState;
    strcpy(vc.devName, arg->device.c_str());
    int closeDeviceArg = 1;
    objMemberCallerArg omcaCD;
    omcaCD.arg = &closeDeviceArg;
    omcaCD.memfunc = &VideoCapture::closeDevice;
    omcaCD.vc = &vc;
    vc.openDevice();
    pthread_cleanup_push(objMemberCaller, (void*) &omcaCD);
    int uninitDeviceArg = 1;
    vc.initDevice();
    objMemberCallerArg omcaID;
    omcaID.arg = &uninitDeviceArg;
    omcaID.memfunc = &VideoCapture::uninitDevice;
    omcaID.vc = &vc;
    pthread_cleanup_push(objMemberCaller, (void*) &omcaID);
    *arg->returnObj.state = 1;
    int stopCapturingArg = 1;
    objMemberCallerArg omcaSC;
    omcaSC.arg = &closeDeviceArg;
    omcaSC.memfunc = &VideoCapture::stopCapturing;
    omcaSC.vc = &vc;
    vc.startCapturing();
    pthread_cleanup_push(objMemberCaller, (void*) &omcaSC);
    vc.mainloop();
    vc.stopCapturing(NULL);
    pthread_cleanup_pop(0);
    vc.uninitDevice(NULL);
    pthread_cleanup_pop(0);
    vc.closeDevice(NULL);
    pthread_cleanup_pop(0);
    dup2(stdoutfd, 1);
    arg->framerate = vc.framerate;
    //#ifdef _DEBUG    
    //    fp_debug(FPOL_CAP, "\n--------videoparams-------");
    //    if (fp_debug_lvl(FPOL_CAP)) {
    //        print_videoparams();
    //    }
    //    fp_debug(FPOL_CAP, "\n--------videoparams-------");
    //#endif
    *arg->returnObj.state = -1;
    pthread_cleanup_pop(0);
    return NULL;
}
