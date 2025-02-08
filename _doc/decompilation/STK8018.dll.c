/* This file was generated mostly by the Hex-Rays decompiler version 8.4.0.240320.
   Copyright (c) 2007-2021 Hex-Rays <info@hex-rays.com>
*/

#include <windows.h>

/*
VOIP151.exe startup:
    STK8018_CustomerID(1137, 8211);
    if ( a2 )
    {
      v2 = unknown_libname_80(this);
      STK8018_Attach(v2);
    }
    STK8018_SetMonitor(18);
    STK8018_SetGPO(768, 255);
    STK8018_SetGPO(768, 0);
    Sleep(0x3E8u);
    STK8018_SetKeyscan(1);
    STK8018_SetDebouncePeriod(80);
    STK8018_SetStartRepeatInterval(500);
    STK8018_SetRepeatInterval(500);
    LOBYTE(v3) = STK8018_GetAudio();
    STK8018_SetAudio(v3)
*/

const WCHAR Name = 0u; // idb
__int16 word_10004024 = 1505; // weak
__int16 word_10004026 = 8192; // weak
__int16 word_1000402C = 1505; // weak
__int16 word_1000402E = 8208; // weak
char mcuValue = '\x80'; // weak
int startRepeatIntervalValue = -1; // weak
int repeatIntervalValue = -1; // weak
int debouncePeriodValue = 50; // weak
DWORD dwMilliseconds = 100u; // idb
char byte_10004050 = '\0'; // weak
char readFromHidValue = '\0'; // weak
__int16 word_10004059 = 0; // weak
__int16 word_1000405B = 0; // weak
int dword_10004064 = 0; // weak

__int16 usbVidArray[] = { 0 }; // weak
__int16 usbPidArray[] = { 0 }; // weak
__int16 customerIdSetCounter = 0; // weak
HANDLE HidDeviceObject = NULL; // idb
LPSECURITY_ATTRIBUTES hThread = NULL; // idb
HANDLE hFile = NULL; // idb
HANDLE hEvent = NULL; // idb
int HidVendorIdValue = 0; // weak
int HidDeviceIdValue = 0; // weak
HWND hWnd = NULL; // idb
int deviceIsValid = 0; // weak
int keyscanValue = 0; // weak
int buzzerValue = 0; // weak
char audioValue = '\0'; // weak
char gpoValue2 = '\0'; // weak
char gpoValue1 = '\0'; // weak
char gpoValueHigh = '\0'; // weak
int monitorValue = 0; // weak
__int16 word_1000415C = 0; // weak
int dword_10004160 = 0; // weak
int lastAppMessageLParam = 0; // weak
int dword_10004168 = 0; // weak
int dword_1000416C = 0; // weak
int repeatTickCount = 0; // weak
int dword_10004178 = 0; // weak

