/* 
 * File:   mypcm.h
 * Author: gowtham
 *
 * Created on 20 October, 2013, 9:36 AM
 */

#ifndef MYPCM_H
#define	MYPCM_H

#include <string>
#include <malloc.h>
#include <valarray>
#include <sys/uio.h>

int pcm_types_n_formats();
int pcm_open_set_device();
int playback(char*);

class ferryperiod {
public:
    char * period = NULL;
    int length = 0;
    short wordsize = 1;

    void initferryperiod(int length, short wordsize);

    ferryperiod();

    ~ferryperiod();
};

class VFerryPeriod {
public:
    iovec period;
    short wordsize = 1;
    void initferryperiod(int length, short wordsize);
    VFerryPeriod();
    ~VFerryPeriod();
};

class snd_record_return {
public:
    int errorcode = 0;
    std::string error = "";
    short * state = NULL;
};

/**
 * object that should be passed to snd_record. It encapsulate various arguments for the function
 * @see void* ::snd_record(void* voidargs)
 * 
 */
class snd_record_args {
public:
    std::string dev_name; //*< device name default
    int samplingFrequency; //*< its the sampling frequencey; typically 44100 samples per second for cd quality
    int duration; //*< to record continuously set to zero
    std::valarray<ferryperiod> *periodbuffer; //*< array priods
    int* periodbufferlength; //*< size of each period
    int* periodbufferfloat; //*< index above the new filled buffer
    snd_record_return returnObj; //*< return value. u find error code @see snd_record_return
};

void deallocate_srarg(void* buffer);

/**
 * records sound from specified device into ferrybuffer
 * @param voidargs
 * @return 
 */
void * snd_record(void* voidargs);
int pcm_open_set_device();

#endif	/* PCM_H */

