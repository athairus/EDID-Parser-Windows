#include "edid.h"

int _tmain( int argc, _TCHAR *argv [] ) {

    // Identify the HMONITOR of interest via the callback MonitorFoundCallback
    EnumDisplayMonitors( NULL, NULL, MonitorFoundCallback, NULL );
    
    DISPLAY_DEVICE ddMon;
    
    if( DisplayDeviceFromHMonitor( g_hMonitor, ddMon ) == FALSE ) {
        return 1;
    }
    
    CString DeviceID;
    DeviceID.Format( _T( "%s" ), ddMon.DeviceID );
    DeviceID = Get2ndSlashBlock( DeviceID );
    
    short WidthMm = -1, HeightMm = -1;
    bool bFoundDevice = GetSizeForDevID( DeviceID, WidthMm, HeightMm );
    
    return !bFoundDevice;
    
}

BOOL CALLBACK MonitorFoundCallback( _In_ HMONITOR hMonitor, _In_ HDC hdcMonitor, _In_ LPRECT lprcMonitor, _In_ LPARAM dwData ) {

    // Use this function to identify the monitor of interest: MONITORINFO contains the Monitor RECT.
    MONITORINFOEX mi;
    mi.cbSize = sizeof( MONITORINFOEX );
    
    GetMonitorInfo( hMonitor, &mi );
    OutputDebugString( mi.szDevice );
    OutputDebugString( _T( "\n" ) );
    
    // For simplicity, we set the first monitor to be the one of interest
    if( g_hMonitor == NULL ) {
        g_hMonitor = hMonitor;
    }
    
    return TRUE;
    
}

BOOL DisplayDeviceFromHMonitor( HMONITOR hMonitor, DISPLAY_DEVICE &ddMonOut ) {

    MONITORINFOEX mi;
    mi.cbSize = sizeof( MONITORINFOEX );
    GetMonitorInfo( hMonitor, &mi );
    
    DISPLAY_DEVICE dd;
    dd.cb = sizeof( dd );
    DWORD devIdx = 0; // device index
    
    CString DeviceID;
    bool bFoundDevice = false;
    
    while( EnumDisplayDevices( 0, devIdx, &dd, 0 ) ) {
    
        devIdx++;
        
        if( 0 != _tcscmp( dd.DeviceName, mi.szDevice ) ) {
            continue;
        }
        
        DISPLAY_DEVICE ddMon;
        ZeroMemory( &ddMon, sizeof( ddMon ) );
        ddMon.cb = sizeof( ddMon );
        DWORD MonIdx = 0;
        
        while( EnumDisplayDevices( dd.DeviceName, MonIdx, &ddMon, 0 ) ) {
        
            MonIdx++;
            
            ddMonOut = ddMon;
            return TRUE;
            
            ZeroMemory( &ddMon, sizeof( ddMon ) );
            ddMon.cb = sizeof( ddMon );
            
        }
        
        ZeroMemory( &dd, sizeof( dd ) );
        dd.cb = sizeof( dd );
        
    }
    
    return FALSE;
    
}

CString Get2ndSlashBlock( const CString &sIn ) {

    int FirstSlash = sIn.Find( _T( '\\' ) );
    CString sOut = sIn.Right( sIn.GetLength() - FirstSlash - 1 );
    FirstSlash = sOut.Find( _T( '\\' ) );
    sOut = sOut.Left( FirstSlash );
    return sOut;
    
}

bool GetSizeForDevID( const CString &TargetDevID, short &WidthMm, short &HeightMm ) {

    HDEVINFO devInfo = SetupDiGetClassDevsEx(
                           &GUID_CLASS_MONITOR,             // Class GUID
                           NULL,                            // Enumerator
                           NULL,                            // HWND
                           DIGCF_PRESENT | DIGCF_PROFILE,   // Flags //DIGCF_ALLCLASSES|
                           NULL,                            // Device info, create a new one.
                           NULL,                            // Machine name, local machine
                           NULL );                          // Reserved
                           
    if( NULL == devInfo ) {
        return false;
    }
    
    bool bRes = false;
    
    for( ULONG i = 0; ERROR_NO_MORE_ITEMS != GetLastError(); ++i ) {
    
        SP_DEVINFO_DATA devInfoData;
        memset( &devInfoData, 0, sizeof( devInfoData ) );
        devInfoData.cbSize = sizeof( devInfoData );
        
        if( SetupDiEnumDeviceInfo( devInfo, i, &devInfoData ) ) {
        
            TCHAR Instance[MAX_DEVICE_ID_LEN];
            SetupDiGetDeviceInstanceId( devInfo, &devInfoData, Instance, MAX_PATH, NULL );
            
            CString sInstance( Instance );
            
            if( -1 == sInstance.Find( TargetDevID ) ) {
                continue;
            }
            
            HKEY hEDIDRegKey = SetupDiOpenDevRegKey( devInfo, &devInfoData,
                               DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ );
                               
            if( !hEDIDRegKey || ( hEDIDRegKey == INVALID_HANDLE_VALUE ) ) {
                continue;
            }
            
            bRes = GetMonitorSizeFromEDID( hEDIDRegKey, WidthMm, HeightMm );
            
            RegCloseKey( hEDIDRegKey );
            
        }
        
    }
    
    SetupDiDestroyDeviceInfoList( devInfo );
    return bRes;
    
}

bool GetMonitorSizeFromEDID( const HKEY hEDIDRegKey, short &WidthMm, short &HeightMm ) {

    DWORD dwType, AcutalValueNameLength = NAME_SIZE;
    TCHAR valueName[NAME_SIZE];
    
    BYTE EDIDdata[1024];
    DWORD edidsize = sizeof( EDIDdata );
    
    for( LONG i = 0, retValue = ERROR_SUCCESS; retValue != ERROR_NO_MORE_ITEMS; ++i ) {
        retValue = RegEnumValue( hEDIDRegKey, i, &valueName[0],
                                 &AcutalValueNameLength, NULL, &dwType,
                                 EDIDdata, // buffer
                                 &edidsize ); // buffer size
                                 
        if( retValue != ERROR_SUCCESS || 0 != _tcscmp( valueName, _T( "EDID" ) ) ) {
            continue;
        }
        
        WidthMm = ( ( EDIDdata[68] & 0xF0 ) << 4 ) + EDIDdata[66];
        HeightMm = ( ( EDIDdata[68] & 0x0F ) << 8 ) + EDIDdata[67];
        
        return true; // valid EDID found
    }
    
    return false; // EDID not found
    
}