int OpenHidDevice()
{
  HDEVINFO ClassDevsW; // ebp
  struct _SP_DEVICE_INTERFACE_DETAIL_DATA_W *v1; // esi
  int v2; // ecx
  __int16 *v3; // eax
  int result; // eax
  char v5 = 0; // [esp+Ah] [ebp-4Eh]
  char v6 = 0; // [esp+Bh] [ebp-4Dh]
  DWORD MemberIndex = 0; // [esp+Ch] [ebp-4Ch]
  DWORD RequiredSize; // [esp+10h] [ebp-48h] BYREF
  HDEVINFO v9; // [esp+14h] [ebp-44h]
  PHIDP_PREPARSED_DATA PreparsedData; // [esp+18h] [ebp-40h] BYREF
  DWORD v11; // [esp+1Ch] [ebp-3Ch] BYREF
  struct _HIDD_ATTRIBUTES Attributes; // [esp+20h] [ebp-38h] BYREF
  GUID HidGuid; // [esp+2Ch] [ebp-2Ch] BYREF
  struct _SP_DEVICE_INTERFACE_DATA DeviceInterfaceData; // [esp+3Ch] [ebp-1Ch] BYREF

  deviceIsValid = 0;
  HidVendorIdValue = 0;
  HidDeviceIdValue = 0;
  HidD_GetHidGuid(&HidGuid);
  ClassDevsW = SetupDiGetClassDevsW(&HidGuid, 0, 0, 0x12u);
  v9 = ClassDevsW;
  DeviceInterfaceData.cbSize = 28;
  do
  {
    if ( !SetupDiEnumDeviceInterfaces(ClassDevsW, 0, &HidGuid, MemberIndex, &DeviceInterfaceData) )
    {
      v6 = 1;
      goto LABEL_26;
    }
    SetupDiGetDeviceInterfaceDetailW(ClassDevsW, &DeviceInterfaceData, 0, 0, &RequiredSize, 0);
    v1 = (struct _SP_DEVICE_INTERFACE_DETAIL_DATA_W *)malloc(RequiredSize);
    v1->cbSize = 6;
    SetupDiGetDeviceInterfaceDetailW(ClassDevsW, &DeviceInterfaceData, v1, RequiredSize, &v11, 0);
    HidDeviceObject = CreateFileW(v1->DevicePath, 0xC0000000, 3u, 0, 3u, 0, 0);
    Attributes.Size = 12;
    if ( !HidD_GetAttributes(HidDeviceObject, &Attributes) )
    {
LABEL_23:
      CloseHandle(HidDeviceObject);
      HidDeviceObject = 0;
LABEL_24:
      free(v1);
      goto LABEL_26;
    }
    HidVendorIdValue = Attributes.VendorID;
    HidDeviceIdValue = Attributes.ProductID;
    if ( Attributes.VendorID == word_1000402C && Attributes.ProductID == word_1000402E
      || Attributes.VendorID == word_10004024 && Attributes.ProductID == word_10004026 )
    {
      goto LABEL_17;
    }
    if ( !v5 )
    {
      if ( !customerIdSetCounter )
        goto LABEL_23;
      v2 = 0;
      v3 = usbPidArray;
      while ( HidVendorIdValue != (unsigned __int16)*(v3 - 1) || HidDeviceIdValue != (unsigned __int16)*v3 )
      {
        ++v2;
        v3 += 3;
        if ( v2 >= (unsigned __int16)customerIdSetCounter )
        {
          ClassDevsW = v9;
          goto LABEL_23;
        }
      }
      ClassDevsW = v9;
LABEL_17:
      v5 = 1;
    }
    HidD_GetPreparsedData(HidDeviceObject, &PreparsedData);
    HidP_GetCaps(PreparsedData, &Capabilities);
    if ( Capabilities.InputReportByteLength == 5 && Capabilities.OutputReportByteLength == 5 )
      deviceIsValid = 1;
    HidD_FreePreparsedData(PreparsedData);
    hFile = CreateFileW(v1->DevicePath, 0xC0000000, 3u, 0, 3u, 0x40000000u, 0);
    if ( hEvent )
      goto LABEL_24;
    hEvent = CreateEventW(0, 1, 0, &Name);
    Overlapped.hEvent = hEvent;
    Overlapped.8 = 0LL;
    free(v1);
LABEL_26:
    ++MemberIndex;
  }
  while ( !v6 && !deviceIsValid );
  SetupDiDestroyDeviceInfoList(ClassDevsW);
  result = deviceIsValid;
  if ( deviceIsValid )
  {
    dword_10004160 = 0;
    dword_1000416C = 0;
    lastAppMessageLParam = 0;
    dword_10004168 = 0;
    buzzerValue = 0;
    if ( hWnd )
    {
      PostMessageW(hWnd, 0x464u, 0, 1);
      return deviceIsValid;
    }
  }
  return result;
}

