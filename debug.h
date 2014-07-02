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
    FPOL_GPS = 1 << 1,
    FPOL_PCM = 1 << 2
};

#endif	/* DEBUG_H */

