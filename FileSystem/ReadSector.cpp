#include "ReadSector.h"

DWORD readSector(std::string drive, uint64_t byteOffset, uint8_t sector[]) { 
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

    SetFilePointer(partition, byteOffset, NULL, FILE_BEGIN); //Set a Point to Read - MUST be a multiple of 512

    if (!ReadFile(partition, sector, BOOT_SECTOR_SIZE, &bytesRead, NULL)) {
        throw "Không thể đọc sector!";
    }

    CloseHandle(partition);

    return bytesRead;
}

DWORD readCluster(std::string drive, uint64_t byteOffset, uint8_t cluster[], int clusterSize) {
    if (drive.size() != 2 || drive[1] != ':') {
		SetLastError(3);
		throw "Đường dẫn ổ đĩa không hợp lệ!";
	}

	DWORD bytesRead = 0;
	HANDLE partition = NULL;

	std::wstring driveWstr = std::wstring(drive.begin(), drive.end());
	driveWstr = L"\\\\.\\" + driveWstr;
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

	SetFilePointer(partition, byteOffset, NULL, FILE_BEGIN); //Set a Point to Read - MUST be a multiple of 512

	if (!ReadFile(partition, cluster, clusterSize, &bytesRead, NULL)) {
		throw "Không thể đọc cluster!";
	}

	CloseHandle(partition);

	return bytesRead;
}

FileSystem getFileSystemType(uint8_t bootSector[]) {
    char oemID[9] = { 0 };
    memcpy(oemID, bootSector + 3, 8);
    if (strcmp(oemID, "NTFS    ") == 0) return FileSystem::NTFS;

    char fatName[9] = { 0 };
    memcpy(fatName, bootSector + 0x52, 8);
    if (strcmp(fatName, "FAT32   ") == 0) return FileSystem::FAT32;

    return FileSystem::UNKNOWN;
}

std::vector<uint32_t> readClusterChainFat32(uint32_t cluster, std::string drive, FAT32_BS* bootSector) {
    // if cluster is 0 or 1, return empty vector
    if (cluster <= 1) return std::vector<uint32_t>();

    std::vector<uint32_t> clusterChain;
    clusterChain.push_back(cluster);

    int nextCluster = cluster;
    bool loop = true;
    while (loop) {
		uint32_t* sector = new uint32_t[bootSector->bytesPerSec / 4] { 0 };
        int entriesPerSector = bootSector->bytesPerSec / 4;

        int startSector = bootSector->rsvdSec + nextCluster / entriesPerSector;
        readSector(drive, startSector * bootSector->bytesPerSec, (uint8_t*)sector);

        while (true) {
            int offset = nextCluster % entriesPerSector;
            nextCluster = sector[offset] & 0x0FFFFFFF;
            if (nextCluster >= 0x0FFFFFF7 || nextCluster == 0) {
                loop = false; // break outer loop
                break;
            }
            clusterChain.push_back(nextCluster);
            int nextStartSector = bootSector->rsvdSec + nextCluster / entriesPerSector;
            if (nextStartSector != startSector) break; // break inner loop
        }
        delete[] sector;
	}

    return clusterChain;
}

std::string toHexString(uint8_t array[], int size) {
    std::stringstream ss;
    for (int i = 0; i < size; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)array[i] << " ";
    }
    return ss.str();
}

