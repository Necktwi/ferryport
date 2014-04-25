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

#define MAX_CAMS 10
#define APP_NAME "remotedevicecontroller"
using namespace std;

enum RecordState {
    RECORD_PREVIOUS_STATE, RECORD_STOP, RECORD_STREAM
};
string recordStateStr[] = {"RECORD_PREVIOUS_STATE", "RECORD_STOP", "RECORD_STREAM"};

enum camState {
    CAM_PREVIOUS_STATE, CAM_OFF, CAM_RECORD, CAM_STREAM, CAM_STREAM_N_RECORD, CAM_NEW_STATE
};
string camStateString [] = {"CAM_PREVIOUS_STATE", "CAM_OFF", "CAM_RECORD", "CAM_STREAM", "CAM_STREAM_N_RECORD", "CAM_NEW_STATE"};


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
string recordfps;
string streamfps;
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
    int stdio[2];
    string cam;
    camState state;
    camState newState;
    string SId;
    bool disable;
    string recordPath;
    string streamPath;
};

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
#ifdef DEBUG
            if ((debug & 8) == 8) {
                cout << "\n" + getTime() + " gpsManager: No bluetooth gps found; sleeping for 20 seconds\n";
                fflush(stdout);
            }
#endif
            sleep(20);
            initConnectTrials--;
        } else {
#ifdef DEBUG
            if ((debug & 8) == 8) {
                cout << "\n" + getTime() + " gpsManager: No bluetooth gps found; sleeping for 120 seconds\n";
                fflush(stdout);
            }
#endif
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
#ifdef DEBUG
                    if ((debug & 8) == 8) {
                        cout << "\n" + getTime() + " gpsManager: gpsCoordinates : " + gpsCoordinates + ".\n";
                        fflush(stdout);
                    }
#endif
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
#ifdef BEDUG
                    if ((debug & 8) == 8) {
                        cout << "\n" + getTime() + " gpsManager: gpsCoordinates : " + gpsCoordinates + ".\n";
                        fflush(stdout);
                    }
#endif
                } else {
#ifdef DEBUG
                    if ((debug & 1) == 1) {
                        cout << "\n" + getTime() + " gpsManager: illegal string from GPS device.\n";
                        fflush(stdout);
                    }
#endif
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
#ifdef DEBUG
            if ((debug & 1) == 1) {
                cout << "\n" + getTime() + " gpsManager: connecting to bluetooth gps device...\n";
                fflush(stdout);
            }
#endif
            sleep(6);
        }
opendevice:
        FILE *f = fopen(gpsDevice.c_str(), "r");
#ifdef DEBUG
        if ((debug & 1) == 1) {
            cout << "\n" + getTime() + " gpsManager: reading gpsdevice:" + gpsDevice + "\n";
            fflush(stdout);
        }
#endif
        if (gt == RS232 && f && gpsSDeviceBaudrate.length() > 0) {
            cmd = "stty -F " + gpsDevice + " " + gpsSDeviceBaudrate;
            spawn *gpsdbrsetter = new spawn(cmd, false, NULL, false, true);
            delete gpsdbrsetter;
#ifdef DEBUG
            if ((debug & 1) == 1) {
                cout << "\n" + getTime() + " gpsManager: baudrate set:cmd:" + cmd + "\n";
                fflush(stdout);
            }
#endif
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
#ifdef DEBUG
            if ((debug & 1) == 1) {
                cout << "\n" + getTime() + " gpsManager: enabling serial port ttyO1\n";
                fflush(stdout);
            }
#endif
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
#ifdef DEBUG
                if ((debug & 1) == 1) {
                    cout << "\n" + getTime() + " gpsManager: gps device disconnected. sleeping for 10 secs.\n";
                    fflush(stdout);
                }
#endif
                sleep(10);
                goto opendevice;
            }
        }
#ifdef DEBUG
        if ((debug & 1) == 1) {
            cout << "\n" + getTime() + " gpsManager: no gps device found. sleeping for 60 secs.\n";
            fflush(stdout);
        }
#endif
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
            string cmd = "ffmpeg -re -i " + rfa + " -r " + streamfps + " -s " + streamResolution + " -f flv " + sa;
            records[rIndex].recorder = spawn(cmd, true, NULL, false);
#ifdef DEBUG
            if ((debug & 1) == 1) {
                cout << "\n" + getTime() + " setRecordState: " + cmd + " :" + string(itoa(records[rIndex].recorder.cpid)) + "\n";
                fflush(stdout);
            }