int CloseHidDeviceAndCleanup()
{
  if ( hFile )
  {
    CloseHandle(hFile);
    hFile = 0;
  }
  if ( HidDeviceObject )
  {
    CloseHandle(HidDeviceObject);
    HidDeviceObject = 0;
  }
  if ( deviceIsValid )
  {
    deviceIsValid = 0;
    if ( hWnd )
      PostMessageW(hWnd, 0x464u, 0, 0);
  }
  return 1;
}

char __cdecl GenerateAppKeyEvent(_BYTE *a1, int a2)
{
  int v4; // edx
  int v5; // ebx
  unsigned __int16 v6; // bx
  char result; // al

  DWORD TickCount = GetTickCount();
  unsigned int timeDelta = (TickCount - repeatTickCount) / 100; // unit = 100 ms
  dword_10004178 = TickCount;
  if ( timeDelta > 0xFF )
    timeDelta = 255;
  if ( a2 )
  {
    v4 = dword_10004160 | (timeDelta << 24);
    v5 = dword_10004160;
  }
  else
  {
    HIBYTE(v6) = a1[1];
    LOBYTE(v6) = a1[3];
    v5 = (unsigned __int8)a1[2] | (v6 << 8);
    v4 = v5 | (timeDelta << 24);
  }
  result = monitorValue;
  if ( (monitorValue & 2) == 0 )
  {
    if ( (monitorValue & 1) == 0 )
      goto LABEL_26;
    if ( !a2 )
    {
      dword_10004064 = startRepeatIntervalValue;
      repeatTickCount = TickCount;
      goto LABEL_24;
    }
    goto LABEL_15;
  }
  if ( a2 )
  {
    if ( dwMilliseconds == debouncePeriodValue )
    {
      result = startRepeatIntervalValue;
      dword_10004064 = startRepeatIntervalValue;
      repeatTickCount = TickCount;
      if ( dword_10004160 )
        goto LABEL_24;
      goto LABEL_26;
    }
LABEL_15:
    if ( dword_10004160 )
    {
      if ( startRepeatIntervalValue != -1 )
      {
        result = repeatIntervalValue;
        if ( repeatIntervalValue != -1 && TickCount - repeatTickCount >= dword_10004064 )
        {
          dword_10004064 += repeatIntervalValue;
          if ( hWnd )
          {
            result = BYTE2(v4);
            if ( BYTE2(v4) != 8 && BYTE2(v4) != 4 )
            {
              result = PostMessageW(hWnd, 0x465u, 0, v4);
              dword_10004160 = v5;
              return result;
            }
          }
        }
      }
    }
    goto LABEL_26;
  }
  if ( !v5 )
  {
LABEL_24:
    result = (char)hWnd;
    if ( hWnd )
      result = PostMessageW(hWnd, 0x465u, 0, v4);
  }
LABEL_26:
  dword_10004160 = v5;
  return result;
}

HWND __cdecl PostWindowsAppMessage(_BYTE *a1, HWND a2)
{
  HWND result = a2;
  if ( !a2 )
  {
    unsigned __int16 v3; // cx
    HIBYTE(v3) = a1[2];
    LOBYTE(v3) = a1[4];
    result = (HWND)monitorValue;
    int lParam = (unsigned __int8)a1[3] | (v3 << 8);
    if ( (monitorValue & 0x100) != 0 || (monitorValue & 0x200) != 0 && lParam != lastAppMessageLParam )
    {
      result = hWnd;
      if ( hWnd )
        result = (HWND)PostMessageW(hWnd, 0x467u, 0, lParam);
    }
    lastAppMessageLParam = lParam;
  }
  return result;
}

