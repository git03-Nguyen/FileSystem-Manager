#pragma once

#include <Windows.h>

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#include "FAT32_NTFS.h"

// read a sector from a drive
DWORD readByte(std::string drive, uint64_t byteOffset, uint8_t sector[], int sizeToRead = BOOT_SECTOR_SIZE);

// read a cluster from a drive
DWORD readCluster(std::string drive, uint64_t byteOffset, uint8_t cluster[], int clusterSize);

// get the file system type of a drive
FileSystem getFileSystemType(uint8_t bootSector[]);

// read the cluster chain of a file/folder
std::vector<uint32_t> readClusterChainFat32(uint32_t cluster, std::string drive, FAT32_BS* bootSector);

// helper function to convert a uint8_t array to a hex string
std::string toHexString(uint8_t array[], int size = 1);