#endif
            records[rIndex].spid = records[rIndex].recorder.cpid;
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
    read(process->cpstderr, ffmpegerr, 100);
    string ffmpegerrstr = string(ffmpegerr);
#ifdef DEBUG
    if ((debug & 1) == 1) {
        cout << "\n" << getTime() << " ffmpegOnStopHandler: ffmpeg->pid=" << process->cpid << ", ffmpeg->exitcode=" << (int) process->getChildExitStatus() << ",ffmpegerr->error=" << ffmpegerr << "\n";
        fflush(stdout);
    }
#endif
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
        set_bus_device_file_name(usbHubVendorProductID, usb_hub_bus_device_file_name);
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
#ifdef DEBUG
    if ((debug & 1) == 1) {
        cout << "\n" + getTime() + " correctTimeStampFileNames: correcting timestamps ...\n";
        fflush(stdout);
    }
#endif
    while (recordedFileNames.size() != 0) {
        fileName = recordedFileNames[recordedFileNames.size() - 1];
        recordedFileNames.pop_back();
        lastslashpos = fileName.rfind('/', fileName.length());
        fold = fileName.substr(0, lastslashpos);
        camName = fold.substr(fold.rfind('/', fileName.length()) + 1);
        timestamp = fileName.substr(lastslashpos + 1);
        strptime(timestamp.c_str(), "%Y-%m-%d_%H:%M:%S.flv", &tms);
        t = mktime(&tms);
        t += timeGapToCorrectTime;
        struct tm * timeinfo;
        char fn [80];
        char dn [80];
        timeinfo = localtime(&t);
        strftime(fn, 80, "%Y-%m-%d_%H:%M:%S.flv", timeinfo);
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
#ifdef DEBUG
        if ((debug & 1) == 1) {
            cout << "\n" + getTime() + " correctTimeStampFileNames: renamed " << fileName << " to " << fname << "\n";
            fflush(stdout);
        }
#endif
    }
}

class csList {
private:
    static int s;
    static camService csl[];
    static int camCount;

    static int moveCamService(int srcIndex, int dstIndex) {
        if (csList::csl[srcIndex].cam.length() > 0) {
            csList::copyCamService(srcIndex, dstIndex);
            csList::cleanCamService(srcIndex);
            return dstIndex;
        } else {
            return -1;
        }
    }

    static int copyCamService(int srcIndex, int dstIndex) {
        if (csList::csl[srcIndex].cam.length() > 0) {
            csList::csl[dstIndex].cam = csList::csl[srcIndex].cam;
            csList::csl[dstIndex].pid = csList::csl[srcIndex].pid;
            csList::csl[dstIndex].SId = csList::csl[srcIndex].SId;
            csList::csl[dstIndex].recordPath = csList::csl[srcIndex].recordPath;
            csList::csl[dstIndex].state = csList::csl[srcIndex].state;
            csList::csl[dstIndex].newState = csList::csl[srcIndex].newState;
            csList::csl[dstIndex].disable = csList::csl[srcIndex].disable;
            return srcIndex;
        } else {
            return -1;
        }
    }

    static int cleanCamService(int srcIndex) {
        if (srcIndex < MAX_CAMS && srcIndex>-1) {
            csList::csl[srcIndex].cam = "";
            csList::csl[srcIndex].SId = "";
            csList::csl[srcIndex].disable = false;
            csList::csl[srcIndex].newState = CAM_PREVIOUS_STATE;
            csList::csl[srcIndex].pid = 0;
            csList::csl[srcIndex].recordPath = "";
            csList::csl[srcIndex].state = CAM_PREVIOUS_STATE;
            csList::csl[srcIndex].streamPath = "";
        }
    }
public:

    csList() {
        camCount = 0;
        s = 0;
    }

    static void initialize(int ss) {
        s = ss;
    }

    static void addCamService(string cam) {
        int i = 0;
        bool camAdded = false;
        while (csList::csl[i].cam.length() != 0) {
            if ((int) csList::csl[i].cam.find(cam, 0) == 0) {
                camAdded = true;
            }
            i++;
        }
        if (!camAdded) {
            csList::csl[i].cam = cam;
            csList::camCount++;
            csList::csl[i].state = CAM_OFF;
            csList::csl[i].newState = CAM_RECORD;
        }
    }

    static void removeCamService(string cam) {
        int csIndex;
        camService cs = getCamService(cam, &csIndex);
        if (csIndex != -1) {
            if (csList::camCount > 1) {
                csList::moveCamService(csList::camCount - 1, csIndex);
            } else {
                csList::csl[csIndex].cam = "";
            }
        }
        csList::camCount--;
    }

