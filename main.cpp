/*
 * File:   main.cpp
 * Author: satya gowtham kudupudi
 *
 * Created on 15 March, 2013, 12:18 PM
 */

#include "Multimedia.h"
#include "MediaManager.h"
#include "myconverters.h"
#include "mystdlib.h"
#include "myxml.h"
#include "mycurl.h"
#include "debug.h"
#include "Socket.h"
#include "test-echo.c"
#include "libavcodec_util.h"
#include "capture.h"
#include "mypcm.h"
#include "logger.h"
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <getopt.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <time.h>
#include <netdb.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlstring.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/mman.h>
#include <dirent.h>
#include <vector>
#include <ostream>
#include <unistd.h>
#include <stdexcept>
#include <signal.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <algorithm>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
#include <pwd.h>
#include <grp.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>
#include <libudev.h>
#include <list>
#include <thread>
#include <map>

#define MAX_CAMS 10
#define APP_NAME "remotedevicecontroller"
using namespace std;

enum RecordState {
    RECORD_PREVIOUS_STATE, RECORD_STOP, RECORD_STREAM
};
string recordStateStr[] = {"RECORD_PREVIOUS_STATE", "RECORD_STOP", "RECORD_STREAM"};

enum camState {
    CAM_PREVIOUS_STATE = -1,
    CAM_NEW_STATE = -2,
    CAM_OFF = 0,
    CAM_RECORD = 1 << 0,
    CAM_STREAM = 1 << 1,
    CAM_STREAM_N_RECORD = 0b11
};
std::map<int, std::string> camStateString;

static void camStateStringInit() {
    camStateString[CAM_PREVIOUS_STATE] = "CAM_PREVIOUS_STATE";
    camStateString[CAM_NEW_STATE] = "CAM_NEW_STATE";
    camStateString[CAM_OFF] = "CAM_OFF";
    camStateString[CAM_RECORD] = "CAM_RECORD";
    camStateString[CAM_STREAM] = "CAM_STREAM";
    camStateString[CAM_STREAM_N_RECORD] = "CAM_STREAM_N_RECORD";
}

string serverAddr;
string serverPort;
string streamAddr;
string streamPort;
string appName;
string systemId;
Socket::SOCKET_TYPE socketType;
string trustedCA = "/etc/ssl/certs/ferryfair.cert";
string privatecert = "/etc/ssl/certs/" + string(APP_NAME) + ".ferryfair.cert";
string privatekey = "/etc/ssl/certs/" + string(APP_NAME) + ".ferryfair.key";
string xmlnamespace;
string securityKey;
string videoStreamingType;
string pollInterval;
string autoInsertCameras;
string configFile = "/etc/" + string(APP_NAME) + ".conf.xml";
string camFolder = "/dev/";
bool camcaptureCompression;
string recordResolution;
string streamResolution;
int recordWidth = 0;
int recordHeight = 0;
int streamWidth = 0;
int streamHeight = 0;
string recordfps;
string streamfps;
int rfps;
int sfps;
string mobileBroadbandCon;
string corpNWGW;
int manageNetwork = 0;
string gpsDevice;
string gpsSDeviceBaudrate;
int reconnectDuration;
int reconnectPollCount = 0;
int reconnectPollCountCopy;
string internetTestURL;
bool configModem;
string recordsFolder = "/var/" + string(APP_NAME) + "records/";
string logFile = "/var/log/" + string(APP_NAME) + ".log";
string initFile = "/etc/init/" + string(APP_NAME) + ".conf";
string initOverrideFile = "/etc/init/" + string(APP_NAME) + ".override";
string initdFile = "/etc/init.d/" + string(APP_NAME);
string installationFolder = "/usr/local/bin/";
string binFile = installationFolder + string(APP_NAME);
string rootBinLnk = "/usr/bin/" + string(APP_NAME);
string srcFolder = "/usr/local/src/" + string(APP_NAME);
string deviceRulesFile = "/etc/udev/rules.d/" + string(APP_NAME) + ".rules";
string runningProcessFile = "/var/tmp/" + string(APP_NAME) + ".pid";
string mobileModemVendorProductID = "";
string usbHubVendorProductID = "";
bool mobile_modem_disconnect_toggle = false;
time_t hub_last_reset_time = 0;
int volatile_usb_hub_reset_interval = 500;
int const_usb_hub_reset_interval = 500;
time_t gpsUpdatePeriod = 10;
bool pkilled = true;
int SOAPServiceReqCount = 0;
int ferr;
bool contiguousPoll = false;
bool updateTimeStamps = false;
vector<string> recordedFileNames;
bool CMOSWorking;
time_t timeGapToCorrectTime;
bool internetTimeUpdated = false;
bool enable_mic = false;

bool allCams = false;
string reqCam;
string reqSId;
bool newlyMarried = true;

camState ps = CAM_OFF;
camState cs = CAM_OFF;
time_t st;

pid_t rootProcess;
pid_t firstChild;
pid_t secondChild;
pid_t runningProcess;
string runMode = "normal";


pthread_t nwMgrThread;
string modemInode = "/dev/ttyUSB0";
string currentIP;

time_t currentTime;

time_t nm_presentCheckTime;
time_t nm_previousCheckTime;

string mobile_modem_bus_device_file_name;
string usb_hub_bus_device_file_name;

int ff_log_type = FFL_NOTICE | FFL_WARN | FFL_ERR | FFL_DEBUG;
unsigned int ff_log_level = FPOL_MAIN | FPOL_MM;

enum StreamType {
    FMSP,
    RTMP
} stream_type;

char stream_type_str[][10] = {
    "FFMP",
    "RTMP"
};

union family_message {
    int timestampdiff;
};

struct family_message_block {

    struct {

        enum {
            CORRECT_TIME_STAMP
        } type;

        union {
            int timeGapToCorrectTime;
        } data;
    } msg;
    pid_t msg_from;
    pid_t msg_to;
} * family_message_block_ptr;
int family_message_block_id;

void run();
void stop();
string readConfigValue(string name);
void instReInstComCode(string sk);
void stopRunningProcess();
void reinstallKey();
void* networkManager(void* arg);
void correctTimeStampFileNames();
void correctAllTimeVariables();
int writeRootProcess();
void set_bus_device_file_name(string vpid, string& bus_device_file_name);
void usb_reset(string device);

void cleanRecords(int days) {
    DIR * dpdf = opendir(recordsFolder.c_str());
    struct dirent * epdf;
    vector<string> ls;
    string fn;
    while (epdf = readdir(dpdf)) {
        fn = string(epdf->d_name);
        if (fn.compare(".") != 0 && fn.compare("..") != 0) {
            ls.push_back(fn);
        }
    }
    sort(ls.begin(), ls.end());
    while (days) {
        days--;
        if (rmmydir(recordsFolder + ls[days]) == 1) {
            cout << "\n" << getTime() << " cleanRecords removed " << ls[days];
        }
    }
}

void checkCMOSWorking() {
    CMOSWorking = CMOSWorking;
}

class camService {
public:
    pid_t pid;
    pthread_t* t; //*> thread pointer for media streaming code if its a function rather than a program E.g. Used by MediaManager::capture function but not by ffmpeg program
    MediaManager::capture_thread_iargs * cti; //*> input arguments pointer
    int stdio[2];
    string cam;
    camState state = CAM_OFF;
    camState newState = CAM_PREVIOUS_STATE;
    string SId;
    bool disable;
    string recordPath;
    string streamPath;
    ~camService();
};

camService::~camService() {
    if (t != NULL)pthread_cancel(*t);
    delete cti;
}

class GPSManager {
public:

    enum GPSProto {
        GPS_PROTO_UNKNOWN, GPS_PROTO_LOCAL, GPS_PROTO_NMEA0183
    };
    static const char* gpsProtoStr[]; // = {"GPS_PROTO_UNKNOWN", "GPS_PROTO_LOCAL", "GPS_PROTO_NMEA0183"};

    enum gps_type {
        RS232, BT
    };
    static time_t gpsReadStart;
    static time_t gpsReadEnd;
    static string gpsCoordinates;
    static pthread_t gpsUpdaterThread;
    static bool bt_respawn;
    static GPSProto gp;
    static gps_type gt;
    static int initConnectTrials;

    static void bye_rfspawn(spawn* rfspawn) {
        bt_respawn = true;
        if (initConnectTrials > 0) {
            ffl_warn(FPOL_GPS, "gpsManager: No bluetooth gps found; sleeping for 20 seconds");
            sleep(20);
            initConnectTrials--;
        } else {
            ffl_warn(FPOL_GPS, "gpsManager: No bluetooth gps found; sleeping for 120 seconds");
            sleep(120);
        }
    };

    static void parseNMEA0183(FILE* f, time_t& gpsReadStart, time_t& gpsReadEnd, std::string& gpsCoordinates) {
        string gpsphrase;
        bool gpsphrase_start;
        vector<string> GPRMCphrases;
        int c = getc(f);
        while (c != EOF) {
            if (c == '$') {
                time(&gpsReadStart);
                gpsphrase = "";
                gpsphrase_start = true;
            } else if (gpsphrase_start && c == '\n') {
                gpsphrase_start = false;
                time(&gpsReadEnd);
                if ((int) gpsphrase.find("GPRMC", 0) > 0) {
                    GPRMCphrases = explode(",", gpsphrase);
                    gpsCoordinates = GPRMCphrases[3].length() > 0 ? gpsDevice + ":" + "Latitude,Longitude:-" + GPRMCphrases[3] + GPRMCphrases[4] + "," + GPRMCphrases[5] + GPRMCphrases[6] : gpsDevice + ":" + "Latitude,Longitude:-0000.0000,0000.0000";
                    ffl_debug(FPOL_GPS, "gpsCoordinates: %s", gpsCoordinates.c_str());
                }
            }
            if (gpsphrase_start) {
                gpsphrase.append(1, (char) c);
            }
            c = getc(f);
        }
    };

    static void parseLocalGPSProtocol(FILE* f, time_t& gpsReadStart, time_t& gpsReadEnd, std::string& gpsCoordinates) {
        int c = getc(f);
        char buf[100];
        int i = 0;
        while (c != EOF) {
            if (c == '@') {
                time(&gpsReadStart);
                i = 0;
                c = getc(f);
                while (c != '#' && i < 100) {
                    if (c == EOF) {
                        break;
                    }
                    buf[i] = (char) c;
                    i++;
                    c = getc(f);
                }
                if (i < 100) {
                    time(&gpsReadEnd);
                    buf[i] = '\0';
                    gpsCoordinates = gpsDevice + ":" + string(buf, i);
                    ffl_debug(FPOL_GPS, "gpsCoordinates : %s", gpsCoordinates.c_str());
                } else {
                    ffl_err(FPOL_GPS, "gpsManager: illegal string from GPS device");
                }
            } else if (c == EOF) {
                break;
            }
            c = getc(f);
        }
    }

    static void* gpsLocationUpdater(void* arg) {
        char buf[48];
        int c;
        int i = 0;
        string cmd;
        int devno;
        sleep(10);
        if ((devno = (int) gpsDevice.find("rfcomm", 0)) > 0) {
            devno += 6;
            gt = BT;
            gp = GPS_PROTO_NMEA0183;
rerfspawn:
            string cmd = "rfcomm connect " + gpsDevice.substr(devno, gpsDevice.length());
            spawn * rfspawn = new spawn(cmd, true, &GPSManager::bye_rfspawn, false, false);
            ffl_notice(FPOL_GPS, "gpsManager: connecting to bluetooth gps device...");
            sleep(6);
        }
opendevice:
        FILE *f = fopen(gpsDevice.c_str(), "r");
        ffl_notice(FPOL_GPS, "reading gps device: %s", gpsDevice.c_str());
        if (gt == RS232 && f && gpsSDeviceBaudrate.length() > 0) {
            cmd = "stty -F " + gpsDevice + " " + gpsSDeviceBaudrate;
            spawn *gpsdbrsetter = new spawn(cmd, false, NULL, false, true);
            delete gpsdbrsetter;
            ffl_debug(FPOL_GPS, "baudrate set:cmd:%s", cmd.c_str());
        } else if (gt == BT && f == NULL) {
            sleep(30);
            if (bt_respawn) {
                bt_respawn = false;
                goto rerfspawn;
            }
            goto opendevice;
        } else if ((devno = (int) gpsDevice.find("/dev/ttyO1", 0)) >= 0) {
            sleep(30);
            //int slots = open("/sys/devices/bone_capemgr.9/slots", O_WRONLY);
            //char dto[] = "ttyO1_armhf.com";
            //write(slots, dto, 15);
            //close(slots);
            ffl_notice(FPOL_GPS, "gpsManager: enabling serial port ttyO1");
            DIR *dpdf;
            dirent *epdf;
            dpdf = opendir("/sys/devices");
            char search_string[] = "bone_capemgr";
            if (dpdf != NULL) {
                while (epdf = readdir(dpdf)) {
                    if (strstr(epdf->d_name, search_string) != NULL) {
                        string slots = string("/sys/devices/") + string(epdf->d_name) + string("/slots");
                        int slotsf = open((char*) slots.c_str(), O_WRONLY);
                        char dto[] = "ttyO1_armhf.com";
                        write(slotsf, dto, 15);
                        close(slotsf);
                    }
                }
            }
            //system("echo ttyO1_armhf.com > /sys/devices/bone_capemgr*/slots");
            goto opendevice;
        }
        if (f) {
            if (gt == BT && gp == GPS_PROTO_NMEA0183) {
                parseNMEA0183(f, gpsReadStart, gpsReadEnd, gpsCoordinates);
            } else {
                if (gp == GPS_PROTO_NMEA0183) {
                    parseNMEA0183(f, gpsReadStart, gpsReadEnd, gpsCoordinates);
                } else {
                    parseLocalGPSProtocol(f, gpsReadStart, gpsReadEnd, gpsCoordinates);
                }
                ffl_notice(FPOL_GPS, "gpsManager: gps device disconnected. sleeping for 10 secs.");
                sleep(10);
                goto opendevice;
            }
        }
        ffl_notice(FPOL_GPS, "gpsManager: no gps device found. sleeping for 60 secs.");
        sleep(60);
        goto opendevice;
    }

