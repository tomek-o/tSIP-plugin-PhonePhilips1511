#include "CommThread.h"
#include "..\tSIP\tSIP\phone\Phone.h"
#include "HidDevice.h"
#include "Log.h"
#include "CustomConf.h"
#include "Philips1511.h"
#include "Mutex.h"
#include "ScopedLock.h"
#include <windows.h>
#include <assert.h>
#include <time.h>
#include <vector>


void Key(int keyCode, int state);

using namespace nsHidDevice;
using namespace Philips1511;

namespace {
volatile bool connected = false;
volatile bool exited = false;

Mutex mutexState;
int regState = 0;
int callState = 0;
int ringState = 0;
std::string callDisplay;
bool displayUpdateFlag = false;
bool ringUpdateFlag = false;

std::string GetCallDisplay(void) {
    ScopedLock<Mutex> lock(mutexState);
    return callDisplay;
}

/** \note Two buttons: "Skype" (KEY_APP) and Mute seem to randomly generate
duplicated codes, one correct and one with one of the two bits missing. These are the
only two buttons using more that 1 bit in code, perhaps they are handled in a special way.
To deal with this issue, small delay was added to button event processing and possible
false codes are rejected.
Observation: keys can be combined (detected when they are pressed together)
except of those two keys using more that one bit in code.
*/
void UpdateButtons(uint16_t code, bool active) {
    enum E_KEY key;
    uint16_t rawKey = code;
    LOG("Philips 1511: Update buttons, code 0x%04X, %s", code, active?"down":"up");
    switch (rawKey) {
    case 0x8000:
        key = KEY_0;
        break;
    case 0x0100:
        key = KEY_1;
        break;
    case 0x1000:
        key = KEY_2;
        break;
    case 0x0001:
        key = KEY_3;
        break;
    case 0x0200:
        key = KEY_4;
        break;
    case 0x2000:
        key = KEY_5;
        break;
    case 0x0002:
        key = KEY_6;
        break;
    case 0x0400:
        key = KEY_7;
        break;
    case 0x4000:
        key = KEY_8;
        break;
    case 0x0004:
        key = KEY_9;
        break;
    case 0x0800:
        key = KEY_STAR;
        break;
    case 0x0008:
        key = KEY_HASH;
        break;
//    case 0x07:
//        key = KEY_OK;
//        break;
//    case 0x0080:      // down arrow
//        key = KEY_C;
//        break;
    case 0x0040:
        key = KEY_UP;
        break;
    case 0x0080:
        key = KEY_C;
        //key = KEY_DOWN;
        break;
    case 0x2200:
        key = KEY_APP;
        break;
    case 0x0010:
        key = KEY_CALL_MAKE_ANSWER;
        break;
    case 0x0020:
        key = KEY_CALL_HANGUP;
        break;
    case 0x8800:
        key = KEY_MUTE_TOGGLE;
        break;
    default:
        LOG("Unhandled raw key code 0x%04X", rawKey);
        return;
    }
    Key(key, active?1:0);
}

struct Report {
    unsigned char data[4];
    Report(unsigned char d0, unsigned char d1, unsigned char d2, unsigned char d3)
    {
        data[0] = d0;
        data[1] = d1;
        data[2] = d2;
        data[3] = d3;
    }
};

int UpdateDisplay(HidDevice &dev) {
    int status;

    displayUpdateFlag = false;

    std::string callDisplayVal = GetCallDisplay();

    std::vector<Report> reports;

    reports.push_back(Report(0x10, 0x00, 0x00, 0x30));
    reports.push_back(Report(0x50, CmdLed, ringState?0x04:0x00, 0x01));
    reports.push_back(Report(0x50, CmdDispStart, 0x00, 0x00)); // 137 = 0x89

    if (callDisplayVal.empty()) {
        // display time
        time_t now = time(NULL);
        struct tm *tm_struct = localtime(&now);
        int hour = tm_struct->tm_hour;
        int minute = tm_struct->tm_min;
        int second = tm_struct->tm_sec;
        reports.push_back(Report(0x50, CmdTimeStartHHMM, 0x01, 0x00));
        reports.push_back(Report(0x50, CmdTime, '0' + hour / 10, '0' + hour % 10)); // bytes 2, 3: ASCII hour
        reports.push_back(Report(0x50, CmdTime, '0' + minute / 10, '0' + minute % 10)); // bytes 2, 3: ASCII minutes
        reports.push_back(Report(0x50, CmdTime, '0' + second / 10, '0' + second % 10)); // for CmdTimeStartHHMM: not displayed but apparently required
    } else {
        unsigned int pairs = callDisplayVal.length() / 2;
        for (unsigned int i=0; i<pairs; i++) {
            char c1 = toupper(callDisplayVal[i*2]);
            char c2 = toupper(callDisplayVal[i*2 + 1]);
            reports.push_back(Report(0x50, CmdCharAtPos, c1, c2));
        }
        if (callDisplayVal.length() % 2) {
            char c1 = toupper(callDisplayVal[pairs*2]);
            reports.push_back(Report(0x50, CmdCharAtPos, c1, 0x00));
        } else {
            reports.push_back(Report(0x50, CmdCharAtPos, 0x00, 0x00));
        }
    }
    reports.push_back(Report(0x50, CmdPingTwoSeconds, 0x00, 0x04));  // ping

#if 0
    // test: using different modes one by one
    enum { MODE_START = 0x60 };
    enum { MODE_END = 0xC9 };
    static unsigned int mode = MODE_START;

    unsigned char buffers[][4] = {
        { 0x10, 0x00, 0x00, 0x30 },
        { 0x50, 0x9A, 0x00, 0x01 },
        { 0x50, 0x89, 0x00, 0x00 },
        { 0x50, 0x8B, 0x03, 0x00 }, // 2nd byte: display mode?
        { 0x50, 0x8C, 0x31, 0x32 }, // bytes 2, 3: ASCII hour
        { 0x50, 0x8C, 0x33, 0x34 }, // bytes 2, 3: ASCII minutes
        { 0x50, 0x8C, 0x35, 0x36 },
        { 0x50, 0xA1, 0x03, 0x06 }, // byte [1]: 0x93 => RINGER YYYYY
                                    // byte [1]: 0x94 => VOL YYYYY__
                                    // byte [1]: 0x95 => check + blinking clock
                                    // 0x96 -> more symbols
                                    // 0x99 -> "MUTE"
                                    // 0x9A -> blinking LED
                                    // 0x9B -> fast blinking symbols
                                    // 0x9C -> ring
    };


    buffers[3][2] = mode;
    LOG("Philips: writing mode = 0x%02X", mode);

    mode++;
    if (mode > MODE_END)
        mode = MODE_START;
#endif

    for (unsigned int i=0; i<reports.size(); i++) {
        const Report &report = reports[i];
        status = dev.WriteReport(HidDevice::E_REPORT_OUT, 0, report.data, sizeof(report.data));
        if (status != 0)
            break;
        Sleep(10);
    }

    if (status != 0) {
        LOG("Philips: UpdateDisplay status/error = %d", status);
    }
    return status;
}

int UpdateRing(HidDevice &dev) {
    int status;

    ringUpdateFlag = false;

    unsigned char sendbuf[] = { 0x50, CmdRing, 0x07, 0x8D };
    // Philips app shows 6 different ring type sounds, but all are quite awful
    sendbuf[2] += customConf.ringType;

    LOG("Philips 1511: UpdateRing: state = %d, type = %u", ringState, customConf.ringType);

    // ring ON
    if (ringState) {

    } else {
        sendbuf[2] = 0x00;
        sendbuf[3] = 0x00;
    }

    status = dev.WriteReport(HidDevice::E_REPORT_OUT, 0, sendbuf, sizeof(sendbuf));
    Sleep(10);
    return status;
}

}

