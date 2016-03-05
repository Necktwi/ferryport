#include "debug.h"
#include <sys/types.h>
#include <base/mystdlib.h>

#ifdef DEBUG
int debug = 0;
#endif
int stdinfd;
int stdoutfd;
int stderrfd;
//std::map<pid_t, spawn*> processMap;
int child_exit_status;

_ff_log_type fp_log_type = (_ff_log_type) (FFL_NOTICE | FFL_WARN | FFL_ERR |
		FFL_DEBUG);
unsigned int fp_log_level = FPOL_MAIN | FPOL_MM | FPOL_LAV;

