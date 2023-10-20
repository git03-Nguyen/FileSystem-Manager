#include "ReadSector.h"

DWORD readSector(std::string drive, int startSector, uint8_t sector[]) { // readPoint MUST be a multiple of 512
    if (drive.size() != 2 || drive[1] != ':') {
        SetLastError(3);
        throw "Đường dẫn ổ đĩa không hợp lệ!";
    }

    DWORD bytesRead = 0;
    HANDLE partition = NULL;

    std::wstring driveWstr = std::wstring(drive.begin(), drive.end());
    driveWstr = L"\\\\.\\"+ driveWstr;
    LPCWSTR lpcwstr = driveWstr.c_str();

    partition = CreateFile(lpcwstr,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    if (partition == INVALID_HANDLE_VALUE) { // cannot open drive
        throw "Không thể mở ổ đĩa!";
    }

    SetFilePointer(partition, startSector * _SECTOR_SIZE, NULL, FILE_BEGIN); //Set a Point to Read

    if (!ReadFile(partition, sector, _SECTOR_SIZE, &bytesRead, NULL)) {
        throw "Không thể đọc sector!";
    }

    CloseHandle(partition);

    return bytesRead;
}

FileSystem readFileSystemType(uint8_t bootSector[]) {
    char oemID[9] = { 0 };
    memcpy(oemID, bootSector + 3, 8);
    if (strcmp(oemID, "NTFS    ") == 0) return FileSystem::NTFS;

    char fatName[9] = { 0 };
    memcpy(fatName, bootSector + 0x52, 8);
    if (strcmp(fatName, "FAT32   ") == 0) return FileSystem::FAT32;

    return FileSystem::Others;
}

