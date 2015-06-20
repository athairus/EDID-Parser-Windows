// Credit: http://ofekshilon.com/2014/06/19/reading-specific-monitor-dimensions/

#pragma once

#include <atlstr.h>
#include <SetupApi.h>
#include <cfgmgr32.h> // for MAX_DEVICE_ID_LEN
#pragma comment(lib, "setupapi.lib")

#define NAME_SIZE 128

const GUID GUID_CLASS_MONITOR = { 0x4d36e96e, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 };

HMONITOR g_hMonitor = NULL;

int _tmain( int argc, _TCHAR *argv [] );

// Callback called when EnumDisplayMonitors() finds a monitor to process
BOOL CALLBACK MonitorFoundCallback( _In_ HMONITOR hMonitor, _In_ HDC hdcMonitor, _In_ LPRECT lprcMonitor, _In_ LPARAM dwData );

// Given a handle to a monitor, get a DISPLAY_DEVICE struct (which contains the DeviceID)
BOOL DisplayDeviceFromHMonitor( HMONITOR hMonitor, DISPLAY_DEVICE &ddMonOut );

// Get the second slash block, used to match DeviceIDs to instances, example:
// DeviceID: MONITOR\GSM4B85\{4d36e96e-e325-11ce-bfc1-08002be10318}\0011
// Instance: DISPLAY\GSM4B85\5&273756F2&0&UID1048833
CString Get2ndSlashBlock( const CString &sIn );

// Take a device instance ID that points to a monitor and write its dimensions to arguments 2 and 3
bool GetSizeForDevID( const CString &TargetDevID, short &WidthMm, short &HeightMm );

// Parse EDID from given registry key to get width and height parameters, assumes hEDIDRegKey is valid
bool GetMonitorSizeFromEDID( const HKEY hEDIDRegKey, short &WidthMm, short &HeightMm );
