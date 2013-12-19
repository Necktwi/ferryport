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

int pcm_types_n_formats();
int pcm_open_set_device();
int playback();

class ferryperiod {
public:
    char * period=NULL;
    int length=0;
    short wordsize=1;

    void initferryperiod(int length, short wordsize);

    ferryperiod();

    ~ferryperiod();
};

class snd_record_return {
public:
    int errorcode = 0;
    std::string error = "";
    short * state;
};

class snd_record_args {
public:
    std::string dev_name;
    int samplingFrequency;
    int duration; //to record continuously set to zero
    std::valarray<ferryperiod> *periodbuffer;
    int* periodbufferlength;
    int* periodbufferfloat;
    snd_record_return returnObj;
};

void deallocate_srarg(void* buffer);
void * snd_record(void* voidargs);
int pcm_open_set_device();

#endif	/* PCM_H */