    static camService getCamService(string cam, int * index) {
        int i = 0;
        camService *nul = new camService;
        while (csl[i].cam.length() != 0) {
            if (csl[i].cam.compare(cam) == 0) {
                *index = i;
                return csl[i];
            }
            i++;
        }
        *index = -1;
        return *nul;
    }

    static int getCamCount() {
        return csList::camCount;
    }

    static bool camReattached() {
        int i = 0;
        string procF = "/proc/";
        string proc;
        struct stat ptr;
        while (i < csList::camCount) {
            if (!csList::csl[i].disable) {
                proc = procF + string(itoa(csl[i].pid));
                if (stat(proc.c_str(), &ptr) == -1) {
                    return true;
                }
            }
            i++;
        }
        return false;
    }

    static pid_t setCamState(string cam) {
        int csIndex;
        camService cs = csList::getCamService(cam, &csIndex);
        if (!cs.disable) {
            string dev = camFolder + cam;
            pid_t fcpid = cs.pid;
            camState ns = cs.state;
            spawn *process;
            string cmd;
            if (cs.newState == CAM_RECORD) {
                cmd = "ffmpeg -loglevel error -f video4linux2 " + (camcaptureCompression ? string("-vcodec mjpeg ") : string("")) + "-r " + recordfps + " -s " + recordResolution + " -i " + dev + " " + cs.recordPath;
                csList::stopCam(cam);
                process = new spawn(cmd, true, &ffmpegOnStopHandler, false);
#ifdef DEBUG
                if ((debug & 1) == 1) {
                    cout << "\n" + getTime() + " setCamState: " + cmd + " :" + string(itoa(process->cpid)) + "\n";
                    fflush(stdout);
                }
#endif
                fcpid = process->cpid;
                ns = CAM_RECORD;
                recordedFileNames.push_back(cs.recordPath);
            } else if (cs.newState == CAM_STREAM) {
                cmd = "ffmpeg -loglevel error -f video4linux2 " + (camcaptureCompression ? string("-vcodec mjpeg ") : string("")) + "-r " + recordfps + " -s " + recordResolution + " -i " + dev + " -r " + streamfps + " -s " + streamResolution + " -f flv " + cs.streamPath;
                csList::stopCam(cam);
                process = new spawn(cmd, true, &ffmpegOnStopHandler, false);
#ifdef DEBUG
                if ((debug & 1) == 1) {
                    cout << "\n" + getTime() + " setCamState: " + cmd + " :" + string(itoa(process->cpid)) + "\n";
                    fflush(stdout);
                }
#endif
                fcpid = process->cpid;
                ns = CAM_STREAM;
            } else if (cs.newState == CAM_STREAM_N_RECORD) {
                cmd = "ffmpeg -loglevel error -f video4linux2 " + (camcaptureCompression ? string("-vcodec mjpeg ") : string("")) + "-r " + recordfps + " -s " + recordResolution + " -i " + dev + " -r " + streamfps + " -s " + streamResolution + " -f flv " + cs.streamPath + " " + cs.recordPath;
                csList::stopCam(cam);
                process = new spawn(cmd, true, &ffmpegOnStopHandler, false);
#ifdef DEBUG
                if ((debug & 1) == 1) {
                    cout << "\n" + getTime() + " setCamState: " + cmd + " :" + string(itoa(process->cpid)) + "\n";
                    fflush(stdout);
                }
#endif
                fcpid = process->cpid;
                ns = CAM_STREAM_N_RECORD;
                if (!CMOSWorking&&!updateTimeStamps) {
                    recordedFileNames.push_back(cs.recordPath);
                }
            } else if (cs.newState == CAM_OFF) {
                kill(cs.pid, SIGTERM);
            }
            csList::csl[csIndex].pid = fcpid;
            csList::csl[csIndex].state = ns;
            csList::csl[csIndex].newState = CAM_PREVIOUS_STATE;
            return fcpid;
        } else {
            return -1;
        }
    }

    static int stopCam(string cam) {
        int i = 0;
        int csIndex;
        camService cs = csList::getCamService(cam, &csIndex);
        string proc = "/proc/" + string(itoa(cs.pid));
        struct stat ptr;
        if (stat(proc.c_str(), &ptr) != -1) {
            pkilled = false;
            i = kill(cs.pid, SIGKILL);
            while (!pkilled);
        }
        cs.state = CAM_OFF;
        return i;
    }

