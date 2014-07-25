/* 
 * File:   debug.h
 * Author: gowtham
 *
 * Created on 7 June, 2013, 4:07 PM
 */
#ifndef DEBUG_H
#define	DEBUG_H
#include "mystdlib.h"
#include <map>
#include <utility>
#include <sys/types.h>

#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))
#define DEBUG
extern int debug;
extern int stdinfd;
extern int stdoutfd;
extern int stderrfd;
//extern std::map<pid_t, spawn*> processMap;
extern int child_exit_status;

enum FPOL_LEVEL {
    FPOL_MAIN = 1 << 0,
    FPOL_SPAWN = 1 << 1,
    FPOL_SOCKET = 1 << 2,
    FPOL_GPS = 1 << 3,
    FPOL_MM = 1 << 4, //*< MediaManager
    FPOL_LL = 1 << 5, //*< lowlevel calls like siganlHandler, spawn
    FPOL_PCM = 1 << 6, //*< pulse code modulation(sound recording)
    FPOL_CAP = 1 << 7, //*< video capture
    FPOL_CAP_L = 1 << 8, //*< low level or frequent log
    FPOL_LAV = 1 << 9, //*< libavcodec

    NO_NEW_LINE = 1 << 31 //*< or it with above options for the log to be terminated with out new line
};

#endif	/* DEBUG_H */

