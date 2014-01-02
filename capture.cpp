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

#include "capture.h"
#include "debug.h"
#include "mystdlib.h"
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

enum io_method {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

struct buffer {
    void *start;
    size_t length;
};

static char dev_name[20];
static enum io_method io = IO_METHOD_MMAP;
static int fd = -1;
struct buffer *buffers;
static unsigned int n_buffers;
static int out_buf;
static char* force_format;
static int frame_count = 100;
static int width = 640;
static int height = 480;
static int ofd = -1;
static int final_frame_count = 0;
static time_t start_time;
static time_t stop_time;
static int readerscount;
static int framerate;
static int duration;

int *buffer_size;
int *buf_float;
std::valarray<ferryframe> *frameBuffer;

void ferryframe::initferryframe(int length) {
    free(this->frame);
    this->frame = malloc(length);
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

static void errno_exit(const char *s) {
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
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

static void process_image(const void *p, int size) {
    if (out_buf) {
        //fwrite(p, size, 1, stdout);
        //if()
        (*frameBuffer)[*buf_float].initferryframe(size);
        memcpy((*frameBuffer)[*buf_float].frame, p, size);
        (*frameBuffer)[*buf_float].readerscount = readerscount;
        *buf_float = (*buf_float + 1) % (*buffer_size);
    }
    fflush(stderr);
#ifdef DEBUG
    if ((debug & 16) == 16) {
        fprintf(stdout, ".");
        fflush(stdout);
    }
#endif
    final_frame_count++;
}

static int read_frame(void) {
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
                        errno_exit("read");
                }
            }

            process_image(buffers[0].start, buffers[0].length);
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
                        errno_exit("VIDIOC_DQBUF");
                }
            }

            assert(buf.index < n_buffers);

            process_image(buffers[buf.index].start, buf.bytesused);

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
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
                        errno_exit("VIDIOC_DQBUF");
                }
            }

            for (i = 0; i < n_buffers; ++i)
                if (buf.m.userptr == (unsigned long) buffers[i].start && buf.length == buffers[i].length)
                    break;

            assert(i < n_buffers);

            process_image((void *) buf.m.userptr, buf.bytesused);

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
            break;
    }

    return 1;
}

static void mainloop(void) {
    unsigned int count;
    unsigned int loopIsInfinite = 0;

    if (frame_count == 0) loopIsInfinite = 1; //infinite loop
    count = frame_count;
    time(&start_time);
    while ((count-- > 0) || loopIsInfinite) {
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
                errno_exit("select");
            }

            if (0 == r) {
                //fprintf(stderr, "select timeout\n");
                errno_exit("select timeout");
            }

            if (read_frame())
                break;
            /* EAGAIN - continue select loop. */
        }
    }
    time(&stop_time);
}

static void stop_capturing(void) {
    enum v4l2_buf_type type;

    switch (io) {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
                errno_exit("VIDIOC_STREAMOFF");
            break;
    }
}

static void start_capturing(void) {
    unsigned int i;
    enum v4l2_buf_type type;

    switch (io) {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
            for (i = 0; i < n_buffers; ++i) {
                struct v4l2_buffer buf;

                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    errno_exit("VIDIOC_QBUF");
            }
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                errno_exit("VIDIOC_STREAMON");
            break;

        case IO_METHOD_USERPTR:
            for (i = 0; i < n_buffers; ++i) {
                struct v4l2_buffer buf;

                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;
                buf.index = i;
                buf.m.userptr = (unsigned long) buffers[i].start;
                buf.length = buffers[i].length;

                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    errno_exit("VIDIOC_QBUF");
            }
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                errno_exit("VIDIOC_STREAMON");
            break;
    }
}

static void uninit_device(void) {
    unsigned int i;

    switch (io) {
        case IO_METHOD_READ:
            free(buffers[0].start);
            break;

        case IO_METHOD_MMAP:
            for (i = 0; i < n_buffers; ++i)
                if (-1 == munmap(buffers[i].start, buffers[i].length))
                    errno_exit("munmap");
            break;

        case IO_METHOD_USERPTR:
            for (i = 0; i < n_buffers; ++i)
                free(buffers[i].start);
            break;
    }

    free(buffers);
}

static void init_read(unsigned int buffer_size) {
    buffers = (buffer*) calloc(1, sizeof (*buffers));

    if (!buffers) {
        //fprintf(stderr, "Out of memory\n");
        errno_exit("Out of memory");
    }

    buffers[0].length = buffer_size;
    buffers[0].start = malloc(buffer_size);

    if (!buffers[0].start) {
        //fprintf(stderr, "Out of memory\n");
        errno_exit("Out of memory");
    }
}

