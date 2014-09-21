/* 
 * File:   audio.h
 * Author: gowtham
 *
 * Created on 15 April, 2014, 6:30 PM
 */

#ifndef AUDIO_H
#define	AUDIO_H

#include <sys/types.h>

class audio {
public:
    audio();
    audio(const audio& orig);
    static int record(void* buf);
    virtual ~audio();

private:
    static ssize_t loop_write(int fd, const void*data, size_t size);
};

#endif	/* AUDIO_H */