DWORD WINAPI CommThreadProc(LPVOID data) {
    //ThreadComm *thrRead = (ThreadComm*)data;
    HidDevice hidDevice;
    bool devConnected = false;
    unsigned int loopCnt = 0;

    unsigned char rcvbuf[4];
    LOG("Running Philips VOIP1511 comm thread");
    bool lastCodeFilled = false;
    uint16_t lastCode = 0;
    bool lastCodePending = false;

    while (connected) {
        if (devConnected == false) {
            if (loopCnt % 100 == 0) {
                int status = hidDevice.Open(VendorID, ProductID, NULL, NULL);
                if (status == 0) {
                    devConnected = true;
                    LOG("Philips VOIP1511 connected");
                } else {
                    LOG("Error opening Philips VOIP1511: %s", HidDevice::GetErrorDesc(status).c_str());
                }
            }
        } else {
            int status = 0;
            if ((loopCnt & 0x03) == 0) {
                displayUpdateFlag = true;
            }

            if (displayUpdateFlag) {
                status = UpdateDisplay(hidDevice);
            }
            if (status == 0 && ringUpdateFlag) {
                status = UpdateRing(hidDevice);
            }
            if (status) {
                LOG("Philips VOIP1511: error updating, %s", HidDevice::GetErrorDesc(status).c_str());
                hidDevice.Close();
                devConnected = false;
                lastCodeFilled = false;
                lastCodePending = false;
            } else {
                int size = sizeof(rcvbuf);
                int status = hidDevice.ReadReport(HidDevice::E_REPORT_IN, 0, (char*)rcvbuf, &size, 100);
                if (status == 0) {
                    if (size == sizeof(rcvbuf)) {
                        uint16_t code = 256 * rcvbuf[1] + rcvbuf[2];
                        LOG("Philips 1511: REPORT_IN received: %02X %02X %02X %02X, code 0x%04X, prev code 0x%04X [%s]",
                            rcvbuf[0], rcvbuf[1], rcvbuf[2], rcvbuf[3],
                            static_cast<unsigned int>(code),
                            static_cast<unsigned int>(lastCode),
                            lastCodeFilled?"filled":"not filled"
                        );
                        if ((code != lastCode) || !lastCodeFilled)
                        {
                            if (lastCodeFilled) {
                                if (code && lastCode) {
                                    LOG("Philips 1511: rejecting/ignoring code 0x%04X", static_cast<unsigned int>(std::min(code, lastCode)));
                                    code = std::max(code, lastCode);    // lower code rejected (0x8800 mute or 0x2200 Skype wins)
                                    lastCode = code;
                                    lastCodePending = true;
                                } else if (code) {
                                    lastCode = code;
                                    lastCodePending = true;
                                } else if (code == 0) {
                                    if (lastCodePending) {
                                        UpdateButtons(lastCode, true);
                                        lastCodePending = false;
                                    }
                                    UpdateButtons(lastCode, false);
                                }
                            } else {
                                lastCode = code;
                                lastCodePending = true;
                            }
                            lastCode = code;
                        }
                        else
                        {
                            LOG("Philips 1511: ignoring repeated code 0x%04X", code);
                        }
                        lastCodeFilled = true;
                    } else {
                        LOG("Philips 1511: REPORT_IN unexpected length (%d bytes)", size);
                    }
                } else if (status != HidDevice::E_ERR_TIMEOUT) {
                    LOG("Philips VOIP1511: error reading report");
                    hidDevice.Close();
                    devConnected = false;
                    lastCodeFilled = false;
                    lastCodePending = false;
                } else {
                    // timeout = nothing changes
                    if (lastCodePending) {
                        lastCodePending = false;
                        UpdateButtons(lastCode, true);
                    }
                }
            }
        }
        loopCnt++;
        Sleep(50);
    }

    // Philips 1511: apparently it is switching automatically to showing "VOIP151" text on disconnection.
    // Probably message 0x9E works as kind of a ping for this purpose.
    // No display clearing is needed.

    hidDevice.Close();
    exited = true;
    return 0;
}


int CommThreadStart(void) {
    DWORD dwtid;
    exited = false;
    connected = true;
    HANDLE CommThread = CreateThread(NULL, 0, CommThreadProc, /*this*/NULL, 0, &dwtid);
    if (CommThread == NULL) {
        connected = false;
        exited = true;
    }

    return 0;
}

int CommThreadStop(void) {
    connected = false;
    while (!exited) {
        Sleep(50);
    }
    return 0;
}

void UpdateRegistrationState(int state) {
    regState = state;
    //LOG("regState = %d", regState);
    displayUpdateFlag = true;
}

void UpdateCallState(int state, const char* display) {
    ScopedLock<Mutex> lock(mutexState);
    callState = state;
    callDisplay = display;
    displayUpdateFlag = true;
}

void UpdateRing(int state) {
    if (ringState != state) {
        ringState = state;
        ringUpdateFlag = true;
    }
    //LOG("ringState = %d", ringState);
}