char __cdecl HandleHidReadTimeout(int a1, HWND a2)
{
  int v2; // edx
  unsigned __int16 v3; // cx
  char result; // al
  int v5; // ecx
  unsigned __int16 v6; // dx

  result = a1;
  if ( !*(_BYTE *)a1 )
  {
    v5 = *(_BYTE *)(a1 + 1) & 0xC0;
    if ( (*(_BYTE *)(a1 + 1) & 0xC0) != 0 )
    {
      if ( v5 == 64 )
      {
        return (unsigned __int8)PostWindowsAppMessage((_BYTE *)a1, a2);
      }
      else if ( v5 == 192 )
      {
        result = (char)a2;
        if ( !a2 )
        {
          v2 = *(unsigned __int8 *)(a1 + 2);
          v3 = *(_WORD *)(a1 + 3);
          ++word_1000415C;
          result = v2;
          dword_10004168 = v2 | (v3 << 8);
        }
      }
    }
    else if ( keyscanValue )
    {
      return GenerateAppKeyEvent((_BYTE *)a1, (int)a2);
    }
    else if ( a2 )
    {
      return dword_1000416C;
    }
    else
    {
      HIBYTE(v6) = *(_BYTE *)(a1 + 4);
      LOBYTE(v6) = *(_BYTE *)(a1 + 2);
      result = v6;
      dword_1000416C = v6;
    }
  }
  return result;
}

// Function used inside main thread and other functions for reading
void ReadFromHidDevice()
{
  __int16 v2; // ax
  int v3; // ecx
  DWORD NumberOfBytesRead; // [esp+0h] [ebp-Ch] BYREF

  char Buffer; // [esp+4h] [ebp-8h] BYREF
  __int16 v6; // [esp+5h] [ebp-7h]
  unsigned __int16 v7; // [esp+7h] [ebp-5h]
  // end of Buffer

  if ( deviceIsValid || OpenHidDevice() )
  {
    EnterCriticalSection(&CriticalSection);
    BOOL readOk = ReadFile(hFile, &Buffer, Capabilities.InputReportByteLength, &NumberOfBytesRead, &Overlapped);
    DWORD waitResult = WaitForSingleObject(hEvent, dwMilliseconds);
    ResetEvent(hEvent);
    LeaveCriticalSection(&CriticalSection);
    if ( readOk || GetLastError() != ERROR_DEVICE_NOT_CONNECTED )
    {
      if ( waitResult )
      {
        if ( waitResult == WAIT_TIMEOUT )
        {
          HandleHidReadTimeout((int)&readFromHidValue, (HWND)1);
          dwMilliseconds = 100;
        }
      }
      else               // WAIT_OBJECT_0 = 0, signaled
      {
        v2 = v6;
        if ( !Buffer )
        {
          v3 = v6 & 0xC0;
          if ( (v6 & 0xC0) != 0 )
          {
            if ( v3 == 64 )
            {
              PostWindowsAppMessage(&Buffer, 0);
              v2 = v6;
            }
            else if ( v3 == 192 )
            {
              ++word_1000415C;
              dword_10004168 = HIBYTE(v6) | (v7 << 8);
            }
          }
          else if ( keyscanValue )
          {
            GenerateAppKeyEvent(&Buffer, 0);
            v2 = v6;
          }
          else
          {
            dword_1000416C = HIBYTE(v6) | (HIBYTE(v7) << 8);
          }
        }
        word_10004059 = v2;
        readFromHidValue = Buffer;
        word_1000405B = v7;
        dwMilliseconds = debouncePeriodValue;
      }
    }
    else
    {
      CloseHidDeviceAndCleanup();
    }
  }
  else
  {
    Sleep(250);
  }
}

void __stdcall __noreturn MainThread(LPVOID lpThreadParameter)
{
  while ( 1 )
    ReadFromHidDevice();
}