    static void init() {
        pthread_create(&gpsUpdaterThread, NULL, &gpsLocationUpdater, NULL);
    }
};

time_t GPSManager::gpsReadStart;
time_t GPSManager::gpsReadEnd;
string GPSManager::gpsCoordinates;
pthread_t GPSManager::gpsUpdaterThread;
bool GPSManager::bt_respawn;
GPSManager::GPSProto GPSManager::gp;
GPSManager::gps_type GPSManager::gt = GPSManager::RS232;
int GPSManager::initConnectTrials = 10;
const char* GPSManager::gpsProtoStr[] = {"GPS_PROTO_UNKNOWN", "GPS_PROTO_LOCAL", "GPS_PROTO_NMEA0183"};

class RecordsManager {
public:
    typedef int RecordIndex;

    class Record {
    public:
        string recordName[3];
        string recordFile;
        RecordState state;
        RecordState newState;
        string sId;
        pid_t spid;
        spawn recorder;

        class RecordFileNotFoundException {
        public:
            string notFoundFile;

            RecordFileNotFoundException(string f) {
                this->notFoundFile = f;
            }
        };

        Record(string* recordName) {
            struct stat s;
            string fL = recordsFolder + recordName[0] + "/" + recordName[1] + "/" + recordName[2];
            if (stat(fL.c_str(), &s) != -1) {
                this->recordName[0] = recordName[0];
                this->recordName[1] = recordName[1];
                this->recordName[2] = recordName[2];
                this->recordFile = fL;
                this->state = RECORD_STOP;
                this->newState = RECORD_PREVIOUS_STATE;
                this->spid = 0;
            } else {
                throw RecordFileNotFoundException(fL);
            }
        }
    };

private:
    static vector<Record> records;

    static void serviceStopHandler(pid_t pid) {

    }

public:

    static int setNewState(RecordIndex ri, RecordState rs) {
        try {
            if (records[ri].state != rs) {
                records[ri].newState = rs;
                return 1;
            } else {
                records[ri].newState = RECORD_PREVIOUS_STATE;
                return 0;
            }
        } catch (std::out_of_range e) {
            return -1;
        }
    }

    static int setSId(RecordIndex ri, string sid) {
        try {
            if (records[ri].sId.compare(sid) != 0) {
                records[ri].sId = string(sid);
                return 1;
            } else {
                return 0;
            }
        } catch (std::out_of_range e) {
            return -1;
        }
    }

    static RecordIndex addRecord(string * recordName) {
        if (getRecordIndex(recordName) == -1) {
            try {
                Record r = Record(recordName);
                RecordsManager::records.push_back(r);
                return RecordsManager::records.size() - 1;
            } catch (Record::RecordFileNotFoundException e) {
                return -1;
            }
        }
        return 0;
    }

    static Record* getRecord(string * recordName) {
        int i = 0;
        Record* sr;
        while (i < (int) RecordsManager::records.size()) {
            sr = &RecordsManager::records.at(i);
            if (sr->recordName[0].compare(recordName[0]) == 0) {
                if (sr->recordName[1].compare(recordName[1]) == 0) {
                    if (sr->recordName[2].compare(recordName[2]) == 0) {
                        return sr;
                    }
                }
            }
            i++;
        }
        return NULL;
    }

    static Record* getRecordByPID(pid_t pid) {
        int i = 0;
        Record* sr;
        while (i < records.size()) {
            sr = &records.at(i);
            if (sr->spid == pid) {
                return sr;
            }
            i++;
        }
        return NULL;
    }

    static int getRecordIndex(string* recordName) {
        int i = 0;
        Record* sr;
        while (i < (int) RecordsManager::records.size()) {
            sr = &RecordsManager::records.at(i);
            if (sr->recordName[0].compare(recordName[0]) == 0) {
                if (sr->recordName[1].compare(recordName[1]) == 0) {
                    if (sr->recordName[2].compare(recordName[2]) == 0) {
                        return i;
                    }
                }
            }
            i++;
        }
        return -1;
    }

    static int setRecordState(int rIndex) {
        struct stat rStat;
        char procF[20] = "/proc/";
        string sa = "rtmp://" + streamAddr + ":" + streamPort + "/oflaDemo/RecordedFiles" + records[rIndex].sId;
        string rfa = records[rIndex].recordFile;
        if (records[rIndex].newState == RECORD_STREAM) {
            stopRecord(rIndex);
            if (stream_type == FMSP) {
            } else {
                string cmd = "ffmpeg -re -i " + rfa + " -r " + streamfps + " -s " + streamResolution + " -f flv " + sa;
                records[rIndex].recorder = spawn(cmd, true, NULL, false);
                ffl_debug(FPOL_MAIN, "%s : %d", cmd.c_str(), records[rIndex].recorder.cpid);
                records[rIndex].spid = records[rIndex].recorder.cpid;
            }
            records[rIndex].newState = RECORD_PREVIOUS_STATE;
            records[rIndex].state = RECORD_STREAM;
        } else if (records[rIndex].newState == RECORD_STOP) {
            stopRecord(rIndex);
            records[rIndex].newState = RECORD_PREVIOUS_STATE;
        }
        if (records[rIndex].spid == 0)return -1;
        return 1;
    }

    static int stopRecord(RecordIndex ri) {
        int i = 0;
        records[ri].state = RECORD_STOP;
        string proc = "/proc/" + string(itoa(records[ri].spid));
        struct stat ptr;
        if (stat(proc.c_str(), &ptr) != -1) {
            pkilled = false;
            i = kill(records[ri].spid, SIGTERM);
            while (!pkilled);
        }
        return i;
    }
};
vector<RecordsManager::Record> RecordsManager::records;

void ffmpegOnStopHandler(spawn* process) {
    char ffmpegerr[100];
    ffmpegerr[0] = '\0';
    ffmpegerr[100] = '\0';
    read(process->cpstderr, ffmpegerr, 99);
    string ffmpegerrstr = string(ffmpegerr);
    ffl_debug(FPOL_MAIN, "ffmpeg->pid=%d, ffmpeg->exitcode=%d,"
            "ffmpegerr->error=%s", (int) process->cpid,
            process->getChildExitStatus(), ffmpegerr);
    if ((int) ffmpegerrstr.find("No space left on device", 0) > 0) {
        cleanRecords(1);
    } else if ((int) ffmpegerrstr.find("Device or resource busy", 0) > 0) {
        system("pkill ffmpeg");
    } else if (!CMOSWorking&&!updateTimeStamps && internetTimeUpdated) {
        correctTimeStampFileNames();
        updateTimeStamps = true;
    }
    /*resets hub only when recording breaks*/
    if (mobile_modem_disconnect_toggle) {
        set_bus_device_file_name(usbHubVendorProductID,
                usb_hub_bus_device_file_name);
        usb_reset(usb_hub_bus_device_file_name);
        mobile_modem_disconnect_toggle = false;
    }
    delete process;
}

void correctTimeStampFileNames() {
    string fileName;
    int lastslashpos;
    string fold;
    string camName;
    string timestamp;
    struct tm tms;
    time_t t;
    string path;
    string fname;
    ffl_debug(FPOL_MAIN, "correcting timestamps ...");
    while (recordedFileNames.size() != 0) {
        fileName = recordedFileNames[recordedFileNames.size() - 1];
        recordedFileNames.pop_back();
        lastslashpos = fileName.rfind('/', fileName.length());
        fold = fileName.substr(0, lastslashpos);
        camName = fold.substr(fold.rfind('/', fileName.length()) + 1);
        timestamp = fileName.substr(lastslashpos + 1);
        if (stream_type == FMSP) {
            strptime(timestamp.c_str(), "%Y-%m-%d_%H:%M:%S.fp", &tms);
        } else if (stream_type == RTMP) {
            strptime(timestamp.c_str(), "%Y-%m-%d_%H:%M:%S.flv", &tms);
        }
        t = mktime(&tms);
        t += timeGapToCorrectTime;
        struct tm * timeinfo;
        char fn [80];
        char dn [80];
        timeinfo = localtime(&t);
        if (stream_type == FMSP) {
            strftime(fn, 80, "%Y-%m-%d_%H:%M:%S.fp", timeinfo);
        } else if (stream_type == RTMP) {
            strftime(fn, 80, "%Y-%m-%d_%H:%M:%S.flv", timeinfo);
        }
        strftime(dn, 80, "%Y-%m-%d", timeinfo);
        string path = "/var/" + string(APP_NAME) + "records/" + string(dn) + "/";
        struct stat st = {0};
        if (stat(path.c_str(), &st) == -1) {
            mkdir(path.c_str(), 0774);
        }
        path += camName + "/";
        fname = path + "/" + fn;
        copyfile(fileName, fname);
        unlink((char*) fileName.c_str());
        ffl_debug(FPOL_MAIN, "renamed %s to %s", fileName.c_str(), fname.c_str());
    }
}

class csList {
private:
    static int s;
    static std::map<std::string, camService> csl;

public:

    class Exception {
    public:
        Exception(string msg);
        ~Exception();
        string what();
    private:
        string msg;
    };

    csList() {
        s = 0;
    }

    static void initialize(int ss) {
        s = ss;
    }

    static void addCamService(string cam) {
        int i = 0;
        bool camAdded = false;
        std::map<string, camService>::iterator csit = csl.find(cam);
        if (csit != csl.end()) {
            camAdded = true;
        }
        if (!camAdded) {
            csl[cam].cam = cam;
            csit = csl.find(cam);
            if (stream_type == FMSP) {
                csit->second.t = new pthread_t();
                MediaManager::capture_thread_iargs* args = new MediaManager::capture_thread_iargs();
                csit->second.cti = args;
                args->inputs = new std::valarray<MediaManager::media>(enable_mic ? 2 : 1);
                (*args->inputs)[0].duration = 0;
                (*args->inputs)[0].encoding = MediaManager::MJPEG;
                (*args->inputs)[0].height = recordHeight > streamHeight ? recordHeight : streamHeight;
                (*args->inputs)[0].identifier = camFolder + cam;
                (*args->inputs)[0].type = MediaManager::VIDEO;
                (*args->inputs)[0].videoframerate = sfps > rfps ? sfps : rfps;
                (*args->inputs)[0].width = recordWidth > streamWidth ? recordWidth : streamWidth;
                (*args->inputs)[0].state = 0;
                if (enable_mic) {
                    (*args->inputs)[1].audioSamplingFrequency = 44100;
                    (*args->inputs)[1].duration = 0;
                    (*args->inputs)[1].identifier = "plughw:1";
                    (*args->inputs)[1].type = MediaManager::AUDIO;
                    (*args->inputs)[1].state = 0;
                }
                args->outputs = new std::valarray<MediaManager::media>(2);
                //(*args->outputs)[0].identifier = "fmsp://fms.newmeksolutions.com:92711/" + appName + "1780";
                //    (*args->output)[0].identifier = "ferrymediacapture1/";
                (*args->outputs)[0].identifier = csit->second.recordPath;
                (*args->outputs)[0].segmentDuration = 1;
                (*args->outputs)[0].videoframerate = rfps;
                (*args->outputs)[0].audioBitrate = 64000;
                (*args->outputs)[0].duration = 0;
                (*args->outputs)[0].encoding = MediaManager::MP3;
                (*args->outputs)[0].splMediaProps.fPRecorderExclProps.perFileDurationSec = 15 * 60;
                (*args->outputs)[0].state = 0;
                (*args->outputs)[0].signalNewState = 0;
                (*args->outputs)[1].identifier = csit->second.streamPath;
                (*args->outputs)[1].segmentDuration = 1;
                (*args->outputs)[1].videoframerate = sfps;
                (*args->outputs)[1].audioBitrate = 64000;
                (*args->outputs)[1].duration = 0;
                (*args->outputs)[1].encoding = MediaManager::MP3;
                (*args->outputs)[1].splMediaProps.fmpFeederSplProps.reconnect = true;
                (*args->outputs)[1].splMediaProps.fmpFeederSplProps.reconnectIntervalSec = 10;
                (*args->outputs)[1].state = 0;
                (*args->outputs)[1].signalNewState = 0;
                pthread_create(csit->second.t, NULL, &MediaManager::capture, (void*) (args));
            }
            csit->second.state = CAM_OFF;
            csit->second.newState = CAM_RECORD;
        }
    }

