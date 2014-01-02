#include "mystdlib.h"
#include "myconverters.h"
#include "debug.h"
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <istream>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <sys/stat.h> 
#include <ftw.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <signal.h>
#include <vector>
#include <wait.h>
#include <malloc.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <assert.h>
#include <sstream>
#include <cerrno>
#include <ext/stdio_filebuf.h>
#include <signal.h>
#include <list>

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
static struct termios old, mnew;

/* Initialize new terminal i/o settings */
void initTermios(int echo) {
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    mnew = old; /* make new settings same as old settings */
    mnew.c_lflag &= ~ICANON; /* disable buffered i/o */
    mnew.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &mnew); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) {
    tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) {
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

/* Read 1 character without echo */
char getch(void) {
    return getch_(0);
}

/* Read 1 character with echo */
char getche(void) {
    return getch_(1);
}

bool spawn::processCleaned;

void spawn::defaultOnStopHandler(spawn* process) {
    //delete process;
}

spawn::spawn() {

}

spawn::spawn(std::string command, bool daemon, void (*onStopHandler)(spawn*), bool freeChild, bool block) {
    sigset_t chldmask;
    if ((sigemptyset(&chldmask) == -1) || (sigaddset(&chldmask, SIGCHLD) == -1)) {
        throw "Failed to initialize the signal mask.";
    }
    pipe(this->cpstdinp);
    pipe(this->cpstdoutp);
    pipe(this->cpstderrp);
    std::vector<std::string> cmdv = explode(" ", command);
    char * args[cmdv.size() + 1];
    int i = 0;
    int a = 0;
    bool validcmd = true;
    std::string arg;
    for (i = 0; i < cmdv.size(); i++) {
        arg = std::string(cmdv[i]);
        if (cmdv[i][0] == '"') {
            arg = arg.substr(1);
            i++;
            while (i < cmdv.size() && cmdv[i][cmdv[i].length() - 1] != '"') {
                arg += " " + cmdv[i];
                i++;
            }
            if (i < cmdv.size() && cmdv[i][cmdv[i].length() - 1] == '"') {
                arg += " " + cmdv[i];
                arg = arg.substr(0, arg.length() - 1);
            } else {
                validcmd = false;
                break;
            }
        } else if (cmdv[i][0] == '\'') {
            arg = arg.substr(1);
            i++;
            while (i < cmdv.size() && cmdv[i][cmdv[i].length() - 1] != '\'') {
                arg += " " + cmdv[i];
                i++;
            }
            if (i < cmdv.size() && cmdv[i][cmdv[i].length() - 1] == '\'') {
                arg += " " + cmdv[i];
                arg = arg.substr(0, arg.length() - 1);
            } else {
                validcmd = false;
                break;
            }
        } else if (arg[arg.length() - 1] == '\\') {
            arg = arg.substr(0, arg.length() - 1) + " " + cmdv[++i];
        }
        char * buf = (char*) malloc(arg.length() + 1);
        arg.copy(buf, arg.length(), 0);
        buf[arg.length()] = '\0';
        args[a] = buf;
        ++a;
    }
    args[a] = NULL;
    this->cmdName = std::string(args[0]);
    this->cmd = command;
    if (onStopHandler == NULL) {
        onStopHandler = &spawn::defaultOnStopHandler;
    }
    if (validcmd) {
        if (pthread_sigmask(SIG_BLOCK, &chldmask, NULL) == -1) {
            throw "Failed to block signal SIGCHLD.";
        }
        this->cpid = fork();
        if (this->cpid == 0) {
            if (pthread_sigmask(SIG_UNBLOCK, &chldmask, NULL) == -1) {
                throw "Failed to unblock signal SIGCHLD.";
                exit(-2);
            }
            int fi;
            if (!freeChild) {
                prctl(PR_SET_PDEATHSIG, SIGHUP);
            }
            dup2(this->cpstdinp[0], 0);
            close(this->cpstdinp[1]);
            dup2(this->cpstdoutp[1], 1);
            close(this->cpstdoutp[0]);
            dup2(this->cpstderrp[1], 2);
            close(this->cpstderrp[0]);
            /*Most beautiful thing I ever commented :|*/
            /*if ((debug & 2) == 2) {
                close(this->cpstdinp[0]);
                close(this->cpstdoutp[1]);
                close(this->cpstderrp[1]);
                if (daemon) {
                    dup2(stderrfd, 1);
                    dup2(stderrfd, 2);
                } else {
                    dup2(stdinfd, 0);
                    dup2(stdoutfd, 1);
                    dup2(stdoutfd, 2);
                }
                std::cout << "\n" + getTime() + " spawn : daemon=" + std::string(itoa(daemon)) + "\n";
                fflush(stdout);
            } else if (daemon) {
                close(this->cpstdinp[0]);
                close(this->cpstdoutp[1]);
                close(this->cpstderrp[1]);
            }*/
            execvp(args[0], args);
            exit(127);
        } else {
            processMap[this->cpid] = this;
            this->childExitStatus = child_exit_status;
#ifdef DEBUG
            if ((debug & 32) == 32) {
                std::cout << "\n" << getTime() << " spawn: process " << getpid() << " spawned " << this->cmdName << " with pid:" << this->cpid << "\n";
                fflush(stdout);
            }
#endif
            this->onStopHandler = onStopHandler;
            close(this->cpstdinp[0]);
            this->cpstdin = this->cpstdinp[1];
            close(this->cpstdoutp[1]);
            this->cpstdout = this->cpstdoutp[0];
            close(this->cpstderrp[1]);
            this->cpstderr = this->cpstderrp[0];
            for (i = 0; i < a; i++) {
                free(args[a]);
            }
            if (block) {
                int pid;
wait_till_child_exit:
                pid = waitpid(this->cpid, &this->childExitStatus, 0);
                if (pid == -1) {
                    if (waitpid(this->cpid, &this->childExitStatus, WNOHANG) == 0) {
#ifdef DEBUG
                        if ((debug & 32) == 32) {
                            std::cout << "\n" << getTime() << " spawn: process " << getpid() << " received a signal during cmd: " << this->cmdName << "\n";
                            fflush(stdout);
                        }
#endif
                        goto wait_till_child_exit;
                    }
                }
#ifdef DEBUG
                if ((debug & 32) == 32) {
                    std::cout << "\n" << getTime() << " spawn: process " << getpid() << "'s child \"" << this->cmdName << "\" with pid " << pid << " exited.\n";
                    fflush(stdout);
                }
#endif
                onStopHandler(this);
            }
            if (pthread_sigmask(SIG_UNBLOCK, &chldmask, NULL) == -1) {
                throw "Failed to unblock signal SIGCHLD.";
            }
        }
    } else {

    }
}

int spawn::getChildExitStatus() {
    return this->childExitStatus;
}

int spawn::pkill(int signal) {
    return kill(this->cpid, signal);
}

int copyfile(std::string src, std::string dst) {
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(src.c_str(), O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0) {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0) {
                nread -= nwritten;
                out_ptr += nwritten;
            } else if (errno != EINTR) {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0) {
        if (close(fd_to) < 0) {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return -1;
}

std::string getCurrentDir() {
    char cCurrentPath[FILENAME_MAX];
    if (!GetCurrentDir(cCurrentPath, sizeof (cCurrentPath))) {
        return std::string(itoa(errno));
    }
    cCurrentPath[sizeof (cCurrentPath) - 1] = '\0'; /* not really required */
    return std::string(cCurrentPath);
}

std::string getMachineName() {
    //struct addrinfo hints, *info, *p;
    //int gai_result;

    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);

    //memset(&hints, 0, sizeof hints);
    //hints.ai_family = AF_UNSPEC; /*either IPV4 or IPV6*/
    //hints.ai_socktype = SOCK_STREAM;
    //hints.ai_flags = AI_CANONNAME;

    //if ((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0) {
    //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_result));
    //exit(1);
    //}
    //std::string mn = std::string(info->ai_canonname);
    //freeaddrinfo(info);
    return std::string(hostname);
}

static int jgjg(const char *fpath, const struct stat *sb, int tflag, struct FTW * ftwbuf) {
    unlink(fpath);
}

int rmmydir(std::string dirn) {
    //return nftw(dirn.c_str(), jgjg, 20, FTW_DEPTH | FTW_PHYS);
    std::string cmd = "rm -Rf " + dirn;
    system(cmd.c_str());
    return 1;
}

std::string inputText() {
    std::string buf;
    char ch;
    //while ((ch = getchar()) != '\n' && ch != EOF);
    ch = getch();
    while ((int) ch != 10) {
        if ((int) ch == '\b' || (int) ch == 127) {
            if (buf.size() > 0) {
                printf("\b \b");
                fflush(stdout);
                buf.erase(buf.size() - 1);
            }
        } else {
            buf.push_back(ch);
            std::cout << ch;
        }
        ch = getch();
    }
    return buf;
}

std::string inputPass() {
    char ch;
    std::string buf;
    //while ((ch = getchar()) != '\n' && ch != EOF);
    ch = getch();
    while ((int) ch != 10) {
        if ((int) ch == '\b' || (int) ch == 127) {
            if (buf.size() > 0) {
                printf("\b \b");
                fflush(stdout);
                buf.erase(buf.size() - 1);
            }
        } else {
            buf.push_back(ch);
            std::cout << '*';
        }
        ch = getch();
    }
    return buf;
}

std::string getStdoutFromCommand(std::string cmd) {
    std::string data;
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1"); // Do we want STDERR?

    stream = popen(cmd.c_str(), "r");
    if (stream) {
        while (!feof(stream))
            if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
        pclose(stream);
    }
    return data;
}

std::string getTime() {
    struct tm * timeinfo;
    char tb[20];
    time_t ct;
    time(&ct);
    timeinfo = localtime(&ct);
    strftime(tb, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(tb);
}

std::string get_command_line(pid_t pid) {
    FILE *f;
    char file[256], cmdline[256] = {0};
    sprintf(file, "/proc/%d/cmdline", pid);

    f = fopen(file, "r");
    if (f) {
        char *p = cmdline;
        fgets(cmdline, sizeof (cmdline) / sizeof (*cmdline), f);
        fclose(f);

        while (*p) {
            p += strlen(p);
            if (*(p + 1)) {
                *p = ' ';
            }
            p++;
        }
        return std::string(cmdline);
    } else {
        return std::string();
    }
}

int poke(std::string ip) {
    /*int mysocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    struct sockaddr_in sip;
    memset(&sip, '0', sizeof (sip));
    sip.sin_family = AF_INET;
    sip.sin_port = htons(80);
    if (inet_pton(AF_INET, ip.c_str(), &sip.sin_addr) <= 0) {
        return -1;
    }
    return connect(mysocket, (sockaddr*) & sip, sizeof (sip));*/
#ifdef DEBUG
    if ((debug & 1) == 1) {
        std::cout << "\n" + getTime() + " poking " + ip + "....";
        fflush(stdout);
    }
#endif
    ip = GetPrimaryIp();
#ifdef DEBUG
    if ((debug & 1) == 1) {
        std::cout << "with ipadress: " + ip + "\n";
        fflush(stdout);
    }
#endif
    if (ip.length() == 0) {
        return 1;
    } else {
        return 0;
    }
}

int getIp() {
    struct ifaddrs * ifAddrStruct = NULL;
    struct ifaddrs * ifa = NULL;
    void * tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr = &((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        }
    }
    if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
    return 0;
}

std::string GetPrimaryIp() {
    char buffer[16];
    int buflen = 16;
    buffer[0] = '\0';
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock != -1) {

        const char* kGoogleDnsIp = "8.8.8.8";
        uint16_t kDnsPort = 53;
        struct sockaddr_in serv;
        memset(&serv, 0, sizeof (serv));
        serv.sin_family = AF_INET;
        serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
        serv.sin_port = htons(kDnsPort);

        int err = connect(sock, (const sockaddr*) &serv, sizeof (serv));
        if (err != -1) {

            sockaddr_in name;
            socklen_t namelen = sizeof (name);
            err = getsockname(sock, (sockaddr*) & name, &namelen);
            if (err != -1) {

                const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, buflen);
                assert(p);

                close(sock);
            }
        }
    }
    return std::string(buffer);
}

std::string get_fd_contents(int fd) {
    __gnu_cxx::stdio_filebuf<char> filebuf(fd, std::ios::in); // 1
    std::istream is(&filebuf); // 2
    std::string line;
    std::string para;
    while (!is.eof()) {
        getline(is, line);
        para += line;
    }
    return para;
}

char const * sperm(__mode_t mode) {
    static char local_buff[16] = {0};
    int i = 0;
    // user permissions
    if ((mode & S_IRUSR) == S_IRUSR) local_buff[i] = 'r';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IWUSR) == S_IWUSR) local_buff[i] = 'w';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IXUSR) == S_IXUSR) local_buff[i] = 'x';
    else local_buff[i] = '-';
    i++;
    // group permissions
    if ((mode & S_IRGRP) == S_IRGRP) local_buff[i] = 'r';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IWGRP) == S_IWGRP) local_buff[i] = 'w';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IXGRP) == S_IXGRP) local_buff[i] = 'x';
    else local_buff[i] = '-';
    i++;
    // other permissions
    if ((mode & S_IROTH) == S_IROTH) local_buff[i] = 'r';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IWOTH) == S_IWOTH) local_buff[i] = 'w';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IXOTH) == S_IXOTH) local_buff[i] = 'x';
    else local_buff[i] = '-';
    return local_buff;
}

FerryTimeStamp::FerryTimeStamp() {
    ferryTimesList.push_back(&this->t);
};

FerryTimeStamp::~FerryTimeStamp() {
    std::list<time_t*>::iterator i;
    i = ferryTimesList.begin();
    while (i != ferryTimesList.end()) {
        if (*i == &this->t) {
            ferryTimesList.erase(i);
            break;
        }
    }
};

std::list<time_t*> FerryTimeStamp::ferryTimesList;

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

char *base64_encode(const unsigned char *data,
        size_t input_length,
        size_t *output_length) {

    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = (char*) malloc(*output_length);
    if (encoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}

unsigned char *base64_decode(const char *data,
        size_t input_length,
        size_t *output_length) {

    if (decoding_table == NULL) build_decoding_table();

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = (unsigned char *) malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
                + (sextet_b << 2 * 6)
                + (sextet_c << 1 * 6)
                + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

void build_decoding_table() {

    decoding_table = (char*) malloc(256);

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}

void base64_cleanup() {
    free(decoding_table);
}