/*
Philips VOIP151: VID 0x0471 = 1137, PID 0x2013 = 8211 (this is the one I have)
or
Philips VOIP150: VID 0x0471 = 1137, PID 0x2014
*/
int __stdcall STK8018_CustomerID(__int16 UsbVid, __int16 UsbPid)
{
  __int16 v2; // cx
  int v4; // eax

  v2 = customerIdSetCounter;
  if ( (unsigned __int16)customerIdSetCounter >= 0xAu )
    return 0;
  v4 = 3 * (unsigned __int16)customerIdSetCounter;
  usbVidArray[v4] = UsbVid;
  usbPidArray[v4] = UsbPid;
  customerIdSetCounter = v2 + 1;
  return 1;
}

int __stdcall STK8018_GetDeviceVendorID()
{
  return HidVendorIdValue;
}

int __stdcall STK8018_GetDeviceProductID()
{
  return HidDeviceIdValue;
}

BOOL __stdcall STK8018_Attach(HWND ThreadId)
{
  struct _SECURITY_ATTRIBUTES *Thread;

  hWnd = ThreadId;
  Thread = hThread;
  deviceIsValid = 0;
  if ( !hThread )
  {
    Thread = (struct _SECURITY_ATTRIBUTES *)CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, 0, 0, (LPDWORD)&ThreadId);
    hThread = Thread;
  }
  return Thread != 0;
}

int __stdcall STK8018_Unload()
{
  if ( hThread )
  {
    TerminateThread(hThread, 0);
    hThread = 0;
  }
  CloseHidDeviceAndCleanup();
  hWnd = 0;
  return 1;
}

/* VOIP151.exe: STK8018_SetMonitor(18 = 0x12) */
__int16 __stdcall STK8018_SetMonitor(__int16 a1)
{
  __int16 result = a1;
  LOWORD(monitorValue) = a1;
  return result;
}

__int16 __stdcall STK8018_GetMonitor()
{
  return monitorValue;
}

HANDLE WriteToHidDevice()
{
  HANDLE result; // eax
  DWORD NumberOfBytesWritten; // [esp+0h] [ebp-Ch] BYREF
  char Buffer[8]; // [esp+4h] [ebp-8h] BYREF

  Buffer[0] = 0;
  Buffer[2] = gpoValue2;
  result = HidDeviceObject;
  Buffer[1] = audioValue | (keyscanValue != 1 ? 0 : 0x10) | (buzzerValue != 1 ? 0 : 0x20);
  Buffer[3] = gpoValue1;
  Buffer[4] = gpoValueHigh;
  if ( HidDeviceObject )
  {
    result = (HANDLE)WriteFile(HidDeviceObject, Buffer, Capabilities.OutputReportByteLength, &NumberOfBytesWritten, 0);
    if ( !result )
    {
      result = (HANDLE)GetLastError();
      if ( result == (HANDLE)ERROR_DEVICE_NOT_CONNECTED )
        return (HANDLE)CloseHidDeviceAndCleanup();
    }
  }
  return result;
}

int __stdcall STK8018_SetKeyscan(int a1)
{
  keyscanValue = a1;
  repeatTickCount = GetTickCount();
  WriteToHidDevice();
  return keyscanValue;
}

int __stdcall STK8018_GetKeyscan()
{
  return keyscanValue;
}

int __stdcall STK8018_SetBuzzer(int a1)
{
  buzzerValue = a1;
  WriteToHidDevice();
  return buzzerValue;
}

int __stdcall STK8018_GetBuzzer()
{
  return buzzerValue;
}

char __stdcall STK8018_SetAudio(char a1)
{
  audioValue = a1 & 0xE;
  WriteToHidDevice();
  return audioValue;
}

char __stdcall STK8018_GetAudio()
{
  return audioValue;
}

