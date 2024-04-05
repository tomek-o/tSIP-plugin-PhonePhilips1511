#include "CommThread.h"
#include "..\tSIP\tSIP\phone\Phone.h"
#include "HidDevice.h"
#include "Log.h"
#include "CustomConf.h"
#include <windows.h>
#include <assert.h>
#include <time.h>

void Key(int keyCode, int state);

using namespace nsHidDevice;

namespace {
volatile bool connected = false;
volatile bool exited = false;

int VendorID = 0x0471;  // Philips
int ProductID = 0x2013;

int regState = 0;
int callState = 0;
int ringState = 0;
std::string callDisplay;
bool displayUpdateFlag = false;
bool ringUpdateFlag = false;

void UpdateButtons(uint16_t code, uint16_t lastCode) {
    LOG("Update buttons, raw code 0x%04X, previous: 0x%04X", code, lastCode);
    enum E_KEY key;
    uint16_t rawKey = code;
    if (rawKey == 0) {
        rawKey = lastCode;
    }
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
//    case 0x01:
//        key = KEY_C;
//        break;
    case 0x0040:
        key = KEY_UP;
        break;
    case 0x0080:
        key = KEY_DOWN;
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
    Key(key, code?1:0);
}


int UpdateDisplay(HidDevice &dev) {
    int status;

    displayUpdateFlag = false;

    unsigned char buffers[][4] = {
        { 0x10, 0x00, 0x00, 0x30 },
        { 0x50, 0x9A, 0x00, 0x01 },
        { 0x50, 0x89, 0x00, 0x00 },
        { 0x50, 0x8B, 0x01, 0x00 },
        { 0x50, 0x8C, 0x31, 0x30 }, // bytes 2, 3: ASCII hour
        { 0x50, 0x8C, 0x35, 0x30 }, // bytes 2, 3: ASCII minutes
        { 0x50, 0x8C, 0x30, 0x33 },
        { 0x50, 0x9E, 0x00, 0x04 },
    };

    time_t now = time(NULL);
    struct tm *tm_struct = localtime(&now);
    int hour = tm_struct->tm_hour;
    int minute = tm_struct->tm_min;

    buffers[4][2] = '0' + hour / 10;
    buffers[4][3] = '0' + hour % 10;
    buffers[5][2] = '0' + minute / 10;
    buffers[5][3] = '0' + minute % 10;

    for (unsigned int i=0; i<sizeof(buffers)/sizeof(buffers[0]); i++) {
        status = dev.WriteReport(HidDevice::E_REPORT_OUT, 0, buffers[i], sizeof(buffers[i]));
        if (status != 0)
            break;
        Sleep(10);
    }

    //
    if (status != 0) {
        LOG("Philips: UpdateDisplay status/error = %d", status);
    }
    return status;
}

int UpdateRing(HidDevice &dev) {
    int status;

    ringUpdateFlag = false;

    unsigned char sendbuf[] = { 0x50, 0x9C, 0x07, 0x8D };
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

};

DWORD WINAPI CommThreadProc(LPVOID data) {
    //ThreadComm *thrRead = (ThreadComm*)data;
    HidDevice hidDevice;
    bool devConnected = false;
    unsigned int loopCnt = 0;

    unsigned char rcvbuf[17];
    LOG("Running Philips VOIP1511 comm thread");
    bool lastCodeFilled = false;
    uint16_t lastCode = 0;

    while (connected) {
        if (devConnected == false) {
            if (loopCnt % 100 == 0) {
                int status = hidDevice.Open(VendorID, ProductID, NULL, NULL);
                if (status == 0) {
                    devConnected = true;
                    LOG("Philips VOIP1511 connected");
                    //LOG("  devConnected: %d", (int)devConnected);
                } else {
                    LOG("Error opening Philips VOIP1511: %s", HidDevice::GetErrorDesc(status).c_str());
                }
            }
        } else {
            int status = 0;
            if (callState == 0) {
                if ((loopCnt & 0x03) == 0) {
                    displayUpdateFlag = true;
                }
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
            } else
            {
                int size = sizeof(rcvbuf);
                //LOG("%03d  devConnected: %d, size = %d", __LINE__, (int)devConnected, size);
                int status = hidDevice.ReadReport(HidDevice::E_REPORT_IN, 0, (char*)rcvbuf, &size, 100);
                //LOG("%03d  devConnected: %d, size = %d", __LINE__, (int)devConnected, size);
                if (status == 0) {
                    if (size >= 5) {
                        LOG("Philips 1511: REPORT_IN received %d bytes: %02X %02X %02X %02X %02X", size, rcvbuf[0], rcvbuf[1], rcvbuf[2], rcvbuf[3], rcvbuf[4]);
                        uint16_t code = 256 * rcvbuf[1] + rcvbuf[2];
                        if (code != lastCode || !lastCodeFilled)
                        {
                            UpdateButtons(code, lastCode);
                            lastCode = code;
                        }
                        else
                        {
                            LOG("Philips 1511: ignoring repeated code 0x%04X", code);
                        }
                        lastCodeFilled = true;
                    } else {
                        LOG("Philips 1511: REPORT_IN too short");
                    }
                } else if (status != HidDevice::E_ERR_TIMEOUT) {
                    LOG("Philips VOIP1511: error reading report");
                    hidDevice.Close();
                    devConnected = false;
                    lastCodeFilled = false;
                }
            }
        }
        loopCnt++;
        Sleep(50);
    }

    // Philips 1511: apparently it is switching automatically to showing "VOIP151" text on disconnection
    // no display clearing is needed

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