static void init_mmap(void) {
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            //fprintf(stderr, "%s does not support memory mapping\n", dev_name);
            errno_exit("device does not support memory mapping");
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        //fprintf(stderr, "Insufficient buffer memory on %s\n",dev_name);
        errno_exit("Insufficient buffer memory on device");
    }

    buffers = (buffer*) calloc(req.count, sizeof (*buffers));

    if (!buffers) {
        //fprintf(stderr, "Out of memory\n");
        errno_exit("Out of memory\n");
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
            errno_exit("VIDIOC_QUERYBUF");

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
                mmap(NULL /* start anywhere */,
                buf.length,
                PROT_READ | PROT_WRITE /* required */,
                MAP_SHARED /* recommended */,
                fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start)
            errno_exit("mmap");
    }
}

static void init_userp(unsigned int buffer_size) {
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            //fprintf(stderr, "%s does not support user pointer i/o\n", dev_name);
            errno_exit("device does not support user pointer i/o");
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    buffers = (buffer*) calloc(4, sizeof (*buffers));

    if (!buffers) {
        //fprintf(stderr, "Out of memory\n");
        //exit(EXIT_FAILURE);
        errno_exit("Out of memory");
    }

    for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
        buffers[n_buffers].length = buffer_size;
        buffers[n_buffers].start = malloc(buffer_size);

        if (!buffers[n_buffers].start) {
            //fprintf(stderr, "Out of memory\n");
            //exit(EXIT_FAILURE);
            errno_exit("Out of memory");
        }
    }
}

static void init_device(void) {
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    struct v4l2_frmivalenum argp;
    unsigned int min;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            //fprintf(stderr, "%s is no V4L2 device\n", dev_name);
            //exit(EXIT_FAILURE);
            errno_exit("device is no v4l2 device");
        } else {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }
#ifdef DEBUG
    if ((debug & 16) == 16) {
        //std::cout << "\ndriver:" << cap.driver << "\ncard:" << cap.card << "\nbus_info:" << cap.bus_info << "\nversion:" << cap.version << "\ncapabilities:" << cap.capabilities << "\ndevice_caps:" << cap.device_caps << "\nreserved:" << cap.reserved << "\n";
        fflush(stdout);
    }
#endif

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        //        fprintf(stderr, "%s is no video capture device\n", dev_name);
        //        exit(EXIT_FAILURE);
        errno_exit("device is no video capture device");
    }

    switch (io) {
        case IO_METHOD_READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                //                fprintf(stderr, "%s does not support read i/o\n",dev_name);
                //                exit(EXIT_FAILURE);
                errno_exit("device does not support read i/o");
            }
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                //                fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
                //                exit(EXIT_FAILURE);
                errno_exit("device does not support streaming i/o");
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
#ifdef DEBUG
    if ((debug & 16) == 16) {
        //std::cout << "\ncropcap.bounds:" << (const unsigned char*)cropcap.bounds << "\ndefract:" << cropcap.defrect << "\npixelaspect:" << cropcap.pixelaspect << "\ntype:" << cropcap.type << "\n";
        fflush(stdout);
    }
#endif
    argp.index = 5;
    argp.width = 320;
    argp.height = 240;
    argp.pixel_format = V4L2_PIX_FMT_MJPEG;
    if (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &argp) < 0) {
        //exit(EXIT_FAILURE);
        errno_exit("VIDIOC_ENUM_FRAMEINTERVALS");
    }

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
#ifdef DEBUG
    if ((debug & 16) == 16) {
        std::cout << "\n" << getTime() << " caputure: input_format=" << force_format << "\n";
        fflush(stdout);
    }
#endif
    if (force_format) {
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;

        if (strcmp(force_format, "h264") == 0) {
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
            fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        } else if (strcmp(force_format, "mjpeg") == 0) {
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
            fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        }

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
            errno_exit("VIDIOC_S_FMT");

        /* Note VIDIOC_S_FMT may change width and height. */
    } else {
        /* Preserve original settings as set by v4l2-ctl for example */
        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
            errno_exit("VIDIOC_G_FMT");
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
        errno_exit("VIDIOC_S_PARM");
    };
    if (ioctl(fd, VIDIOC_G_PARM, &parm) < 0) {
        errno_exit("VIDIOC_G_PARM");
    };
    framerate = parm.parm.capture.timeperframe.denominator;
    frame_count = framerate*duration;
    switch (io) {
        case IO_METHOD_READ:
            init_read(fmt.fmt.pix.sizeimage);
            break;

        case IO_METHOD_MMAP:
            init_mmap();
            break;

        case IO_METHOD_USERPTR:
            init_userp(fmt.fmt.pix.sizeimage);
            break;
    }
}