/* STK8018_WriteAudio(6, 33); */
char __stdcall STK8018_WriteAudio(char a1, DWORD NumberOfBytesWritten)
{
  char result; // al
  char Buffer[8]; // [esp+8h] [ebp-8h] BYREF

  Buffer[0] = 0;
  Buffer[4] = 0;
  result = audioValue;
  Buffer[1] = (audioValue + 1) | (keyscanValue != 1 ? 0 : 0x10) | (buzzerValue != 1 ? 0 : 0x20);
  Buffer[2] = NumberOfBytesWritten;
  Buffer[3] = a1;
  if ( HidDeviceObject )
  {
    if ( !WriteFile(HidDeviceObject, Buffer, Capabilities.OutputReportByteLength, &NumberOfBytesWritten, 0)
      && GetLastError() == ERROR_DEVICE_NOT_CONNECTED )
    {
      CloseHidDeviceAndCleanup();
    }
    return audioValue;
  }
  return result;
}

/* VOIP151.exe:
   startup:
    STK8018_SetGPO(768, 255);
    STK8018_SetGPO(768, 0);
*/
HANDLE __stdcall STK8018_SetGPO(unsigned int a1, __int16 a2)
{
  gpoValue2 = a2;
  gpoValue1 = a1;
  keyscanValue = 0;
  gpoValueHigh = (a1 >> 4) ^ ((a1 >> 4) ^ HIBYTE(a2)) & 0xF;
  return WriteToHidDevice();
}

/* VOIP151.exe: unused? */
char __stdcall STK8018_SetMCU(DWORD NumberOfBytesWritten)
{
  char Buffer[8]; // [esp+0h] [ebp-8h] BYREF

  switch ( (unsigned __int16)NumberOfBytesWritten )
  {
    case 0xFu:
      mcuValue = 0x80;
      break;
    case 0x1Eu:
      mcuValue = -112;
      break;
    case 0x3Cu:
      mcuValue = -96;
      break;
  }
  Buffer[1] = mcuValue;
  Buffer[0] = 0;
  Buffer[4] = 0;
  if ( HidDeviceObject
    && !WriteFile(HidDeviceObject, Buffer, Capabilities.OutputReportByteLength, &NumberOfBytesWritten, 0)
    && GetLastError() == ERROR_DEVICE_NOT_CONNECTED )
  {
    CloseHidDeviceAndCleanup();
  }
  WriteToHidDevice();
  return mcuValue;
}

__int16 __stdcall STK8018_WriteMCU(DWORD NumberOfBytesWritten, __int16 a2)
{
  __int16 v2; // bx

  char Buffer[3]; // [esp+4h] [ebp-8h] BYREF
  __int16 v5; // [esp+7h] [ebp-5h]
  // end of Buffer

  v2 = a2;
  Buffer[0] = 0;
  v5 = a2;
  Buffer[1] = (audioValue + 0x40) | (keyscanValue != 1 ? 0 : 0x10) | (buzzerValue != 1 ? 0 : 0x20);
  Buffer[2] = NumberOfBytesWritten;
  if ( HidDeviceObject
    && !WriteFile(HidDeviceObject, Buffer, Capabilities.OutputReportByteLength, &NumberOfBytesWritten, 0)
    && GetLastError() == ERROR_DEVICE_NOT_CONNECTED )
  {
    CloseHidDeviceAndCleanup();
  }
  Sleep(10);
  return v2;
}

/*
Ghidra:

undefined2 _STK8018_WriteMCU_8(undefined param_1,undefined2 param_2)

{
  undefined2 uVar1;
  BOOL BVar2;
  DWORD DVar3;
  undefined local_8;
  byte local_7;
  undefined local_6;
  undefined local_5;
  undefined local_4;

                    // 0x1b00  25  _STK8018_WriteMCU@8
  uVar1 = param_2;
  local_8 = 0;
  local_5 = (undefined)param_2;
  local_4 = (undefined)((ushort)param_2 >> 8);
  local_7 = (DAT_10004150 != 1) - 1U & 0x20 | (DAT_1000414c != 1) - 1U & 0x10 | DAT_10004154 + 0x40U
  ;
  local_6 = param_1;
  if (DAT_1000412c != (HANDLE)0x0) {
    BVar2 = WriteFile(DAT_1000412c,&local_8,(uint)DAT_10004086,(LPDWORD)&param_1,(LPOVERLAPPED)0x0);
    if (BVar2 == 0) {
      DVar3 = GetLastError();
      if (DVar3 == 0x48f) {
        FUN_100012c0();
      }
    }
  }
  Sleep(10);
  return uVar1;
}
*/

