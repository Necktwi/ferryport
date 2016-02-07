/* 
 * File:   debug.h
 * Author: gowtham
 *
 * Created on 7 June, 2013, 4:07 PM
 */
#ifndef DEBUG_H
#define	DEBUG_H
#include <base/mystdlib.h>
#include <map>
#include <utility>
#include <sys/types.h>
#include <base/logger.h>

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


extern _ff_log_type fp_log_type;
extern unsigned int fp_log_level;


#define fp_notice(level,...) ffl_notice(fp_log_type,fp_log_level,level,__VA_ARGS__)
#define fp_warn(level,...) ffl_warn(fp_log_type,fp_log_level,level,__VA_ARGS__)
#define fp_err(level,...) ffl_err(fp_log_type,fp_log_level,level,__VA_ARGS__)
#define fp_info(level,...) ffl_info(fp_log_type,fp_log_level,level,__VA_ARGS__)

/*
 *  weaker logging can be deselected at configure time using --disable-debug
 *  that gets rid of the overhead of checking while keeping _warn and _err
 *  active
 */
#ifdef _DEBUG

#define fp_debug(level,...) ffl_debug(fp_log_type,fp_log_level,level,__VA_ARGS__)
#define ffl_parser(...) _ff_log(FFL_PARSER, __VA_ARGS__)
#define ffl_header(...)  _ff_log(FFL_HEADER, __VA_ARGS__)
#define ffl_ext(...)  _ff_log(FFL_EXT, __VA_ARGS__)
#define ffl_client(...) _ff_log(FFL_CLIENT, __VA_ARGS__)
#define ffl_latency(...) _ff_log(FFL_LATENCY, __VA_ARGS__)
#endif  

#endif	/* DEBUG_H */