static void close_device(void) {
    if (-1 == close(fd))
        errno_exit("close");

    fd = -1;
}

static void open_device(void) {
    struct stat st;

    if (-1 == stat(dev_name, &st)) {
        //        fprintf(stderr, "Cannot identify '%s': %d, %s\n",dev_name, errno, strerror(errno));
        //        exit(EXIT_FAILURE);
        errno_exit("Cannot identify device");
    }

    if (!S_ISCHR(st.st_mode)) {
        //        fprintf(stderr, "%s is no device\n", dev_name);
        //        exit(EXIT_FAILURE);
        errno_exit("is not a device");
    }

    fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == fd) {
        //        fprintf(stderr, "Cannot open '%s': %d, %s\n",dev_name, errno, strerror(errno));
        //        exit(EXIT_FAILURE);
        errno_exit("Cannot open device");
    }
}

static void usage(FILE *fp, int argc, char **argv) {
    fprintf(fp,
            "Usage: %s [options]\n\n"
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
            "-c | --count         Number of frames to grab [%i] - use 0 for infinite\n"
            "\n"
            "Example usage: capture -F -o -c 300 > output.raw\n"
            "Captures 300 frames of H264 at 1920x1080 - use raw2mpg4 script to convert to mpg4\n",
            argv[0], dev_name, frame_count);
}

static void print_videoparams() {
    std::cerr << "\nsize : " << width << "x" << height << "\n";
    //std::cerr << "\nformat : " << std::string(force_format) << "\n";
    std::cerr << "\nframe count :" << frame_count << "\n";
    std::cerr << "\nfinal_frame count :" << final_frame_count << "\n";
    std::cerr << "\nduration :" << stop_time - start_time << "\n";
    float framerate = (float) ((float) final_frame_count / (stop_time - start_time));
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

void deallocate_vcarg(void* buffer) {
    struct capture_args* arg = (capture_args*) buffer;
    *arg->returnObj.state = -1;
};

void* videocapture(void * voidarg) {
    char buf[20];
    int i = 0;
    int j = 0;
    struct capture_args* arg = (capture_args*) voidarg;
    *arg->returnObj.state = 0;
#ifdef DEBUG
    if ((debug & 16) == 16) {
        std::cout << "\n" << getTime() << " videocapture: arg->device=" << arg->device << "\n";
        fflush(stdout);
    }
#endif
    //arg->framebuffer->resize(arg->buffersize); //(ferryframe*) calloc(arg->buffersize, sizeof (ferryframe));
    pthread_cleanup_push(deallocate_vcarg, arg);
    readerscount = arg->readerscount;
    if (arg->device.length() > 0) {
        i = arg->device.length();
        memcpy(dev_name, arg->device.c_str(), i);
        dev_name[i + 1] = 0;
    }
    if (arg->mmap) {
        io = IO_METHOD_MMAP;
    }
    if (arg->read) {
        io = IO_METHOD_MMAP;
    }
    if (arg->userp) {
        io = IO_METHOD_USERPTR;
    }
    out_buf++;
    if (arg->format.length() > 0) {
        force_format = (char*) arg->format.c_str();
    }
    //frame_count = arg->count;
    width = arg->Width > 0 ? arg->Width : width;
    height = arg->Height > 0 ? arg->Height : height;
    frameBuffer = arg->framebuffer;
    buffer_size = arg->buffersize;
    buf_float = arg->bufferfloat;
    framerate = arg->framerate;
    duration = arg->duration;
    strcpy(dev_name, arg->device.c_str());
    open_device();
    init_device();
    start_capturing();
    mainloop();
    stop_capturing();
    uninit_device();
    close_device();
    fprintf(stderr, "\n");
    close(ofd);
    dup2(stdoutfd, 1);
    arg->framerate = framerate;
#ifdef DEBUG
    if ((debug & 16) == 16) {
        std::cout << "\n" << getTime() << " videocapture:\n";
        print_videoparams();
        fflush(stdout);
    }
#endif
    pthread_cleanup_pop(1);
    return NULL;
}

void stream(int width, int height, int fps, std::string url) {

}