__int16 __stdcall STK8018_WriteEEP(DWORD NumberOfBytesWritten, __int16 a2)
{
  __int16 v2; // bx

  char Buffer[2]; // [esp+4h] [ebp-8h] BYREF
  __int16 v5; // [esp+6h] [ebp-6h]
  char v6; // [esp+8h] [ebp-4h]
  // end of buffer

  Buffer[1] = mcuValue;
  v2 = a2;
  Buffer[0] = 0;
  v5 = a2;
  v6 = NumberOfBytesWritten | 0xC0;
  if ( HidDeviceObject
    && !WriteFile(HidDeviceObject, Buffer, Capabilities.OutputReportByteLength, &NumberOfBytesWritten, 0)
    && GetLastError() == ERROR_DEVICE_NOT_CONNECTED )
  {
    CloseHidDeviceAndCleanup();
  }
  ReadFromHidDevice();
  buzzerValue = 0;
  WriteToHidDevice();
  return v2;
}

__int16 __stdcall STK8018_ReadEEP(DWORD NumberOfBytesWritten)
{
  __int16 result; // ax
  char Buffer[8]; // [esp+0h] [ebp-8h] BYREF

  result = -2;
  word_1000415C = 0;
  dword_10004168 = 65534;
  if ( deviceIsValid )
  {
    Buffer[1] = mcuValue;
    Buffer[0] = 0;
    Buffer[4] = NumberOfBytesWritten | 0x80;
    if ( HidDeviceObject
      && !WriteFile(HidDeviceObject, Buffer, Capabilities.OutputReportByteLength, &NumberOfBytesWritten, 0)
      && GetLastError() == ERROR_DEVICE_NOT_CONNECTED )
    {
      CloseHidDeviceAndCleanup();
    }
    Sleep(150);
    for ( unsigned int i = 0; i < 10; ++i )
    {
      if ( word_1000415C )
        break;
      ReadFromHidDevice();
    }
    for ( unsigned int j = 0; j < 10; ++j )
    {
      if ( word_1000415C )
        break;
      Sleep(150);
      ReadFromHidDevice();
    }
    buzzerValue = 0;
    WriteToHidDevice();
    return dword_10004168;
  }
  return result;
}

int __stdcall STK8018_GetDebouncePeriod()
{
  return debouncePeriodValue & 0x7FFF;
}

int __stdcall STK8018_SetDebouncePeriod(int period)
{
  int v1 = period;
  if ( period >= 40 )
  {
    if ( a1 > 100 )
      v1 = 100;
    debouncePeriodValue = v1;
    return v1 & 0x7FFF;
  }
  else
  {
    debouncePeriodValue = 40;
    return 40;
  }
}

int __stdcall STK8018_GetStartRepeatInterval()
{
  return startRepeatIntervalValue;
}

int __stdcall STK8018_SetStartRepeatInterval(int a1)
{
  int result = a1;
  if ( a1 > 0 )
  {
    if ( a1 < 100 )
      result = 100;
    startRepeatIntervalValue = result;
  }
  else
  {
    result = -1;
    startRepeatIntervalValue = -1;
  }
  return result;
}

int __stdcall STK8018_GetRepeatInterval()
{
  return repeatIntervalValue;
}

int __stdcall STK8018_SetRepeatInterval(int interval)
{
  int result = interval;
  if ( interval > 0 )
  {
    if ( interval < 100 )
      result = 100;
    repeatIntervalValue = result;
  }
  else
  {
    result = -1;
    repeatIntervalValue = -1;
  }
  return result;
}