    /**
     * removeCamService removes camService corresponding to @param cam from csList, frees all the resources accompanied with it and rearrange csList
     * @param cam
     */
    static std::map<string, camService>::iterator removeCamService(string cam) {
        std::map<std::string, camService>::iterator csit = csl.find(cam);
        ffl_debug(FPOL_MAIN | NO_NEW_LINE, "removing a cam service...");
        if (csit != csl.end()) {
            if (stream_type == FMSP) {
                FerryTimeStamp fts;
                clock_gettime(CLOCK_REALTIME, &fts.ts);
                fts.ts.tv_sec += 2;
                (*csit->second.cti->outputs)[0].signalNewState = -1;
                pthread_timedjoin_np(*csit->second.t, NULL, &fts.ts);
            }
            csit = csl.erase(csit);
            ffl_debug_contnu(FPOL_MAIN, "OK");
        } else {
            ffl_debug_contnu(FPOL_MAIN, "FAIL");
        }
        return csit;
    }

    static int getCamCount() {
        return csl.size();
    }

    static bool camReattached() {
        std::map<string, camService>::iterator csit = csl.begin();
        string procF = "/proc/";
        string proc;
        struct stat ptr;
        while (csit != csl.end()) {
            if (!csit->second.disable) {
                if (stream_type == FMSP) {
                    if (stat((*csit->second.cti->inputs)[0].identifier.c_str(), &ptr) != -1) {
                        return (*csit->second.cti->inputs)[0].state == -1;
                    }
                } else if (stream_type == RTMP) {
                    proc = procF + string(itoa(csit->second.pid));
                    if (stat(proc.c_str(), &ptr) == -1) {
                        return true;
                    }
                }
            }
            csit++;
        }
        return false;
    }

    static pid_t setCamState(string cam) {
        int csIndex;
        camService& cs = csl[cam];
        if (!cs.disable) {
            string dev = camFolder + cam;
            pid_t fcpid = cs.pid;
            pthread_t* t = cs.t;
            camState ns = cs.state;
            spawn *process;
            string cmd;
            MediaManager::capture_thread_iargs* args = cs.cti;
            if (cs.newState == CAM_RECORD) {
                if (stream_type == FMSP) {
                    (*args->outputs)[0].signalNewState = 1;
                    (*args->outputs)[1].signalNewState = 0;
                } else {
                    csList::stopCam(cam);
                    cmd = "ffmpeg -loglevel error -f video4linux2 " + (camcaptureCompression ? string("-vcodec mjpeg ") : string("")) + "-r " + recordfps + " -s " + recordResolution + " -i " + dev + " " + cs.recordPath;
                    process = new spawn(cmd, true, &ffmpegOnStopHandler, false);
                    ffl_debug(FPOL_MAIN, "%s :%d", cmd.c_str(), process->cpid);
                    fcpid = process->cpid;
                }
                ns = CAM_RECORD;
                recordedFileNames.push_back(cs.recordPath);
            } else if (cs.newState == CAM_STREAM) {
                if (stream_type == FMSP) {
                    (*args->outputs)[0].signalNewState = 0;
                    if ((*args->outputs)[1].identifier.compare(cs.streamPath) != 0) {
                        (*args->outputs)[1].identifier = cs.streamPath;
                        (*args->outputs)[1].signalNewState = 1;
                    } else {
                        (*args->outputs)[1].signalNewState = 2;
                    }
                } else if (stream_type = RTMP) {
                    cmd = "ffmpeg -loglevel error -f video4linux2 " + (camcaptureCompression ? string("-vcodec mjpeg ") : string("")) + "-r " + recordfps + " -s " + recordResolution + " -i " + dev + " -r " + streamfps + " -s " + streamResolution + " -f flv " + cs.streamPath;
                    csList::stopCam(cam);
                    process = new spawn(cmd, true, &ffmpegOnStopHandler, false);
                    ffl_debug(FPOL_MAIN, "%s :%d", cmd.c_str(), process->cpid);
                    fcpid = process->cpid;
                }
                ns = CAM_STREAM;
            } else if (cs.newState == CAM_STREAM_N_RECORD) {
                if (stream_type == FMSP) {
                    (*args->outputs)[0].signalNewState = 1;
                    if ((*args->outputs)[1].identifier.compare(cs.streamPath) != 0) {
                        (*args->outputs)[1].identifier = cs.streamPath;
                        (*args->outputs)[1].signalNewState = 1;
                    } else {
                        (*args->outputs)[1].signalNewState = 2;
                    }
                } else if (stream_type == RTMP) {
                    cmd = "ffmpeg -loglevel error -f video4linux2 " + (camcaptureCompression ? string("-vcodec mjpeg ") : string("")) + "-r " + recordfps + " -s " + recordResolution + " -i " + dev + " -r " + streamfps + " -s " + streamResolution + " -f flv " + cs.streamPath + " " + cs.recordPath;
                    csList::stopCam(cam);
                    process = new spawn(cmd, true, &ffmpegOnStopHandler, false);
                    ffl_debug(FPOL_MAIN, "%s :%d", cmd.c_str(), process->cpid);
                    fcpid = process->cpid;
                }
                ns = CAM_STREAM_N_RECORD;
                if (!CMOSWorking&&!updateTimeStamps) {
                    recordedFileNames.push_back(cs.recordPath);
                }
            } else if (cs.newState == CAM_OFF) {
                if (stream_type == FMSP) {
                    (*args->outputs)[0].signalNewState = -1;
                    (*args->outputs)[1].signalNewState = -1;
                } else if (stream_type == RTMP) {
                    kill(cs.pid, SIGTERM);
                }
            }
            cs.pid = fcpid;
            cs.state = ns;
            cs.newState = CAM_PREVIOUS_STATE;
            return fcpid;
        } else {
            return -1;
        }
    }

    static int stopCam(string cam) {
        int i = 0;
        int csIndex;
        camService& cs = csl[cam];
        if (stream_type == RTMP) {
            string proc = "/proc/" + string(itoa(cs.pid));
            struct stat ptr;
            if (stat(proc.c_str(), &ptr) != -1) {
                pkilled = false;
                i = kill(cs.pid, SIGKILL);
                while (!pkilled);
            }
        } else if (stream_type == FMSP) {
            pthread_cancel(*cs.t);
            delete cs.t;
        }
        cs.state = CAM_OFF;
        return i;
    }

    static bool isCamDisabled(string cam) {
        if (csl.find(cam) != csl.end()) {
            return csl[cam].disable;
        } else {
            throw Exception(cam + " not in the cam service list");
        }
        return false;
    }

    static camState getCamNewState(string cam) {
        if (csl.find(cam) != csl.end()) {
            return csl[cam].newState;
        } else {
            throw Exception(cam + " not in the cam service list");
        }
    }

    /**
     * setCams method take in array of cams (from getCams method), remove non functional cams from csList::csl, and addCamService for all the given cams.
     * @param cams array video devices in /dev folder
     */
    static void setCams(string * cams) {
        int i = 0;
        int j = 0;
        bool camFound = false;
        string procf = "/proc/";
        string proc;
        struct stat ptr;
        if (csl.size() > 0) {
            std::map<string, camService>::iterator csit = csl.begin();
            if (stream_type == FMSP) {
                while (csit != csl.end()) {
                    if ((*csit->second.cti->inputs)[0].state == -1) {
                        csit = csList::removeCamService(csit->second.cam);
                    } else {
                        csit++;
                    }
                }
            } else if (stream_type == RTMP) {
                while (csit != csl.end()) {
                    proc = procf + string(itoa(csit->second.pid));
                    if (stat(proc.c_str(), &ptr) == -1) {
                        csit = csList::removeCamService(csit->second.cam);
                    } else {
                        csit++;
                    }
                }
            }
        }
        i = 0;
        while (cams[i].length() > 0) {
            csList::addCamService(string(cams[i]));
            i++;
        }
    }

    static int setNewCamState(string cam, camState ns) {
        if (csl.find(cam) != csl.end()) {
            if (csl[cam].state != ns) {
                csl[cam].newState = ns;
                return 1;
            } else {
                return 0;
            }
        }
        return -1;
    }

    static void setRecordPath(string cam, string recordPath) {
        if (csl.find(cam) != csl.end()) {
            csl[cam].recordPath = recordPath;
            if (stream_type == FMSP) {
                (*csl[cam].cti->outputs)[0].identifier = recordPath;
            }
        }
    }

    static void setStreamPath(string cam, string streamPath) {
        if (csl.find(cam) != csl.end()) {
            csl[cam].streamPath = streamPath;
            if (stream_type == FMSP) {
                (*csl[cam].cti->outputs)[1].identifier = streamPath;
            }
        }
    }

    static int setStateAllCams(camState state) {
        std::map<string, camService>::iterator csit = csl.begin();
        while (csit != csl.end()) {
            if (csit->second.state != state) {
                csit->second.newState = state;
            }
            csit++;
        }
        return 0;
    }

    static int resetStateAllCams(camState state) {
        map<string, camService>::iterator csit = csl.begin();
        while (csit != csl.end()) {
            csit->second.newState = state;
            csit++;
        }
        return 0;
    }

    static void getCams(string * cams) {
        map<string, camService>::iterator csit = csl.begin();
        int i = 0;
        while (csit != csl.end()) {
            cams[i] = string(csit->second.cam);
            csit++;
            i++;
        }
    }

    static string getSId(string cam) {
        if (csl.find(cam) != csl.end()) {
            return csl[cam].SId;
        } else {
            throw Exception(cam + " not in the cam service list");
        }
    }

    static int setSId(string cam, string sid) {
        if (csl.find(cam) != csl.end()) {
            csl[cam].SId = sid;
        }
        return -1;
    }

    static string getCamsWithStateString() {
        string strCameras = "";
        map<string, camService>::iterator csit = csl.begin();
        while (csit != csl.end()) {
            map<string, camService>::iterator csit2 = csit;
            csit2++;
            strCameras += csit->second.cam + ":" + camStateString[csit->second.state] + ((csit2 == csl.end()) ? "" : "^");
            csit++;
        }
        return strCameras;
    }

};
int csList::s = 0;
map<string, camService> csList::csl;

csList::Exception::Exception(string msg) {
    this->msg = msg;
}

csList::Exception::~Exception() {

}

string csList::Exception::what() {
    return this->msg;
}

void uninstall() {
    stopRunningProcess();
    cout << "\nDeleting " + initFile;
    unlink(initFile.c_str());
    cout << "\nDeleting " + initOverrideFile;
    unlink(initOverrideFile.c_str());
    cout << "\nDeleting " + initdFile;
    unlink(initdFile.c_str());
    cout << "\nDeleting  " + configFile;
    unlink(deviceRulesFile.c_str());
    cout << "\nDeleting  " + deviceRulesFile;
    unlink(configFile.c_str());
    cout << "\nDeleting " + binFile;
    unlink(binFile.c_str());
    cout << "\nDeleting " + logFile;
    unlink(logFile.c_str());
    cout << "\nDeleting " + rootBinLnk;
    unlink(trustedCA.c_str());
    cout << "\nDeleting " + trustedCA;
    unlink(privatecert.c_str());
    cout << "\nDeleting " + privatecert;
    unlink(privatekey.c_str());
    cout << "\nDeleting " + privatekey;
    unlink(rootBinLnk.c_str());
    string input;
decision:
    cout << "\nDo you want to remove recorded files?(yes/no):";
    input = inputText();
    if (input.compare("yes") == 0) {
        rmmydir(recordsFolder);
    } else if (input.compare("no") == 0) {
        cout << "\nPlease find the records at " + recordsFolder;
    } else {
        goto decision;
    }
    cout << "\n" + string(APP_NAME) + " uninstalled successfully :D\n";
}

string generateSecurityKey() {
    return systemId;
}

