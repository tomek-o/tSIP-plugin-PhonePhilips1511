#ifndef Philips1511H
#define Philips1511H

namespace Philips1511
{

int VendorID = 0x0471;  // Philips
int ProductID = 0x2013;

enum Cmd {
    CmdDispStart = 0x89,        // = 137
    CmdChar = 0x86,             // = 134
    CmdCharAtPos = 151,         // = 0x97; last character needs to be 0x00 (or 0x00 + 0x00 for even length)
    CmdTimeStartHHMMSS = 0x8A,  // ???
    CmdTimeStartHHMM = 0x8B,    // = 139; third byte parameter 0x01 -> hh:mm:ss, 17 -> "AM", 33 -> "PM"
    CmdTime = 0x8C,             // = 140;
    CmdRing = 0x9C,             // = 156, ON: { 0x50, 0x9C, 0x07 /* type */, 0x8D }, OFF: { 0x50, 0x9C, 0x00, 0x00 }
    CmdPingTwoSeconds = 0x9E,   // STK8018_WriteMCU(158, 1024); some kind of ping?
    CmdLed = 0x9A,              // = 154; 3rd byte: 0x01 => LED blinking fast, 0x04 =>  slower blink, 0x08 => mostly lit, short blink, 0x20 => steady light, 0x40 => steady light
};

}

#endif // Philips1511H
