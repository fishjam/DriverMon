// DriverLog.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "..\DriverMonitor\Common.h"

using namespace std;

int Error(DWORD code = GetLastError()) {
    printf("Error: %d\n", ::GetLastError());
    return 1;
}

PVOID driver;

bool AddDriver(HANDLE hFile, PCWSTR name) {
    DWORD returned;
    if (::DeviceIoControl(hFile, static_cast<DWORD>(DriverMonIoctls::AddDriver), (PVOID)name, (DWORD)wcslen(name) * 2 + 2,
        &driver, sizeof(driver), &returned, nullptr)) {
        return true;
    }
    Error();
    return false;
}

void DisplayIrpArrivedDetails(const IrpArrivedInfo& info) {
    cout << "IRP sent: PID: " << info.ProcessId << " TID: " << info.ThreadId << " Major: " << (int)info.MajorFunction;
    cout << " Minor: " << (int)info.MinorFunction << " Device: " << info.DeviceObject << " Driver: " << info.DriverObject;
    cout << endl;

    bool read = false;

    switch (info.MajorFunction) {
    case IrpMajorCode::DEVICE_CONTROL:
    case IrpMajorCode::INTERNAL_DEVICE_CONTROL:
        cout << "\tIoctl: 0x" << hex << info.DeviceIoControl.IoControlCode << " Input len: " << dec
            << info.DeviceIoControl.InputBufferLength << " Output len: " << info.DeviceIoControl.OutputBufferLength << endl;
        break;

    case IrpMajorCode::READ:
        read = true;

    case IrpMajorCode::WRITE:
        cout << (read ? "\tRead" : "\tWrite") << " Length: " << info.Write.Length << " Offset: " << info.Write.Offset;
        cout << endl;
        break;
    }
}

void DisplayDetails(const CommonInfoHeader* header) {
    DisplayIrpArrivedDetails(*reinterpret_cast<const IrpArrivedInfo*>(header));
}

int wmain(int argc, const wchar_t* argv[]) {
    HANDLE hFile = ::CreateFile(FileDeviceSymLink, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return Error();

    DWORD returned;
    HANDLE hEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    ::DeviceIoControl(hFile, static_cast<DWORD>(DriverMonIoctls::SetEventHandle), &hEvent, sizeof(hEvent), nullptr, 0, &returned, nullptr);

    for (int i = 1; i < argc; i++) {
        if (!AddDriver(hFile, argv[i])) {
            wcout << "Failed to add driver " << argv[i] << endl;
        }
        else {
            wcout << "Successfully added driver " << argv[i] << endl;
        }
    }

    auto size = 1 << 16;
    auto buffer = std::make_unique<BYTE[]>(size);

    for (;;) {
        ::WaitForSingleObject(hEvent, INFINITE);
        if (!::DeviceIoControl(hFile, static_cast<DWORD>(DriverMonIoctls::GetData), nullptr, 0, buffer.get(), size, &returned, nullptr)) {
            Error();
            continue;
        }

        for (DWORD i = 0; i < returned; ) {
            const auto data = reinterpret_cast<CommonInfoHeader*>(buffer.get() + i);
            DisplayDetails(data);
            i += data->Size;
        }
    }

    ::CloseHandle(hEvent);
    ::CloseHandle(hFile);

    return 0;
}