void readConfig() {
    xmlInitParser();
    struct stat st;
    xmlDoc * xd;
    if (stat(configFile.c_str(), &st) != -1) {
        xd = xmlParseFile(configFile.c_str());
    } else {
        xd = xmlParseFile("config.xml");
    }
    if (xd == NULL) {
        cout << "\n" << getTime() << " " << "readConfig : config file not found!\n";
        return;
    }
    xmlXPathContext *xc = xmlXPathNewContext(xd);
    xmlXPathObject* xo = xmlXPathEvalExpression((xmlChar*) "/config/server-addr", xc);
    xmlNode* node;
    xo = xmlXPathEvalExpression((xmlChar*) "/config/server-addr", xc);
    node = xo->nodesetval->nodeTab[0];
    serverAddr = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/server-port", xc);
    node = xo->nodesetval->nodeTab[0];
    serverPort = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/stream-addr", xc);
    node = xo->nodesetval->nodeTab[0];
    streamAddr = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/stream-port", xc);
    node = xo->nodesetval->nodeTab[0];
    streamPort = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/connection-encryption", xc);
    node = xo->nodesetval->nodeTab[0];
    string et = string((char*) xmlNodeGetContent(node));
    socketType = (et.compare("NO") == 0) ? Socket::DEFAULT : ((et.compare("TLS1_1") == 0) ? Socket::TLS1_1 : Socket::DEFAULT);
    xo = xmlXPathEvalExpression((xmlChar*) "/config/app-name", xc);
    node = xo->nodesetval->nodeTab[0];
    appName = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/namespace", xc);
    node = xo->nodesetval->nodeTab[0];
    xmlnamespace = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/debug", xc);
    node = xo->nodesetval->nodeTab[0];
    debug = atoi((char*) xmlNodeGetContent(node));
    ff_log_level = debug;
    xo = xmlXPathEvalExpression((xmlChar*) "/config/camcapture-compression", xc);
    node = xo->nodesetval->nodeTab[0];
    camcaptureCompression = (string((char*) xmlNodeGetContent(node)).compare("true") == 0);
    xo = xmlXPathEvalExpression((xmlChar*) "/config/record-fps", xc);
    node = xo->nodesetval->nodeTab[0];
    recordfps = string((char*) xmlNodeGetContent(node));
    rfps = atoi(recordfps.c_str());
    xo = xmlXPathEvalExpression((xmlChar*) "/config/record-resolution", xc);
    node = xo->nodesetval->nodeTab[0];
    recordResolution = string((char*) xmlNodeGetContent(node));
    int p = recordResolution.find("x");
    recordWidth = atoi(recordResolution.substr(0, p).c_str());
    recordHeight = atoi(recordResolution.substr(p + 1).c_str());
    xo = xmlXPathEvalExpression((xmlChar*) "/config/stream-fps", xc);
    node = xo->nodesetval->nodeTab[0];
    streamfps = string((char*) xmlNodeGetContent(node));
    sfps = atoi(streamfps.c_str());
    xo = xmlXPathEvalExpression((xmlChar*) "/config/stream-resolution", xc);
    node = xo->nodesetval->nodeTab[0];
    streamResolution = string((char*) xmlNodeGetContent(node));
    p = streamResolution.find("x");
    streamWidth = atoi(streamResolution.substr(0, p).c_str());
    streamHeight = atoi(streamResolution.substr(p + 1).c_str());
    xo = xmlXPathEvalExpression((xmlChar*) "/config/manage-network", xc);
    node = xo->nodesetval->nodeTab[0];
    manageNetwork = atoi((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/reconnect-duration", xc);
    node = xo->nodesetval->nodeTab[0];
    reconnectDuration = atoi((char*) xmlNodeGetContent(node))*60;
    xo = xmlXPathEvalExpression((xmlChar*) "/config/reconnect-poll-count", xc);
    node = xo->nodesetval->nodeTab[0];
    reconnectPollCount = atoi((char*) xmlNodeGetContent(node));
    reconnectPollCountCopy = reconnectPollCount;
    xo = xmlXPathEvalExpression((xmlChar*) "/config/mobile-broadband-connection", xc);
    node = xo->nodesetval->nodeTab[0];
    mobileBroadbandCon = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/mobile-modem-vendor-product-id", xc);
    node = xo->nodesetval->nodeTab[0];
    mobileModemVendorProductID = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/usb-hub-vendor-product-id", xc);
    node = xo->nodesetval->nodeTab[0];
    usbHubVendorProductID = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/config-modem", xc);
    node = xo->nodesetval->nodeTab[0];
    configModem = (string((char*) xmlNodeGetContent(node)).compare("true") == 0);
    xo = xmlXPathEvalExpression((xmlChar*) "/config/internet-test-url", xc);
    node = xo->nodesetval->nodeTab[0];
    internetTestURL = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/corporate-network-gateway", xc);
    node = xo->nodesetval->nodeTab[0];
    corpNWGW = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/gps-device", xc);
    node = xo->nodesetval->nodeTab[0];
    gpsDevice = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/gps-sdevice-baudrate", xc);
    node = xo->nodesetval->nodeTab[0];
    gpsSDeviceBaudrate = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/gps-protocol", xc);
    node = xo->nodesetval->nodeTab[0];
    string gp = string((char*) xmlNodeGetContent(node));
    GPSManager::gp = gp.compare("GPS_PROTO_LOCAL") == 0 ? GPSManager::GPSProto::GPS_PROTO_LOCAL : (gp.compare("GPS_PROTO_NMEA0183") == 0 ? GPSManager::GPSProto::GPS_PROTO_NMEA0183 : GPSManager::GPSProto::GPS_PROTO_UNKNOWN);
    xo = xmlXPathEvalExpression((xmlChar*) "/config/cmos-batt", xc);
    node = xo->nodesetval->nodeTab[0];
    CMOSWorking = string((char*) xmlNodeGetContent(node)).compare("true") == 0 ? true : false;
    xo = xmlXPathEvalExpression((xmlChar*) "/config/system-id", xc);
    if (xo->nodesetval->nodeNr > 0) {
        node = xo->nodesetval->nodeTab[0];
        systemId = string((char*) xmlNodeGetContent(node));
        xo = xmlXPathEvalExpression((xmlChar*) "/config/videoStreamingType", xc);
        node = xo->nodesetval->nodeTab[0];
        videoStreamingType = string((char*) xmlNodeGetContent(node));
        if (videoStreamingType.compare("FMSP") == 0) {
            stream_type = FMSP;
        } else if (videoStreamingType.compare("RTMP") == 0) {
            stream_type = RTMP;
        }
        xo = xmlXPathEvalExpression((xmlChar*) "/config/enable-mic", xc);
        node = xo->nodesetval->nodeTab[0];
        enable_mic = (string((char*) xmlNodeGetContent(node)).compare("true") == 0);
        xo = xmlXPathEvalExpression((xmlChar*) "/config/pollInterval", xc);
        node = xo->nodesetval->nodeTab[0];
        pollInterval = string((char*) xmlNodeGetContent(node));
        xo = xmlXPathEvalExpression((xmlChar*) "/config/autoInsertCameras", xc);
        node = xo->nodesetval->nodeTab[0];
        autoInsertCameras = string((char*) xmlNodeGetContent(node));
    }
    securityKey = generateSecurityKey();
    xmlCleanupParser();
}

void writeConfigValue(string name, string value) {
    xmlInitParser();
    xmlDoc * xd;
    struct stat st;
    if (stat(configFile.c_str(), &st) != -1) {
        xd = xmlParseFile(configFile.c_str());
    } else {
        xd = xmlParseFile("config.xml");
    }
    xmlXPathContext* xc = xmlXPathNewContext(xd);
    xmlXPathObject* rxo = xmlXPathEvalExpression((xmlChar*) "/config", xc);
    xmlNode* rn;
    rn = rxo->nodesetval->nodeTab[0];
    string xpath = "/config/" + name;
    xmlXPathObject *xo = xmlXPathEvalExpression((xmlChar*) xpath.c_str(), xc);
    int nn = xo->nodesetval->nodeNr;
    if (nn > 0) {
        xmlNode* node = xo->nodesetval->nodeTab[0];
        xmlNodeSetContent(node, (xmlChar*) value.c_str());
    } else {
        xmlNewTextChild(rn, NULL, (xmlChar*) name.c_str(), (xmlChar*) value.c_str());
    }
    xmlChar* s;
    int size;
    xmlDocDumpMemory(xd, &s, &size);
    FILE *fp = fopen(configFile.c_str(), "w");
    fwrite((char*) s, 1, size, fp);
    fclose(fp);
}

string readConfigValue(string name) {
    xmlInitParser();
    xmlDoc * xd;
    struct stat st;
    if (stat(configFile.c_str(), &st) != -1) {
        xd = xmlParseFile(configFile.c_str());
    } else {
        xd = xmlParseFile("config.xml");
    }
    xmlXPathContext* xc = xmlXPathNewContext(xd);
    string xpath = "/config/" + name;
    xmlXPathObject* xo = xmlXPathEvalExpression((xmlChar*) xpath.c_str(), xc);
    int nn = xo->nodesetval->nodeNr;
    if (nn > 0) {
        xmlNode* node = xo->nodesetval->nodeTab[0];
        return string((char*) xmlNodeGetContent(node));
    } else {
        return "";
    }
    xmlCleanupParser();
}

/**
 * This function scans for video devices in /dev directory and passes array of devices to setCams function
 */
void getCameras() {
    DIR *dpdf;
    struct dirent *epdf;
    dpdf = opendir("/dev/");
    string fn;
    int i = 0;
    i = 0;
    string cams[MAX_CAMS];
    while (epdf = readdir(dpdf)) {
        fn = string(epdf->d_name);
        if ((int) fn.find("video", 0) == 0) {
            cams[i] = fn;
            i++;
        }
    }
    closedir(dpdf);
    csList::setCams(cams);
}

/**
 * setState method sets the state of cams i.e. feeds all the active cam services
 * with streamPath and recordPath. It is called from run at regular interval if
 * there is change in state of camera. It is also called from system state
 * change when ever internet connection is lost to set state of all cams to
 * CAM_RECORD.
 */
void setState() {
    time_t rawtime;
    struct tm * timeinfo;
    char fn [80];
    char dn [80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (stream_type == FMSP) {
        strftime(fn, 80, "%Y-%m-%d_%H:%M:%S.fp", timeinfo);
    } else if (stream_type == RTMP) {
        strftime(fn, 80, "%Y-%m-%d_%H:%M:%S.flv", timeinfo);
    }
    strftime(dn, 80, "%Y-%m-%d", timeinfo);
    string path = "/var/" + string(APP_NAME) + "records/" + string(dn) + "/";
    struct stat st = {0};
    if (stat(path.c_str(), &st) == -1) {
        mkdir(path.c_str(), 0774);
    }
    struct stat fileAtt;
    int i = 0;
    string vd[MAX_CAMS];
    csList::getCams(vd);
    string fold;
    string dev;
    string fname;
    string sa;
    i = 0;
    int k = 0;
    while (vd[i].length() != 0) {
        k = 0;
        if (!csList::isCamDisabled(vd[i]) && csList::getCamNewState(vd[i]) != CAM_PREVIOUS_STATE) {
            fold = path + vd[i];
            if (stat(fold.c_str(), &fileAtt) == -1) {
                mkdir(fold.c_str(), 0774);
            }
            dev = "/dev/" + vd[i];
            fname = fold + "/" + fn;
            if (stream_type == FMSP) {
                sa = "fmsp://" + streamAddr + ":" + streamPort + "/" + csList::getSId(vd[i]);
            } else if (stream_type == RTMP) {
                sa = "rtmp://" + streamAddr + ":" + streamPort + "/oflaDemo/" + csList::getSId(vd[i]);
            }
            csList::setRecordPath(vd[i], fname);
            csList::setStreamPath(vd[i], sa);
            csList::setCamState(vd[i]);
        }
        i++;
    }
    allCams = false;
}

string getStrRecordedFiles() {
    DIR *dpdf;
    DIR *dateFolder;
    DIR *vidFolder;
    struct dirent *epdf;
    struct dirent *dfd;
    struct dirent *vfd;
    dpdf = opendir(recordsFolder.c_str());
    string fn;
    string dn;
    string vn;
    int i = 0;
    i = 0;
    string rfs = "";
    vector<string> sf;
    vector<string> vrfs;
    while (epdf = readdir(dpdf)) {
        fn = string(epdf->d_name);
        if (fn.compare(".") != 0 && fn.compare("..") != 0) {
            string afn = recordsFolder + fn + "/";
            if ((dateFolder = opendir(afn.c_str())) != NULL) {
                while (dfd = readdir(dateFolder)) {
                    dn = string(dfd->d_name);
                    if (dn.compare(".") != 0 && dn.compare("..") != 0) {
                        string adn = afn + dn + "/";
                        if ((vidFolder = opendir(adn.c_str())) != NULL) {
                            while (vfd = readdir(vidFolder)) {
                                vn = string(vfd->d_name);
                                if (vn.compare(".") != 0 && vn.compare("..") != 0) {
                                    sf.clear();
                                    sf.push_back(fn);
                                    sf.push_back(dn);
                                    sf.push_back(vn);
                                    vrfs.push_back(implode("%", sf));
                                }
                            }
                        }
                    }
                }
            };
        }
    }
    rfs = implode("^", vrfs);
    return rfs;
}

string reqSOAPService(string service, xmlChar* content) {
    xmlInitParser();

    xmlDoc* xd = xmlParseDoc((xmlChar*) "<?xml version=\"1.0\" encoding=\"utf-8\"?><soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\"><soap12:Body><content></content></soap12:Body></soap12:Envelope>");
    xmlXPathContext *xpathCtx = xmlXPathNewContext(xd);
    xmlXPathRegisterNs(xpathCtx, (xmlChar*) "soap12", (xmlChar*) "http://www.w3.org/2003/05/soap-envelope");
    xmlXPathRegisterNs(xpathCtx, (xmlChar*) "xsd", (xmlChar*) "http://www.w3.org/2001/XMLSchema");
    xmlXPathRegisterNs(xpathCtx, (xmlChar*) "xsi", (xmlChar*) "http://www.w3.org/2001/XMLSchema-instance");
    xmlXPathRegisterNs(xpathCtx, (xmlChar*) "n", (xmlChar*) xmlnamespace.c_str());
    xmlXPathObject * accNameXpathObj = xmlXPathEvalExpression((xmlChar*) "/soap12:Envelope/soap12:Body/content", xpathCtx);

    xmlNode *node = accNameXpathObj->nodesetval->nodeTab[0];
    xmlDoc* cxd = xmlParseDoc(content);
    xmlNodePtr xnp = xmlDocGetRootElement(cxd);
    xmlReplaceNode(node, xnp);
    xmlChar* s;
    int size;
    xmlDocDumpMemory(xd, &s, &size);
    xmlCleanupParser();
    SOAPServiceReqCount++;
    string res = (socketType == Socket::TLS1_1 ? SOAPReq(serverAddr, serverPort, "/" + appName + "/CamCaptureService.asmx", xmlnamespace + "/" + service, string((char*) s)) : SOAPReq(serverAddr, serverPort, "/" + appName + "/CamCaptureService.asmx", xmlnamespace + "/" + service, string((char*) s), socketType, trustedCA, privatecert, privatekey));
    return res;
}

camState systemStateChange() {
    getCameras();
    ps = cs;
    cs = csList::camReattached() ? CAM_NEW_STATE : CAM_PREVIOUS_STATE;
    string strCameras = csList::getCamsWithStateString();
    string strGPS = "NOGPSDEVICE";
    time(&currentTime);
    if ((currentTime - GPSManager::gpsReadStart) < gpsUpdatePeriod + 2) {
        strGPS = GPSManager::gpsCoordinates;
    }
    fflush(stdout);
    string IP = GetPrimaryIp();
    if (IP.compare(currentIP) != 0) {
        currentIP = IP;
        csList::setStateAllCams(CAM_RECORD);
        setState();
    } else if (IP.length() == 0) {
        cerr << "\n" + getTime() + " systemStateChange: No IP: CONNECTION ERROR.\n";
        csList::setStateAllCams(CAM_RECORD);
        cs = CAM_NEW_STATE;
        return cs;
    }
    string strNetwork = "ip:" + currentIP + ",signalstrength:-1";
    string content = "<GetDataChangeBySystemId xmlns=\"" + xmlnamespace + "\"><SystemName>" + getMachineName() + "</SystemName><SecurityKey>" + securityKey + "</SecurityKey><Cameras>" + strCameras + "</Cameras><GPS>" + strGPS + "</GPS><network>" + strNetwork + "</network></GetDataChangeBySystemId>";
    ffl_debug(FPOL_MAIN, "SOAPRequest %d: %s", SOAPServiceReqCount, content.c_str());
    string response = reqSOAPService("GetDataChangeBySystemId", (xmlChar*) content.c_str());
    if (response.compare("CONNECTION ERROR") == 0) {
        ffl_err(FPOL_MAIN, "unable to connect server CONNECTION ERROR");
        csList::setStateAllCams(CAM_RECORD);
        cs = CAM_NEW_STATE;
        return cs;
    }
    ffl_debug(FPOL_MAIN, "SOAPResponse:%s", response.c_str());
    xmlChar *res = (xmlChar*) response.c_str();
    xmlDoc *xd = xmlParseDoc(res);
    xmlXPathContext *xpathCtx = xmlXPathNewContext(xd);
    xmlXPathObject * xpathObj = xmlXPathEvalExpression((xmlChar*) "//*[local-name()='GetDataChangeBySystemIdResult']", xpathCtx);
    xmlNode *node = xpathObj->nodesetval->nodeTab[0];
    if (xpathObj->nodesetval->nodeNr > 0) {
        node = node->children;
        string resCon;
        while (node != NULL) {
            resCon = string((char*) xmlNodeGetContent(node));
            if ((int) resCon.find("StartVideoRecord^", 0) >= 0) {
                cs = CAM_RECORD;
            } else if ((int) resCon.find("StopVideoRecord^", 0) >= 0) {
                cs = CAM_OFF;
            } else if ((int) resCon.find("StartVideoStream^", 0) >= 0) {
                cs = CAM_OFF;
            } else if ((int) resCon.find("StopVideoStream^", 0) >= 0) {
                vector<string> devs = explode("^", resCon);
                unsigned int devCount = devs.size();
                int i = 1;
                for (i = 1; i < devCount; i++) {
                    if (csList::setNewCamState(devs[i], CAM_RECORD) == 1) {
                        cs = CAM_NEW_STATE;
                    };
                }
            } else if ((int) resCon.find("StartSoundRecord^", 0) >= 0) {
            } else if ((int) resCon.find("StopSoundRecord^", 0) >= 0) {
            } else if ((int) resCon.find("StartSoundStream^", 0) >= 0) {
            } else if ((int) resCon.find("StopSoundStream^", 0) >= 0) {
            } else if ((int) resCon.find("DeleteSystem", 0) >= 0) {
                uninstall();
            } else if ((int) resCon.find("ViewAllRecordedFiles", 0) >= 0) {
                string strRecordedFiles = getStrRecordedFiles();
                content = "<InsertRecordedFiles xmlns=\"" + xmlnamespace + "\"><SystemSecurityKey>" + securityKey + "</SystemSecurityKey><strRecordedFiles>" + strRecordedFiles + "</strRecordedFiles></InsertRecordedFiles>";
                response = reqSOAPService("InsertRecordedFiles", (xmlChar*) content.c_str());
            } else if ((int) resCon.find("ViewSingleRecordedFile", 0) >= 0) {
                string toStreamVidFilesStr = resCon.substr(23);
                vector<string> toStreamVidFilesVector = explode("^", toStreamVidFilesStr);
                vector<string>toStreamVidFileAttrVect;
                int i = 0;
                for (int i = 0; i < toStreamVidFilesVector.size(); i++) {
                    toStreamVidFileAttrVect.clear();
                    toStreamVidFileAttrVect = explode("%", toStreamVidFilesVector.at(i));
                    string recordName[3] = {toStreamVidFileAttrVect.at(2), toStreamVidFileAttrVect.at(3), toStreamVidFileAttrVect.at(4)};
                    if (toStreamVidFileAttrVect.at(5).compare("True") == 0) {
                        RecordsManager::RecordIndex ri = RecordsManager::addRecord(recordName);
                        if (ri != -1) {
                            RecordsManager::setNewState(ri, RECORD_STREAM);
                            RecordsManager::setSId(ri, toStreamVidFileAttrVect.at(0));
                            RecordsManager::setRecordState(ri);
                        }
                    } else {
                        RecordsManager::RecordIndex ri = RecordsManager::getRecordIndex(recordName);
                        if (ri != -1) {
                            RecordsManager::setNewState(ri, RECORD_STOP);
                            RecordsManager::setRecordState(ri);
                        }
                    }
                }
            } else if ((int) resCon.find("ViewAll^") >= 0) {
                cs = CAM_NEW_STATE;
                string camsi = resCon.substr(8);
                vector<string> a = explode("^", camsi);
                int camCount = a.size();
                vector<string> b;
                int i;
                for (i = 0; i < camCount; i++) {
                    b.empty();
                    b = explode(":", a[i]);
                    csList::setSId(b[0], b[1]);
                }
                csList::setStateAllCams(CAM_STREAM_N_RECORD);
            } else if ((int) resCon.find("View^") >= 0) {
                int sIsind = (int) resCon.find(":", 0) - 5;
                reqCam = resCon.substr(5, sIsind);
                reqSId = resCon.substr(sIsind + 6);
                csList::setSId(reqCam, reqSId);
                if (csList::setNewCamState(reqCam, CAM_STREAM_N_RECORD) == 1) {
                    cs = CAM_NEW_STATE;
                };
            }
            node = node->next;
        }
    }
    xmlCleanupParser();
    fflush(stdout);
    return cs;
}

void print_usage(FILE* stream, int exit_code, char* program_name) {
    fprintf(stream, "Usage: %s <option> [<parameter>]\n", program_name);
    string doc = "-c --configure Configures " + string(APP_NAME) + ""
            "\n-f --config-file <file name> it reads configuration from the file specified. It should be given ahead of all other options"
            "\n-d --update Updates " + string(APP_NAME) + ""
            "\n-h --help Display this usage information."
            "\n-i --install Installs " + string(APP_NAME) + "."
            "\n-k --keyInstall Installs " + string(APP_NAME) + " with key given by user."
            "\n-r --reinstall Reinstall the " + string(APP_NAME) + ""
            "\n-s --start=\033[4mTYPE\033[0m Runs client. If \033[4mTYPE\033[0m is 'daemon' " + string(APP_NAME) + " runs as daemon. If \033[4mTYPE\033[0m is 'normal' " + string(APP_NAME) + " runs normally."
            "\n-u --uninstall Uninstalls " + string(APP_NAME) + "."
            "\n-x --stop Terminates " + string(APP_NAME) + "."
            "\n---------------------------"
            "\nHave a nice day :)\n\n";
    fprintf(stream, (const char*) doc.c_str());
    exit(exit_code);
}

void internetTimeUpdater(void *arg) {
    time_t oldTimeStamp;
    time(&oldTimeStamp);
    ffl_debug(FPOL_MAIN, "spawning ntpdate");
    spawn *ntpdate = new spawn("ntpdate ntp.ubuntu.com", true, NULL, false, true);
    if ((int) ntpdate->getChildExitStatus() == 0) {
        time_t newTimeStamp;
        time(&newTimeStamp);
        timeGapToCorrectTime = newTimeStamp - oldTimeStamp;
        internetTimeUpdated = true;
        if (firstChild != 0) {
            family_message_block_ptr->msg_to = secondChild;
            family_message_block_ptr->msg_from = getpid();
            family_message_block_ptr->msg.type = family_message_block_ptr->msg.CORRECT_TIME_STAMP;
            family_message_block_ptr->msg.data.timeGapToCorrectTime = timeGapToCorrectTime;
            kill(secondChild, SIGUSR2);
        }
        correctAllTimeVariables();
    }
    ffl_debug(FPOL_MAIN, "ntpdate: ces: %d cout: %s cerr %s", ntpdate->getChildExitStatus(), get_fd_contents(ntpdate->cpstdout).c_str(), get_fd_contents(ntpdate->cpstderr).c_str());
    delete ntpdate;
}

void correctAllTimeVariables() {
    ffl_debug(FPOL_MAIN, "process %d is correcting timestamps", getpid());
    GPSManager::gpsReadStart += timeGapToCorrectTime;
    GPSManager::gpsReadEnd += timeGapToCorrectTime;
    nm_presentCheckTime += timeGapToCorrectTime;
    nm_previousCheckTime += timeGapToCorrectTime;
    hub_last_reset_time += timeGapToCorrectTime;
    std::list<time_t*>::iterator i = FerryTimeStamp::ferryTimesList.begin();
    while (i != FerryTimeStamp::ferryTimesList.end()) {
        **i += timeGapToCorrectTime;
        i++;
    }
}

void run() {
    secondChild = getpid();
    if (geteuid() != 0) {
        cout << "\nPlease login as root are sudo user.\n";
    } else {
        if (runMode.compare("daemon") != 0) {
            readConfig();
            if (manageNetwork == 1) {
                pthread_create(&nwMgrThread, NULL, &networkManager, NULL);
            }
        }
        if (securityKey.length() == 0) {
            cout << "\nPlease install or re-install " + string(APP_NAME) + ".";
            fflush(stdout);
        } else {
            GPSManager::init();
            writeRootProcess();
            csList::initialize(10);
            int ecode;
            time(&st);
            time_t ct;
            int pis = atoi(pollInterval.c_str()) / 1000;
            int pst = pis;
            camState csc;
            camState ps;
            int contiguousRunCounter = 0;
            time_t presentRunTime;
            time_t previousRunTime;

            while (true) {
                previousRunTime = presentRunTime;
                time(&presentRunTime);
                if (presentRunTime - previousRunTime <= 10) {
                    contiguousRunCounter++;
                    if (contiguousRunCounter > 3) {
                        contiguousPoll = true;
                        contiguousRunCounter = 0;
                    }
                } else {
                    contiguousRunCounter = 0;
                }
                if (!contiguousPoll) {
                    time(&ct);
                    if (ct - st > 900) {
                        csc = ps;
                        st = ct;
                        pst = 0;
                        csList::resetStateAllCams(CAM_RECORD);
                        cs = CAM_NEW_STATE;
                    } else {
                        if (newlyMarried) {
                            getCameras();
                            csc = CAM_NEW_STATE;
                            cs = csc;
                            csList::setStateAllCams(CAM_RECORD);
                            newlyMarried = false;
                            allCams = true;
                            pst = 0;
                        } else {
                            csc = systemStateChange();
                            pst = pis;
                        }
                    }
                    switch (csc) {
                        case CAM_NEW_STATE:
                            ps = csc;
                            setState();
                            break;
                        case CAM_OFF:
                            ps = csc;
                            ecode = system("pkill -9 ffmpeg");
                            break;
                        default:
                            break;
                    }
                    sleep(pst);
                } else {
                    contiguousPoll = false;
                    contiguousRunCounter = 0;
                    time(&previousRunTime);
                    sleep(pst);
                    time(&presentRunTime);
                    while ((presentRunTime - previousRunTime) < pst) {
                        previousRunTime = presentRunTime;
                        time(&presentRunTime);
                        sleep(pst);
                    }
                }
            }
        }
    }
}

void install() {
    if (geteuid() != 0) {
        cout << "\nPlease login as root are sudo user.\n";
    } else {
        readConfig();
        int ret = system("which ffmpeg > /dev/null");
        if (ret == 0) {
            cout << "\n Do you want to install existing key(y) or a new key(n)? ";
            string yn = inputText();
            if (yn.compare("n") == 0) {
                cout << "Installation process for MEKCamController\nPlease give in prompted information...";
                cout << "\nusername: ";
                string username = inputText();
                cout << "\npassword: ";
                string password = inputPass();
                string content = "<VendorLoginForClientComponent xmlns='" + xmlnamespace + "'><AccountName></AccountName><Username>" + username + "</Username><Password>" + password + "</Password></VendorLoginForClientComponent>";
                string res = reqSOAPService("VendorLoginForClientComponent", (xmlChar*) content.c_str());
                xmlInitParser();
                xmlDoc* xd2 = xmlParseDoc((xmlChar*) res.c_str());
                xmlXPathContext *xpc2 = xmlXPathNewContext(xd2);
                xmlXPathObject * tableXpathObj = xmlXPathEvalExpression((xmlChar*) "//*[local-name()='Table']", xpc2);
                xmlNode *node4;
                int nn = tableXpathObj->nodesetval->nodeNr;
                if (nn > 0) {
                    cout << "\nSelect one of the systems:\n----------------------------\n";
                    cout << "SNO\tBranchId\tBranchName\tSystemId\tSystemName";
                    fflush(stdout);
                    int i;
                    char* bn[nn];
                    for (i = 0; i < tableXpathObj->nodesetval->nodeNr; i++) {
                        node4 = tableXpathObj->nodesetval->nodeTab[i];
                        cout << ("\n" + string(itoa(i + 1)) + ".\t");
                        fflush(stdout);
                        cout << (string((char*) xmlNodeGetContent(node4->children)) + "\t\t" + string((char*) xmlNodeGetContent(node4->children->next)) + "\t" + string(bn[i] = (char*) xmlNodeGetContent(node4->children->next->next)) + "\t\t" + string((char*) xmlNodeGetContent(node4->children->next->next->next)));
                        fflush(stdout);
                    }
                    xmlCleanupParser();
                    cout << "\nSelect a system(SNO): ";
                    cin >> i;
                    if (i > 0 && i <= nn) {
                        i -= 1;
                        instReInstComCode(bn[i]);
                    } else {
                        cout << "\nSNO out of range.";
                        cout << "\nSelect a system(SNO): ";
                        cin >> i;
                    }
                } else {
                    cout << "\nNo systems are allocated to this user.\n";
                }
                xmlCleanupParser();
            } else {
                reinstallKey();
            }
        } else {
            cout << "\nffmpeg is not installed or not allowed to run as root. Please install ffmpeg and allow it to run as root.";
        }
    }
}

void reinstallKey() {
    readConfig();
    string sk;
    cout << "\nInstalling " + string(APP_NAME) + "...";
    cout << "\nSecurity key: ";
    sk = inputText();
    instReInstComCode(sk);
}

void reinstall() {
    struct stat st;
    if (stat(configFile.c_str(), &st) == -1) {
        cout << "Configuration file not found. Install " + string(APP_NAME) + ".";
    } else {
        string sk = readConfigValue("system-id");
        string cmd = string(APP_NAME) + " -u";
        system(cmd.c_str());
        readConfig();
        instReInstComCode(sk);
    }
}

void installFiles() {
    cout << "Writing required files to file system.";
    struct stat st = {0};
    if (stat(recordsFolder.c_str(), &st) == -1) {
        if (0 == mkdir(recordsFolder.c_str(), 0774))cout << "\n" + recordsFolder + " is created";
    }
    if (0 == copyfile("init.conf", initFile))
        cout << "\n/etc " + initFile + " is created.";
    if (0 == copyfile("init.d", initdFile))
        cout << "\n/etc " + initdFile + " is created.";
    if (0 == copyfile("config.xml", configFile))
        cout << "\n/etc " + configFile + " is created.";
    if (0 == copyfile("devices.rules", deviceRulesFile))
        cout << "\n/etc " + deviceRulesFile + " is created.";
    if (0 == copyfile(string(APP_NAME), rootBinLnk)) {
        string modCmd = "chmod u+x " + rootBinLnk;
        system(modCmd.c_str());
    }
    if (0 == copyfile("certs/ferryfair.cert", trustedCA))
        cout << "\n " + trustedCA + " is created.";
    if (0 == copyfile("certs/ferryport.ferryfair.cert", privatecert))
        cout << "\n " + privatecert + " is created.";
    if (0 == copyfile("certs/ferryport.ferryfair.key", privatekey))
        cout << "\n " + privatekey + " is created.";
    if (0 == copyfile("error.log", logFile))
        cout << "\n" + logFile + " is created.";
}

void instReInstComCode(string sk) {
    string content = (string) "<UpdateSystemInstallStatus xmlns='" + xmlnamespace + "'><SecurityKey>" + sk + "</SecurityKey><InstallStatus>1</InstallStatus></UpdateSystemInstallStatus>";
    string res = reqSOAPService("UpdateSystemInstallStatus", (xmlChar*) content.c_str());
    if ((int) res.find(">1<", 0) != -1) {
        writeConfigValue("system-id", sk);
        securityKey = sk;
        content = "<AddSystem xmlns=\"" + xmlnamespace + "\"><SystemSecurityKey>" + securityKey + "</SystemSecurityKey><SystemName>" + getMachineName() + "</SystemName><IPAddress /></AddSystem>";
        res = reqSOAPService("AddSystem", (xmlChar*) content.c_str());
        xmlInitParser();
        xmlDoc* xd = xmlParseDoc((xmlChar*) res.c_str());
        xmlXPathContext *xc = xmlXPathNewContext(xd);
        xmlXPathObject* xo = xmlXPathEvalExpression((xmlChar*) "//*[local-name()='AddSystemResult']", xc);
        xmlNode* node;
        if (xo->nodesetval->nodeNr > 0) {
            node = xo->nodesetval->nodeTab[0];
            string resultmsg = string((char*) xmlNodeGetContent(node->children));
            if (resultmsg.compare("System Updated Successfully.") == 0) {
                videoStreamingType = string((char*) xmlNodeGetContent(node->children->next));
                autoInsertCameras = string((char*) xmlNodeGetContent(node->children->next->next));
                pollInterval = string((char*) xmlNodeGetContent(node->children->next->next->next));
                writeConfigValue("videoStreamingType", videoStreamingType);
                writeConfigValue("autoInsertCameras", autoInsertCameras);
                writeConfigValue("pollInterval", pollInterval);
                xmlCleanupParser();
                struct stat st;
                if (stat(initFile.c_str(), &st) != -1) {
                    cout << "\nAllowing to run " + string(APP_NAME) + " at startup...";
                    if (stat(initOverrideFile.c_str(), &st) != -1) {
                        string rmfcmd = "rm " + initOverrideFile;
                        system(rmfcmd.c_str());
                    }
                    writeConfigValue("bootup", "true");
                    cout << "ok\n";
                } else {
                    cout << "\nInstall " + string(APP_NAME) + " in first.\n";
                }
                cout << "\n" + string(APP_NAME) + " installed successfully :D"
                        "\nDo u wanna start " + string(APP_NAME) + " now? [Y/n]: ";
                string input = inputText();
                cout << "\n";
                if (input.length() == 0 || tolower(input).compare("y") == 0) {
                    string cmd = "start " + string(APP_NAME);
                    system(cmd.c_str());
                } else if (tolower(input).compare("n") == 0) {
                    cout << string(APP_NAME) + " will start at next system startup. To change configuration run '" + string(APP_NAME) + " -c'\n";
                }
            } else {
                cout << "\nSorry some one booked the system while u r choosing! Try again.\n";
            }
        } else {
            cout << "\n" + res;
            cout << "\nQuandary :( Please contact administrator.\n";
        }
    } else {
        cout << "\n" + res;
        cout << "\nQuandary :( Please contact administrator.\n";
    }
}

void configure() {
    readConfig();
    cout << "\nCurrent " + string(APP_NAME) + " configuration:\n----------------------------";
    cout << "\napp-name:\t" + appName;
    cout << "\nserver-addr:\t" + serverAddr;
    cout << "\nserver-port:\t" + serverPort;
    cout << "\nstream-addr:\t" + streamAddr;
    cout << "\nstream-port:\t" + streamPort;
    cout << "\nconnection-encryption:\t" + readConfigValue("connection-encryption");
    cout << "\nnamespace:\t" + xmlnamespace;
    cout << "\ncamcapture-compression:\t" + readConfigValue("camcapture-compression");
    cout << "\nrecord-fps:\t" + recordfps;
    cout << "\nrecord-resolution:\t" + recordResolution;
    cout << "\nstream-fps:\t" + streamfps;
    cout << "\nstream-resolution:\t" + streamResolution;
    cout << "\nbootup:\t" + readConfigValue("bootup");
    cout << "\nsystem-id:\t" + systemId;
    cout << "\nvideoStreamingType:\t" << stream_type_str[stream_type];
    cout << "\nenable-mic:\t" << (enable_mic ? "true" : "false");
    cout << "\nautoInsertCameras:\t" + autoInsertCameras;
    cout << "\npollInterval:\t" + pollInterval;
    cout << "\nmanage-network:\t" + string(itoa(manageNetwork));
    cout << "\nreconnect-poll-count:\t" + string(itoa(reconnectPollCount));
    cout << "\nreconnect-duration:\t" + string(itoa(reconnectDuration / 60));
    cout << "\ninternet-test-url:\t" + internetTestURL;
    cout << "\nmobile-broadband-connection:\t" + mobileBroadbandCon;
    cout << "\nmobile-modem-vendor-product-id:\t" + mobileModemVendorProductID;
    cout << "\nusb-hub-vendor-product-id:\t" + usbHubVendorProductID;
    cout << "\nconfig-modem:\t" + readConfigValue("config-modem");
    cout << "\ncorporate-network-gateway:\t" + corpNWGW;
    cout << "\ngps-device:\t" + gpsDevice;
    cout << "\ngps-sdevice-baudrate:\t" + gpsSDeviceBaudrate;
    cout << "\ngps-protocol:\t" << (const char*) GPSManager::gpsProtoStr[GPSManager::gp];
    cout << "\ndebug:\t" + string(itoa(debug));
    cout << "\ncmos-batt:\t" + readConfigValue("cmos-batt");
    cout << "\n-------------------------\nSet or Add configuration property\n";
    string pn;
    string val;
    while (true) {
        cout << "\nProperty name:";
        pn = inputText();
        if (pn.length() != 0) {
            cout << "\nValue:";
            val = inputText();
            if (pn.compare("bootup") == 0) {
                if (geteuid() != 0) {
                    cout << "\nPlease login as root are sudo user.\n";
                } else {
                    struct stat st;
                    if (val.compare("true") == 0) {
                        if (stat(initFile.c_str(), &st) != -1) {
                            cout << "\nAllowing to run " + string(APP_NAME) + " at startup...";
                            if (stat(initOverrideFile.c_str(), &st) != -1) {
                                string rmfcmd = "rm " + initOverrideFile;
                                system(rmfcmd.c_str());
                            }
                            writeConfigValue(pn, val);
                            cout << "ok\n";
                        } else {
                            cout << "\nInstall " + string(APP_NAME) + " in first.\n";
                        }
                    } else if (val.compare("false") == 0) {
                        if (stat(initFile.c_str(), &st) != 1) {
                            cout << "\nDisabling " + string(APP_NAME) + " to run at startup...";
                            if (stat(initOverrideFile.c_str(), &st) == -1) {
                                int fd = open(initOverrideFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
                                string buf = "manual";
                                write(fd, buf.c_str(), buf.length());
                                close(fd);
                            }
                            writeConfigValue(pn, val);
                            cout << "ok\n";
                        } else {
                            cout << "\nInstall " + string(APP_NAME) + " in first.\n";
                        }
                    }
                }
            } else {
                writeConfigValue(pn, val);
            }
        } else {
            cout << "\n";
            fflush(stdout);
            break;
        }
    }
}

bool networkManagerRunning = false;

struct networkManagerCleanUpBuffers {
    bool * networkManagerRunning;
} nMCUB;

void networkManagerCleanUp(void* buffers) {
    ffl_debug(FPOL_MAIN, "setting networkManagerRunning to false");
    networkManagerCleanUpBuffers *b = (networkManagerCleanUpBuffers*) buffers;
    *b->networkManagerRunning = false;
}

void secondFork() {
    secondChild = fork();
    if (secondChild != 0) {
        fflush(stdout);
        if (manageNetwork == 1 && !networkManagerRunning) {
            pthread_create(&nwMgrThread, NULL, &networkManager, NULL);
        }
        int status;
wait_till_child_dead:
        int deadpid = waitpid(secondChild, &status, 0);
        if (deadpid == -1 && waitpid(secondChild, &status, WNOHANG) == 0) {
            goto wait_till_child_dead;
        }
        ffl_warn(FPOL_MAIN, "%d process exited!", deadpid);
        secondFork();
    } else {
        secondChild = getpid();
        ffl_notice(FPOL_MAIN, "second child started; pid= %d", secondChild);
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        run();
    }
}

void firstFork() {
    readConfig();
    if (runMode.compare("daemon") == 0) {
        if ((debug & 1) == 1) {
            dup2(ferr, 1);
            stdoutfd = ferr;
        } else {
            close(1);
        }
    }
    firstChild = fork();
    if (firstChild != 0) {
        int status;
wait_till_child_dead:
        int deadpid = waitpid(firstChild, &status, 0);
        if (deadpid == -1 && waitpid(firstChild, &status, WNOHANG) == 0) {
            goto wait_till_child_dead;
        }
        ffl_debug(FPOL_MAIN, "%d process exited", deadpid);
        firstFork();
    } else {
        firstChild = getpid();
        ffl_notice(FPOL_MAIN, "firstChild started; pid=%d", firstChild);
        fflush(stdout);
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        secondFork();
    }
}

int log(string prefix, string msg) {
    int fd = open(logFile.c_str(), O_WRONLY | O_APPEND);
    struct tm * timeinfo;
    char tb[20];
    time(&currentTime);
    timeinfo = localtime(&currentTime);
    strftime(tb, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
    string ts = string(tb);
    string log = ts + " " + prefix + ": " + msg;
    write(fd, log.c_str(), log.length());
    close(fd);
}

void signalHandler(int signal_number) {
    ffl_debug(FPOL_LL, "process %d received signal %d", getpid(),
            signal_number);
    if (signal_number == SIGUSR1) {
        int fd;
        void* file_memory;
        string fn = "/var/" + string(APP_NAME) + ".data";
        fd = open(fn.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
        file_memory = mmap(0, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
        munmap(file_memory, 1024);
    }
    if (signal_number == SIGUSR2) {
        if ((family_message_block_ptr->msg_to == 0) ||
                (family_message_block_ptr->msg_to == getpid())) {
            if (family_message_block_ptr->msg.type ==
                    family_message_block_ptr->msg.CORRECT_TIME_STAMP) {
                timeGapToCorrectTime =
                        family_message_block_ptr->msg.data.timeGapToCorrectTime;
                correctAllTimeVariables();
            }
        }
    }
    if (signal_number == SIGTERM || signal_number == SIGINT) {
        if (getpid() == rootProcess) {
            ffl_err(FPOL_MAIN, "%s terminated by %d number", APP_NAME,
                    signal_number);
        }
    }
    if (signal_number == SIGCHLD) {
        pid_t pid;
        int status;
        while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
            child_exit_status = status;
            if (processMap[pid] != NULL) {
                spawn *process = processMap[pid];
                ffl_debug(FPOL_LL, "process %d's child %s with pid %d exited.",
                        process->cmdName.c_str(), pid);
                process->childExitStatus = status;
                process->onStopHandler(process);
            }
        }
    }
    pkilled = true;
    fflush(stdout);
    fflush(stderr);
}

void stopRunningProcess() {
    if (runningProcess > 0) {
        ffl_notice(FPOL_MAIN | NO_NEW_LINE, "Stopping current process...");
        if (kill(runningProcess, SIGTERM) != -1) {
            cout << "OK\n";
        } else {
            cout << "FAILED\n";
        }
    }
}

void update() {
    struct stat st = {0};
    string cmd;
#ifdef _WIN64
    //define something for Windows (64-bit)
#elif _WIN32
    //define something for Windows (32-bit)
#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
    // iOS Simulator
#elif TARGET_OS_IPHONE
    // iOS device
#elif TARGET_OS_MAC
    // Other kinds of Mac OS
#else
    // Unsupported platform
#endif
#elif __linux
    // linux
    cout << "\n" + getTime() + " updating " << APP_NAME << " ...\n";
    fflush(stdout);
    spawn * update = new spawn("apt-get -q --force-yes install " + string(APP_NAME), false, NULL, false, true);
    char updateerr[100];
    updateerr[0] = '\0';
    read(update->cpstderr, updateerr, 100);
    cout << "\n" + getTime() + " update->exitcode=" + string(itoa(update->getChildExitStatus())) + ",updateerr->error=" + string(updateerr) + "\n";
    cout << "\n" + getTime() + " Writing present configuration...\n";
    fflush(stdout);
    writeConfigValue("system-id", securityKey);
    writeConfigValue("videoStreamingType", videoStreamingType);
    writeConfigValue("autoInsertCameras", autoInsertCameras);
    writeConfigValue("pollInterval", pollInterval);
    writeConfigValue("server-addr", serverAddr);
    writeConfigValue("server-port", serverPort);
    if (update->getChildExitStatus() == 0) {
        spawn * restart = new spawn("init 6");
        exit(0);
    }
#elif __unix // all unices not caught above
    // Unix
    if (stat(srcFolder.c_str(), &st) == -1) {
        chdir("/usr/local/src");
        cmd = "git clone git://github.com/bulbmaker/" + string(APP_NAME) + ".git";
        system(cmd.c_str());
    } else {
        chdir(srcFolder.c_str());
        cout << "\nGit: pulling from online repostiory...";
        fflush(stdout);
        cmd = "git pull";
        system(cmd.c_str());
    }
    chdir(srcFolder.c_str());
    cout << "\nbuilding " + string(APP_NAME) + "...";
    cmd = "make clean";
    system(cmd.c_str());
    cmd = "make";
    system(cmd.c_str());
    cout << "Spawning " + string(APP_NAME) + "...";
    cmd = "./" + string(APP_NAME) + " -r";
    spawn(cmd, false, NULL, true);
#elif __posix
    // POSIX
#endif

}

void onEndTestSpawnHandler(spawn* process) {
    printf("\nls ended\n");
}

void* test(void *) {
    /*To test websockets test-echo.c*/
    //char * options[] = {"remotedevicecontroller", "-c"};
    //testecho(1, (char**) options);

    /*To test libavcodec libavcodec-example.c*/
    //char * options[] = {"libavcodec_example", "mpg"};
    //libavcodec_example(2, options);

    /*To test capture.c*/
    //    int argc = 12;
    //    char * argv[] = {"capture", "", "", "-c", "60", "-f", "mjpeg", "-i", "/dev/video0", "-o", "-O", "libav_output_files/cam_capture_vid_c60_r5_yuyv422_mjpeg.raw"};
    //    //capture(argc, argv);

    /*pthread_sigmask test*/
    //    sigset_t chldmask;
    //    if ((sigemptyset(&chldmask) == -1) || (sigaddset(&chldmask, SIGCHLD) == -1)) {
    //        perror("Failed to initialize the signal mask");
    //        return NULL;
    //    }
    //    if (pthread_sigmask(SIG_BLOCK, &chldmask, NULL) == -1) {
    //        return NULL;
    //    }
    //    fprintf(stderr, "SIGCHLD signal blocked\n");
    //    int child = fork();
    //    if (child == 0) {
    //        cout << "\nI'm child " << getpid() << " exiting now.\n";
    //        fflush(stdout);
    //        exit(0);
    //    }
    //    cout << "\nI'm parent " << getpid() << " not yet exiting.\n";
    //    fflush(stdout);
    //    cout << "\ngonna sleep for 10 secs\n";
    //    sleep(10);
    //    cout << "\nunblocking SIGCHLD after 10 secs\n";
    //    if (pthread_sigmask(SIG_UNBLOCK, &chldmask, NULL) == -1) {
    //        return NULL;
    //    }
    //    cout << "\nExiting the thread!\n";

    /*To test pcm open*/
    //    pcm_open_set_device();

    /*Testing MediaManager*/
    setuid(1000);
    debug = 17;
    readConfig();
    valarray<MediaManager::media> imedia(2);
    imedia[0].duration = 0;
    imedia[0].encoding = MediaManager::MJPEG;
    imedia[0].height = 240;
    imedia[0].identifier = "/dev/video0";
    imedia[0].type = MediaManager::VIDEO;
    imedia[0].videoframerate = 10;
    imedia[0].width = 320;
    imedia[1].audioSamplingFrequency = 44100;
    imedia[1].duration = 0;
    imedia[1].identifier = "plughw:1";
    imedia[1].type = MediaManager::AUDIO;
    //    imedia[2].duration = 0;
    //    imedia[2].encoding = MediaManager::MJPEG;
    //    imedia[2].height = 240;
    //    imedia[2].identifier = "/dev/video2";
    //    imedia[2].type = MediaManager::VIDEO;
    //    imedia[2].videoframerate = 0.4;
    //    imedia[2].width = 320;
    //    imedia[3].audioSamplingFrequency = 44100;
    //    imedia[3].duration = 0;
    //    imedia[3].identifier = "default";
    //    imedia[3].type = MediaManager::AUDIO;
    valarray<MediaManager::media> omedia(1);
    omedia[0].identifier = "fmsp://fms.newmeksolutions.com:92711/" + appName + "1780";
    //    omedia[0].identifier = "ferrymediacapture1/";
    omedia[0].segmentDuration = 1;
    omedia[0].videoframerate = 10;
    omedia[0].audioBitrate = 64000;
    omedia[0].duration = 10;
    omedia[0].encoding = MediaManager::MP2;
    omedia[0].splMediaProps.fmpFeederSplProps.reconnect = true;
    omedia[0].splMediaProps.fmpFeederSplProps.reconnectIntervalSec = 10;
    omedia[0].signalNewState = 0;
    MediaManager::capture(imedia, omedia);
    /*stat*/
    //    struct stat statbuf;
    //    struct passwd *pwd;
    //    struct group *grp;
    //    struct tm *tm;
    //    char datestring[256];
    //    struct dirent *dp;
    //
    //    stat("/dev/CDMAModem", &statbuf);
    //    /* Print out type, permissions, and number of links. */
    //    printf("%10.10s", sperm(statbuf.st_mode));
    //    printf("%4d", statbuf.st_nlink);
    //
    //
    //    /* Print out owner's name if it is found using getpwuid(). */
    //    if ((pwd = getpwuid(statbuf.st_uid)) != NULL)
    //        printf(" %-8.8s", pwd->pw_name);
    //    else
    //        printf(" %-8d", statbuf.st_uid);
    //
    //
    //    /* Print out group name if it is found using getgrgid(). */
    //    if ((grp = getgrgid(statbuf.st_gid)) != NULL)
    //        printf(" %-8.8s", grp->gr_name);
    //    else
    //        printf(" %-8d", statbuf.st_gid);
    //
    //
    //    /* Print size of file. */
    //    printf(" %9jd", (intmax_t) statbuf.st_size);
    //
    //
    //    tm = localtime(&statbuf.st_mtime);
    //
    //
    //    /* Get localized date string. */
    //    strftime(datestring, sizeof (datestring), nl_langinfo(D_T_FMT), tm);
    //
    //
    //    printf(" %s %s\n", datestring, dp->d_name);
    //    int fd = open("/dev/CDMAModem", O_WRONLY);
    //    if (fd > 0) {
    //        int rc = ioctl(fd, USBDEVFS_RESET, 0);
    //        if (rc < 0) {
    //            cerr << "\n" << getTime() << " test: Error in ioctl\n";
    //        } else {
    //        #ifdef DEBUG
    //            if ((debug & 1) == 1) {
    //                cout << "\n" << getTime() << " test: ioctl reset successful\n";
    //                fflush(stdout);
    //            }
    //             #endif
    //        }
    //        close(fd);
    //    }

    /*Testing Modem Manager*/
    //set_mobile_modem_bus_device_file_name();
    //networkManager(NULL);
    /*Testing GPS*/
    //    gpsManager::gpsLocationUpdater(NULL);

    /*Testing libudev*/
    //    struct udev *udev;
    //    struct udev_enumerate *enumerate;
    //    struct udev_list_entry *devices, *dev_list_entry;
    //    struct udev_device *dev;
    //
    //    udev = udev_new();
    //
    //    enumerate = udev_enumerate_new(udev);
    //    udev_enumerate_add_match_subsystem(enumerate, "tty");
    //    udev_enumerate_scan_devices(enumerate);
    //    devices = udev_enumerate_get_list_entry(enumerate);
    //
    //    udev_list_entry_foreach(dev_list_entry, devices) {
    //        const char *path;
    //
    //        path = udev_list_entry_get_name(dev_list_entry);
    //        dev = udev_device_new_from_syspath(udev, path);
    //
    //        fprintf(stderr, "devnum: %s\n",
    //                udev_device_get_sysattr_value(dev, "devnum"));
    //        fprintf(stderr, "busnum: %s\n",
    //                udev_device_get_sysattr_value(dev, "busnum"));
    //        udev_device_unref(dev);
    //    }
    //
    //    udev_enumerate_unref(enumerate);
    //    udev_unref(udev);

    /*GPS*/
    //    debug = 9;
    //    readConfig();
    //    GPSManager::gpsLocationUpdater(NULL);

    /*ValGrind Tool in ReadConfig*/
    //    readConfig();
    //    fflush(stdout);

    /*Conditional Compilation*/
    //#ifdef DEBUG
    //    std::cout << "MakeFile Chnages Reflected\n";
    //#endif
}

void set_bus_device_file_name(string vpid, string& bus_device_file_name) { //#vpid-vendor product id
    spawn lsusb_spawn("lsusb", false, NULL, NULL, true);
    char lsusbout[1000];
    lsusbout[0] = '\0';
    read(lsusb_spawn.cpstdout, lsusbout, 1000);
    string wvdialerrstr = string(lsusbout);
    int ref = wvdialerrstr.find(vpid, 0);
    if (ref > 0) {
        string device_no = wvdialerrstr.substr(ref - 8, 3);
        string bus_no = wvdialerrstr.substr(ref - 19, 3);
        bus_device_file_name = "/dev/bus/usb/" + bus_no + "/" + device_no;
#ifdef DEBUG
        if ((debug & 1) == 1) {
            cout << "\n" << getTime() << " set_bus_device_file_name: lsusb: bud_device_file_name of  vendor:product  " << vpid << " device is " << bus_device_file_name << "\n";
            fflush(stdout);
        }
#endif
    } else {
        //if ((debug & 1) == 1) {
        cout << "\n" << getTime() << " set_bus_device_file_name: lsusb detected no device with vendor:product id " << vpid << "\n";
        fflush(stdout);
        //}
    }
}

void usb_reset(string device) {
    int fd = open(device.c_str(), O_WRONLY);
    if (fd > 0) {
        int rc = ioctl(fd, USBDEVFS_RESET, 0);
        if (rc < 0) {
            volatile_usb_hub_reset_interval = 10;
            cerr << "\n" << getTime() << " usb_reset: Error in ioctl on " << device << ".\n";
            ffl_err(FPOL_LL, "usb_reset: Error in ioctl on %s", device.c_str());
        } else {
            ffl_warn(FPOL_LL, "usb_reset: ioctl reset on %s successful", device.c_str());
        }
        close(fd);
    } else {
        ffl_err(FPOL_MAIN, "usb_reset: unable to open %s", device.c_str());
    }
}

void* networkManager(void* arg) {
    networkManagerRunning = true;
    pthread_cleanup_push(&networkManagerCleanUp, &nMCUB);
    ffl_debug(FPOL_MAIN, "NetworkManager started");
    time_t waitInterval = reconnectDuration;
    spawn *wvdial = NULL;
    spawn *wvdialconf;
    time(&nm_presentCheckTime);
    nm_previousCheckTime = nm_presentCheckTime - reconnectDuration;
    int remainingSleepTime;
    while (true) {
        remainingSleepTime = reconnectDuration - waitInterval;
        remainingSleepTime = remainingSleepTime < 0 ? 0 : remainingSleepTime;
        ffl_debug(FPOL_MAIN, "process %d sleeping for %d", getpid(), remainingSleepTime);
sleep_enough_time:
        sleep((int) remainingSleepTime);
        time(&nm_presentCheckTime);
        waitInterval = nm_presentCheckTime - nm_previousCheckTime;
        if (waitInterval >= reconnectDuration) {
            nm_previousCheckTime = nm_presentCheckTime;
            waitInterval = 0;
            if (poke(internetTestURL) != 0) {
                sleep(4);
                if (poke(internetTestURL) != 0) {
                    if (mobileBroadbandCon.length() > 0) {
                        if (mobileBroadbandCon.compare("wvdial") == 0) {
                            if (wvdial != NULL) {
                                kill(wvdial->cpid, SIGTERM);
                                char wvdialerr[500];
                                memset(wvdialerr, 0, 500);
                                read(wvdial->cpstderr, wvdialerr, 500);
                                string wvdialerrstr = string(wvdialerr);
                                ffl_debug(FPOL_MAIN, "wvdial killed; wvdail->pid=%d, wvdial->exitcode=%d,wvdialerr->error=%s", wvdial->cpid, wvdial->getChildExitStatus(), wvdialerrstr.c_str());
                                ffl_debug(FPOL_MAIN, "Gonna re-spawn wvdial :) don worry I will connect u to the network!...If u've given me what I need (;");
                                waitpid(wvdial->cpid, NULL, WNOHANG);
                                delete wvdial;
                                if ((int) wvdialerrstr.find("Cannot open /dev/CDMAModem:", 0) > 0) {
                                    time_t currentTime = 0;
                                    time(&currentTime);
                                    if (currentTime - hub_last_reset_time > volatile_usb_hub_reset_interval) {
                                        volatile_usb_hub_reset_interval = const_usb_hub_reset_interval;
                                        set_bus_device_file_name(usbHubVendorProductID, usb_hub_bus_device_file_name);
                                        usb_reset(usb_hub_bus_device_file_name);
                                        sleep(2);
                                        time(&hub_last_reset_time);
                                        //mobile_modem_disconnect_toggle = true;
                                    }
                                } else if ((int) wvdialerrstr.find("Modem not responding", 0) > 0) {
                                    set_bus_device_file_name(mobileModemVendorProductID, mobile_modem_bus_device_file_name);
                                    usb_reset(mobile_modem_bus_device_file_name);
                                }
                                sleep(2);
                                wvdial = new spawn("wvdial", true, NULL, true, false);
                            } else {
                                sleep(30);
                                if (configModem) {
                                    wvdialconf = new spawn("wvdialconf", true, NULL, true, true);
#ifdef DEBUG
                                    if ((debug & 1) == 1) {
                                        char wvdialconferr[500];
                                        read(wvdialconf->cpstderr, wvdialconferr, 500);
                                        cout << "\n" + getTime() + " networkManager: wvdialconf->exitcode=" + string(itoa(wvdialconf->getChildExitStatus())) + ",wvdialconferr->error=" + string(wvdialconferr) + ". sleeping 10 seconds...\n";
                                        fflush(stdout);
                                    }
#endif
                                    sleep(10);
                                }
                                spawn("pkill wvdial", false, NULL, false, true);
                                wvdial = new spawn("wvdial", true, NULL, true, false);
#ifdef DEBUG
                                if ((debug & 1) == 1) {
                                    cout << "\n" + getTime() + " networkManager: wvdialed(" << wvdial->cpid << ") for the 1st time.\n";
                                    fflush(stdout);
                                }
#endif
                            }
                        } else {
                            spawn *ifup = new spawn("nmcli con up id " + mobileBroadbandCon + " --timeout 30", false, NULL, false, true);
                            if (ifup->getChildExitStatus() != 0) {
#ifdef DEBUG
                                if ((debug & 1) == 1) {
                                    char ifuperr[100];
                                    read(ifup->cpstderr, ifuperr, 100);
                                    cout << "\n" + getTime() + " networkManager: ifup->exitcode=" + string(itoa(ifup->getChildExitStatus())) + ",ifup->error=" + string(ifuperr) + ". sleeping 10 seconds...\n";
                                    fflush(stdout);
                                }
#endif

                                sleep(30);
                                spawn *ifup2 = new spawn("nmcli con up id " + mobileBroadbandCon, false, NULL, false, true);
                                if (ifup2->getChildExitStatus() != 0) {
#ifdef DEBUG
                                    if ((debug & 1) == 1) {
                                        cout << "\n" + getTime() + " networkManager: nmcli stderror:";
                                        char buf[100];
                                        read(ifup->cpstderr, buf, 100);
                                        printf("%s", buf);
                                        cout << ". Exitcode=" + string(itoa(ifup2->getChildExitStatus())) + ".\n";
                                        fflush(stdout);
                                    }
#endif
#ifdef DEBUG
                                    if ((debug & 1) == 1) {
                                        cout << "\n" + getTime() + " networkManager: disabling wwan." + "\n";
                                        fflush(stdout);
                                    }
#endif
                                    spawn *disableCon = new spawn("nmcli nm wwan off", false, NULL, false, true);
                                    sleep(10);
#ifdef DEBUG
                                    if ((debug & 1) == 1) {
                                        cout << "\n" + getTime() + " networkManager: enabling wwan." + "\n";
                                        fflush(stdout);
                                    }
#endif
                                    spawn *enableCon = new spawn("nmcli nm wwan on", false, NULL, false, true);
                                    delete enableCon;
                                    delete disableCon;
                                } else {
#ifdef DEBUG
                                    if ((debug & 1) == 1) {
                                        cout << "\n" + getTime() + "networkManager: nmcli:";
                                        char buf[200];
                                        read(ifup->cpstdout, buf, 200);
                                        printf("%s", buf);
                                        cout << "\n";
                                        fflush(stdout);
                                    }
#endif
                                }
                                delete ifup2;
                            } else {
#ifdef DEBUG
                                if ((debug & 1) == 1) {
                                    cout << "\n" + getTime() + " networkManager: ifup: connected." + "\n";
                                    fflush(stdout);
                                }
#endif
                            }
                            delete ifup;
                        }
                    }
                }
            } else if (!CMOSWorking && !internetTimeUpdated) {
                internetTimeUpdater(NULL);
            }
        }
    }
    pthread_cleanup_pop(1);
}

string readPreviousProcess() {
    ifstream inData;
    inData.open((const char*) runningProcessFile.c_str());
    string pid;
    getline(inData, pid);
    return pid;
}

int writeRootProcess() {
    int fd = open((const char*) runningProcessFile.c_str(), O_WRONLY | O_CREAT);
    std::string pid = itoa((int) rootProcess);
    write(fd, pid.c_str(), pid.length());
    close(fd);
}

int main(int argc, char** argv) {
    //    GPSManager::gpsProtoStr[0] = string("GPS_PROTO_UNKNOWN");
    //    GPSManager::gpsProtoStr[1] = string("GPS_PROTO_LOCAL");
    //    GPSManager::gpsProtoStr[2] = string("GPS_PROTO_NMEA0183");
    camStateStringInit();
    family_message_block_id = shmget(IPC_PRIVATE, sizeof (family_message_block), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    family_message_block_ptr = (family_message_block*) shmat(family_message_block_id, NULL, 0);
    struct stat statbuf;
    int stat_r = stat(logFile.c_str(), &statbuf);
    ferr = open(logFile.c_str(), O_WRONLY | ((stat_r == -1 || statbuf.st_size > 5000000) ? (O_CREAT | O_TRUNC) : O_APPEND));
    stdinfd = dup(0);
    stdoutfd = dup(1);
    stderrfd = dup(2);
    runningProcess = atoi(readPreviousProcess().c_str());
    struct sigaction signalaction_struct;
    memset(&signalaction_struct, 0, sizeof (signalaction_struct));
    signalaction_struct.sa_handler = &signalHandler;
    sigaction(SIGCHLD, &signalaction_struct, NULL);
    sigaction(SIGUSR1, &signalaction_struct, NULL);
    sigaction(SIGUSR2, &signalaction_struct, NULL);
    int next_option;
    const char* short_options = "cdf:hikrs:tux";
    string opt;
    const struct option long_options[] = {
        {"configure", 0, NULL, 'c'},
        {"config-file", 1, NULL, 'f'},
        {"update", 0, NULL, 'd'},
        {"help", 0, NULL, 'h'},
        {"install", 0, NULL, 'i'},
        {"keyInstall", 0, NULL, 'k'},
        {"reinstall", 0, NULL, 'r'},
        {"start", 1, NULL, 's'},
        {"test", 0, NULL, 't'},
        {"uninstall", 0, NULL, 'u'},
        {"stop", 0, NULL, 'x'},
        {NULL, 0, NULL, 0}
    };

    do {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);
        switch (next_option) {
            case 'h':
                print_usage(stdout, 0, argv[0]);
                break;
            case 's':
                stopRunningProcess();
                rootProcess = getpid();
                opt = string(optarg);
                if (opt.compare("daemon") == 0) {
                    runMode = opt;
                    close(1);
                    dup2(ferr, 2);
                    stderrfd = ferr;
                    firstFork();
                } else if (opt.compare("normal") == 0) {
                    run();
                }
                break;
            case 'i':
                stopRunningProcess();
                install();
                break;
            case 'r':
                reinstall();
                break;
            case 'c':
                configure();
                break;
            case 'u':
                uninstall();
                break;
            case 'x':
                stopRunningProcess();
                break;
            case 'd':
                update();
                break;
            case 'k':
                reinstallKey();
                break;
            case 't':
                //                pthread_t test_thread;
                //                pthread_create(&test_thread, NULL, &test, NULL);
                //                pthread_join(test_thread, NULL);
                test(NULL);
                break;
            case 'f':
                configFile = string(optarg);
                break;
            case '?':
                print_usage(stderr, 1, argv[0]);
                break;
            case -1:
                print_usage(stderr, 1, argv[0]);
                break;
        }
    } while (next_option != -1);
    fflush(stdout);
    shmdt(family_message_block_ptr);
    shmctl(family_message_block_id, IPC_RMID, 0);
    return 0;
}