    static void setCams(string * cams) {
        int i = 0;
        int j = 0;
        bool camFound = false;
        string procf = "/proc/";
        string proc;
        struct stat ptr;
        while (i < csList::camCount) {
            proc = procf + string(itoa(csList::csl[i].pid));
            if (stat(proc.c_str(), &ptr) == -1) {
                camFound = false;
                csList::removeCamService(csList::csl[i].cam);
            } else {
                i++;
            }
        }
        i = 0;
        while (cams[i].length() > 0) {
            csList::addCamService(string(cams[i]));
            i++;
        }
    }

    static int setPid(string cam, pid_t pid) {
        int csIndex;
        csList::getCamService(cam, &csIndex);
        csList::csl[csIndex].pid = pid;
        return csIndex;
    }

    static int setSId(string cam, string SId) {
        int csIndex;
        csList::getCamService(cam, &csIndex);
        if (csIndex != -1) {
            csList::csl[csIndex].SId = SId;
            return csIndex;
        } else {
            return -1;
        }
    }

    static int setNewCamState(string cam, camState ns) {
        int csIndex;
        getCamService(cam, &csIndex);
        if (csIndex != -1) {
            if (csList::csl[csIndex].state != ns) {
                csList::csl[csIndex].newState = ns;
                return 1;
            } else {
                return 0;
            }
        } else {
            return -1;
        }
    }

    static void setRecordPath(string cam, string recordPath) {
        int csIndex;
        getCamService(cam, &csIndex);
        csList::csl[csIndex].recordPath = recordPath;
    }

    static void setStreamPath(string cam, string streamPath) {
        int csIndex;
        getCamService(cam, &csIndex);
        csList::csl[csIndex].streamPath = streamPath;
    }

    static int setStateAllCams(camState state) {
        int i = 0;
        while (i < csList::camCount) {
            if (csList::csl[i].state != state) {
                csList::csl[i].newState = state;
            }
            i++;
        }
        return 0;
    }

    static int resetStateAllCams(camState state) {
        int i = 0;
        while (i < csList::camCount) {
            csList::csl[i].newState = state;
            i++;
        }
        return 0;
    }

    static void getCams(string * cams) {
        int i = 0;
        while (i < csList::camCount) {
            cams[i] = string(csList::csl[i].cam);
            i++;
        }
    }

    static string getCamsWithStateString() {
        string strCameras = "";
        int i = 0;
        while (i < csList::camCount) {
            strCameras += csList::csl[i].cam + ":" + camStateString[csList::csl[i].state] + ((i + 1 == csList::camCount) ? "" : "^");
            i++;
        }
        return strCameras;
    }

};
int csList::s = 0;
camService csList::csl[MAX_CAMS];
int csList::camCount;

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
    xo = xmlXPathEvalExpression((xmlChar*) "/config/camcapture-compression", xc);
    node = xo->nodesetval->nodeTab[0];
    camcaptureCompression = (string((char*) xmlNodeGetContent(node)).compare("true") == 0);
    xo = xmlXPathEvalExpression((xmlChar*) "/config/record-fps", xc);
    node = xo->nodesetval->nodeTab[0];
    recordfps = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/record-resolution", xc);
    node = xo->nodesetval->nodeTab[0];
    recordResolution = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/stream-fps", xc);
    node = xo->nodesetval->nodeTab[0];
    streamfps = string((char*) xmlNodeGetContent(node));
    xo = xmlXPathEvalExpression((xmlChar*) "/config/stream-resolution", xc);
    node = xo->nodesetval->nodeTab[0];
    streamResolution = string((char*) xmlNodeGetContent(node));
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

