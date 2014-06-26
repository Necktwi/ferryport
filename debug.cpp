#include "debug.h"
#include <sys/types.h>
#include "mystdlib.h"

#ifdef DEBUG
int debug = 0;
#endif
int stdinfd;
int stdoutfd;
int stderrfd;
//std::map<pid_t, spawn*> processMap;
int child_exit_status;
