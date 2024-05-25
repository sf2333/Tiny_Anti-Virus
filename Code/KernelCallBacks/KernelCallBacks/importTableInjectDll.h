///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2016 - <company name here>
///
/// Original filename: importTableInjectDll.h
/// Project          : importTableInjectDll
/// Date of creation : <see importTableInjectDll.cpp>
/// Author(s)        : <see importTableInjectDll.cpp>
///
/// Purpose          : <see importTableInjectDll.cpp>
///
/// Revisions:         <see importTableInjectDll.cpp>
///
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifndef __IMPORTTABLEINJECTDLL_H_VERSION__
#define __IMPORTTABLEINJECTDLL_H_VERSION__ 100

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


#include "drvcommon.h"
#include "drvversion.h"


#define DEVICE_NAME			"\\Device\\IMPORTTABLEINJECTDLL_DeviceName"
#define SYMLINK_NAME		"\\DosDevices\\IMPORTTABLEINJECTDLL_DeviceName"
PRESET_UNICODE_STRING(usDeviceName, DEVICE_NAME);
PRESET_UNICODE_STRING(usSymlinkName, SYMLINK_NAME);

#ifndef FILE_DEVICE_IMPORTTABLEINJECTDLL
#define FILE_DEVICE_IMPORTTABLEINJECTDLL 0x8000
#endif

// Values defined for "Method"
// METHOD_BUFFERED
// METHOD_IN_DIRECT
// METHOD_OUT_DIRECT
// METHOD_NEITHER
// 
// Values defined for "Access"
// FILE_ANY_ACCESS
// FILE_READ_ACCESS
// FILE_WRITE_ACCESS

const ULONG IOCTL_IMPORTTABLEINJECTDLL_OPERATION = CTL_CODE(FILE_DEVICE_IMPORTTABLEINJECTDLL, 0x01, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA);

#endif // __IMPORTTABLEINJECTDLL_H_VERSION__