void setState() {
    time_t rawtime;
    struct tm * timeinfo;
    char fn [80];
    char dn [80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(fn, 80, "%Y-%m-%d_%H:%M:%S.flv", timeinfo);
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
    int csIndex;
    camService cs;
    while (vd[i].length() != 0) {
        k = 0;
        if (!(cs = csList::getCamService(vd[i], &csIndex)).disable && (csIndex != -1) && cs.newState != CAM_PREVIOUS_STATE) {
            fold = path + vd[i];
            if (stat(fold.c_str(), &fileAtt) == -1) {
                mkdir(fold.c_str(), 0774);
            }
            dev = "/dev/" + vd[i];
            fname = fold + "/" + fn;
            sa = "rtmp://" + streamAddr + ":" + streamPort + "/oflaDemo/" + cs.SId;
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
#ifdef DEBUG
    if ((debug & 1) == 1) {
        cout << "\n" + getTime() + " systemStateChange: SOAPRequest " + string(itoa(SOAPServiceReqCount)) + ": " + content + "\n";
        fflush(stdout);
    }
#endif
    string response = reqSOAPService("GetDataChangeBySystemId", (xmlChar*) content.c_str());
    if (response.compare("CONNECTION ERROR") == 0) {
        cerr << "\n" + getTime() + " systemStateChange: unable to connect server CONNECTION ERROR.\n";
        csList::setStateAllCams(CAM_RECORD);
        cs = CAM_NEW_STATE;
        return cs;
    }
#ifdef DEBUG
    if ((debug & 1) == 1) {
        cout << "\n" + getTime() + " systemStateChange: SOAPResponse: " + response + "\n";
        fflush(stdout);
    }
#endif
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
#ifdef DEBUG
    if ((debug & 1) == 1) {
        cout << "\n" + getTime() + " internetTimeUpdater: spawning ntpdate\n";
        fflush(stdout);
    }
#endif
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
#ifdef DEBUG
    if ((debug & 1) == 1) {
        cout << "\n" + getTime() + " internetTimeUpdater: ntpdate: ces: " << (int) ntpdate->getChildExitStatus() << " cout: " + get_fd_contents(ntpdate->cpstdout) + " cerr: " + get_fd_contents(ntpdate->cpstderr) + " \n";
        fflush(stdout);
    }
#endif
    delete ntpdate;
}

void correctAllTimeVariables() {
#ifdef DEBUG
    if ((debug & 1) == 1) {
        cout << "\n" << getTime() << " correctAllTimeVariables: process " << getpid() << " is correcting timestamps.\n";
    }
#endif
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
    cout << "\nvideoStreamingType:\t" + videoStreamingType;
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
#ifdef DEBUG
    if ((debug & 1) == 1) {
        cout << "\n" << getTime() << " " << getpid() << " networkManagerCleanUp: setting networkManagerRunning to false.\n";
    }
#endif
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
#ifdef DEBUG
        if ((debug & 1) == 1) {
            cout << "\n" << getTime() << " secondFork: " << deadpid << "process exited!\n";
        }
#endif
        secondFork();
    } else {
        secondChild = getpid();
        cout << "\n" << getTime() << " secondFork: Second child started; pid= " << secondChild << "\n";
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        run();
    }
}

void firstFork() {
    readConfig();
    if (runMode.compare("daemon") == 0) {
#ifdef DEBUG
        if ((debug & 1) == 1) {
            dup2(ferr, 1);
            stdoutfd = ferr;
        } else {
            close(1);
        }
#endif
    }
    firstChild = fork();
    if (firstChild != 0) {
        int status;
wait_till_child_dead:
        int deadpid = waitpid(firstChild, &status, 0);
        if (deadpid == -1 && waitpid(firstChild, &status, WNOHANG) == 0) {
            goto wait_till_child_dead;
        }
#ifdef DEBUG      
        if ((debug & 1) == 1) {
            cout << "\n" << getTime() << " firstFork: " << deadpid << "process exited!\n";
        }
#endif
        firstFork();
    } else {
        firstChild = getpid();
        cout << "\n" << getTime() << " firstFork: firstChild child started; pid= " << firstChild << "\n";
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
#ifdef DEBUG 
    if ((debug & 32) == 32) {
        cout << "\n" << getTime() << " signalHandler: process " << getpid() << " received signal " << signal_number << "\n";
    }
#endif
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
        if ((family_message_block_ptr->msg_to == 0) || (family_message_block_ptr->msg_to == getpid())) {
            if (family_message_block_ptr->msg.type == family_message_block_ptr->msg.CORRECT_TIME_STAMP) {
                timeGapToCorrectTime = family_message_block_ptr->msg.data.timeGapToCorrectTime;
                correctAllTimeVariables();
            }
        }
    }
    if (signal_number == SIGTERM || signal_number == SIGINT) {
        if (getpid() == rootProcess) {
            cerr << "\n" + getTime() + " signalHandler: " + string(APP_NAME) + " terminated by " + string(itoa(signal_number)) + " number.\n";
        }
    }
    if (signal_number == SIGCHLD) {
        pid_t pid;
        int status;
        while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
            child_exit_status = status;
            if (processMap[pid] != NULL) {
                spawn *process = processMap[pid];
#ifdef DEBUG
                if ((debug & 32) == 32) {
                    cout << "\n" << getTime() << " signalHandler: process " << getpid() << "'s child \"" << process->cmdName << "\" with pid " << pid << " exited.\n";
                    fflush(stdout);
                }
#endif
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
        cout << "\nStopping current process.....";
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
    /*Testing MediaManager*/
    setuid(1000);
    debug = 17;
    readConfig();
    valarray<MediaManager::media> imedia(2);
    imedia[0].audioSamplingFrequency = 44100;
    imedia[0].duration = 0;
    imedia[0].identifier = "default";
    imedia[0].type = MediaManager::AUDIO;
    imedia[1].duration = 0;
    imedia[1].encoding = MediaManager::MJPEG;
    imedia[1].height = 240;
    imedia[1].identifier = "/dev/video0";
    imedia[1].type = MediaManager::VIDEO;
    imedia[1].videoframerate = 0.4;
    imedia[1].width = 320;
    valarray<MediaManager::media> omedia(1);
    omedia[0].identifier = "fmsp://fms.newmeksolutions.com:92711/" + appName + "/1780";
    //    omedia[0].identifier = "ferrymediacapture1/";
    omedia[0].segmentDuration = 3;
    omedia[0].videoframerate = 1;
    omedia[0].audioBitrate = 64000;
    omedia[0].duration = 10;
    omedia[0].encoding = MediaManager::MP2;
    omedia[0].splMediaProps.fmpFeederSplProps.reconnect = true;
    omedia[0].splMediaProps.fmpFeederSplProps.reconnectIntervalSec = 10;
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
        } else {
#ifdef DEBUG
            if ((debug & 1) == 1) {
                cout << "\n" << getTime() << " usb_reset: ioctl reset on " << device << " successful\n";
                fflush(stdout);
            }
#endif
        }
        close(fd);
    } else {
        //if ((debug & 1) == 1) {
        cout << "\n" << getTime() << " usb_reset: unable to open " << device << "\n";
        fflush(stdout);
        //}
    }
}

void* networkManager(void* arg) {
    networkManagerRunning = true;
    pthread_cleanup_push(&networkManagerCleanUp, &nMCUB);
#ifdef DEBUG
    if ((debug & 1) == 1) {
        cout << "\n" + getTime() + " networkManager: started.\n";
        fflush(stdout);
    }
#endif

    time_t waitInterval = reconnectDuration;
    spawn *wvdial = NULL;
    spawn *wvdialconf;
    time(&nm_presentCheckTime);
    nm_previousCheckTime = nm_presentCheckTime - reconnectDuration;
    int remainingSleepTime;
    while (true) {
        remainingSleepTime = reconnectDuration - waitInterval;
        remainingSleepTime = remainingSleepTime < 0 ? 0 : remainingSleepTime;
#ifdef DEBUG
        if ((debug & 1) == 1) {
            cout << "\n" << getTime() << " " << getpid() << " networkManager: sleeping for " << remainingSleepTime << ".\n";
        }
#endif
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
#ifdef DEBUG
                                if ((debug & 1) == 1) {
                                    cout << "\n" + getTime() + " networkManager: wvdial killed; wvdial->pid=" << wvdial->cpid << ", wvdial->exitcode=" + string(itoa(wvdial->getChildExitStatus())) + ",wvdialerr->error=" + wvdialerrstr + ". \n";
                                    cout << "\n" + getTime() + " networkManager: Gonna re-spawn wvdial :) don worry I will connect u to the network!...If u've given me what I need (;\n";
                                    fflush(stdout);
                                }
#endif

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
    char* pid = itoa((int) rootProcess);
    write(fd, pid, strlen(pid));
    close(fd);
}

int main(int argc, char** argv) {
    //    GPSManager::gpsProtoStr[0] = string("GPS_PROTO_UNKNOWN");
    //    GPSManager::gpsProtoStr[1] = string("GPS_PROTO_LOCAL");
    //    GPSManager::gpsProtoStr[2] = string("GPS_PROTO_NMEA0183");
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
    const char* const short_options = "cdhikrs:tux";
    string opt;
    const struct option long_options[] = {
        {"configure", 0, NULL, 'c'},
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
            case '?':
                print_usage(stderr, 1, argv[0]);
                break;
            case -1:
                print_usage(stderr, 1, argv[0]);
                break;
        }
        break;
    } while (next_option != -1);
    fflush(stdout);
    shmdt(family_message_block_ptr);
    shmctl(family_message_block_id, IPC_RMID, 0);
    return 0;
}
