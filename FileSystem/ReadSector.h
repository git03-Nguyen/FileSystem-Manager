#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>

#include "FAT32_NTFS.h"

#define _SECTOR_SIZE 512

enum class FileSystem {FAT32, NTFS, Others};

DWORD readSector(std::string drive, int readPoint, uint8_t sector[]);

FileSystem readFileSystemType(uint8_t bootSector[]);

std::vector<uint32_t> readClusterChainFat32(uint32_t cluster, std::string drive, FAT32_BS* bootSector);