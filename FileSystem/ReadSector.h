#pragma once

#include <Windows.h>
#include <cstdint>

#define _SECTOR_SIZE 512

enum class FileSystem {FAT32, NTFS, Others};

DWORD readSector(LPCWSTR drive, int readPoint, uint8_t sector[]);

FileSystem readFileSystemType(uint8_t bootSector[]);
