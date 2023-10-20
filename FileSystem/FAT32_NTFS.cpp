#include "FAT32_NTFS.h"

std::string toHexString(uint8_t array[], int size) {
	std::stringstream ss;
	for (int i = 0; i < size; i++) {
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)array[i] << " ";
	}
	return ss.str();
}
