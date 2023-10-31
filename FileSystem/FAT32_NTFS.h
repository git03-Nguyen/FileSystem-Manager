#pragma once
#include <cstdint>

// Default length of (Partition) Boot Sector
#define BOOT_SECTOR_SIZE 512

// File system types
enum class FileSystem { FAT32, NTFS, UNKNOWN };

// Structure of a FAT32 boot sector
#pragma pack(push, 1)
struct FAT32_BS {
	uint8_t jmpBoot[3];	// 0x00
	char OEM_id[8];		// 0x03
	uint16_t bytesPerSec; // 0x0B
	uint8_t secPerClus;	// 0x0D
	uint16_t rsvdSec;	// 0x0E
	uint8_t numFATs;	// 0x10
	uint16_t RDETcnt16;	// 0x11
	uint16_t numSecs16;	// 0x13
	uint8_t media;		// 0x15 // val 0xF8 for hard disk
	uint16_t secPerFAT16; // 0x16
	uint16_t secPerTrk;	// 0x18
	uint16_t numHeads;	// 0x1A
	uint32_t hiddSecs;	// 0x1C
	uint32_t numSecs;	// 0x20
	uint32_t secPerFAT;	// 0x24
	uint16_t extFlags;	// 0x28
	uint8_t minFAT32Ver; // 0x2A
	uint8_t majFAT32Ver; // 0x2B
	uint32_t rootClus;	// 0x2C
	uint16_t infoFS;	// 0x30
	uint16_t backupBootSec;	// 0x32
	uint8_t reserved[12];	// 0x34
	uint8_t logicDrvNum; // 0x40
	uint8_t unused;		// 0x41
	uint8_t extSig;		// 0x42 - 29h
	uint32_t partitionSerial; // 0x43
	char volName[11]; // 0x47
	char FATname[8]; // 0x52 - FAT32
	char executableCode[420]; // 0x5A
	uint16_t bootRecSig; // 0x1FE - 0xAA55
};
#pragma pack(pop)

// Structure of a NTFS boot sector
#pragma pack(push, 1)
struct NTFS_BS {
	uint8_t jmpBoot[3];	// 0x00
	char OEM_id[8];		// 0x03 // must be NTFS____
	uint16_t bytesPerSec; // 0x0B
	uint8_t secPerClus;	// 0x0D
	uint16_t rsvdSec;	// 0x0E
	uint8_t unused1[5];	// 0x10
	uint8_t media;		// 0x15 // val 0xF8 for hard drive (not floppy)
	uint16_t unused2; // 0x16
	uint16_t secPerTrk;	// 0x18
	uint16_t numHeads;	// 0x1A
	uint32_t hiddSecs;	// 0x1C
	uint64_t unused3;	// 0x20
	uint64_t numSecs;	// 0x28
	uint64_t clusOfMFT;	// 0x30
	uint64_t clusOfMirrMFT;	// 0x38

	int8_t szFileRecord; // 0x40 -> positive: num. of clusters || negative: 2^abs(value) bytes ; i.e: 0xf6 = -10 => 2^10 = 1024 bytes/File Record Segment
	uint8_t unused4[3];		// 0x41
	uint8_t szIndexBuff; // 0x44 -> positive: num. of clusters || negative: 2^abs(value) bytes ; i.e: 0xf6 = -10 => 2^10 = 1024 bytes/Index Buffer
	uint8_t unused5[3]; // 0x45
	uint8_t volumeSerial[8]; // 0x48
	uint32_t unused6;	// 0x50
	char executableCode[426]; // 0x54
	uint16_t bootRecSig; // 0x1FE - 0xAA55
};
#pragma pack(pop)

// Structure of a FAT32 directory entry
#pragma pack(push, 1)
struct FAT32_DirectoryEntry {
	char name[8]; // 0x00
	char ext[3]; // 0x08
	uint8_t attr; // 0x0B
	uint8_t reserved; // 0x0C
	uint8_t cTimeTenth; // 0x0D
	uint16_t cTime; // 0x0E
	uint16_t cDate; // 0x10
	uint16_t aDate; // 0x12
	uint16_t firstClusHi; // 0x14
	uint16_t wTime; // 0x16
	uint16_t wDate; // 0x18
	uint16_t firstClusLo; // 0x1A
	uint32_t fileSize; // 0x1C
};
#pragma pack(pop)

// Structure of a FAT32 LFN directory entry
#pragma pack(push, 1)
struct FAT32_LFN_DirectoryEntry {
	uint8_t seqNum; // 0x00
	uint16_t name1[5]; // 0x01
	uint8_t attr; // 0x0B
	uint8_t type; // 0x0C
	uint8_t checksum; // 0x0D
	uint16_t name2[6]; // 0x0E
	uint16_t firstClusLo; // 0x1C
	uint16_t name3[2]; // 0x1E
};
#pragma pack(pop)

// Structure of a NTFS entry
#pragma pack(push, 1)
struct NTFS_MftEntryHeader {
	uint8_t signature[4]; // 0x00
	uint16_t fixupOffset; // 0x04	
	uint16_t fixupSize; // 0x06
	uint64_t logSeqNum; // 0x08
	uint16_t seqNum; // 0x10
	uint16_t hardLinks; // 0x12
	uint16_t attrOffset; // 0x14 (important)
	uint16_t flags; // 0x16: 0x00: not in use, 0x01: in use, 0x02: directory, 0x03: directory and in use
	uint32_t usedLength; // 0x18
	uint32_t totalLength; // 0x1C
	uint64_t entryBase; // 0x20
	uint16_t nextAttrId; // 0x28
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NTFS_MftEntry {
	NTFS_MftEntryHeader header;
	uint8_t* attributes;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NTFS_AttrHeader {
	uint32_t type; // 0x00
	uint32_t length; // 0x04
	uint8_t nonResident; // 0x08
	uint8_t nameLength; // 0x09
	uint16_t nameOffset; // 0x0A
	uint16_t flags; // 0x0C
	uint16_t attrId; // 0x0E
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NTFS_AttrResident {
	NTFS_AttrHeader header;
	uint16_t length; // 0x10
	uint16_t attrOffset; // 0x12
	uint8_t indexed; // 0x14
	uint8_t padding; // 0x15
	uint16_t contentLength; // 0x16
	uint16_t contentOffset; // 0x18
	uint16_t flags; // 0x1A
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NTFS_AttrNonResident {
	NTFS_AttrHeader header;
	uint64_t startVCN; // 0x10
	uint64_t endVCN; // 0x18
	uint16_t dataRunOffset; // 0x20
	uint16_t compressionUnitSize; // 0x22
	uint32_t padding; // 0x24
	uint64_t allocatedSize; // 0x28
	uint64_t dataSize; // 0x30
	uint64_t initializedSize; // 0x38
	uint64_t compressedSize; // 0x40
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NTFS_AttrDataNonRes {
	NTFS_AttrHeader header;
	uint64_t initialVCN; // 0x10
	uint64_t finalVCN; // 0x18
	uint16_t dataRunOffset; // 0x20
	uint16_t compressionUnitSize; // 0x22
	uint32_t padding; // 0x24
	uint64_t allocatedSize; // 0x28
	uint64_t dataSize; // 0x30
	uint64_t initializedSize; // 0x38
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NTFS_AttrDataRes {
	NTFS_AttrHeader header;
	uint32_t length; // 0x10
	uint16_t attrOffset; // 0x14
	uint8_t indexed; // 0x16
	uint8_t padding; // 0x17
	uint8_t content; // 0x18
};

#pragma pack(push, 1)
struct NTFS_AttrIndexRoot {
	NTFS_AttrHeader header;
	int32_t length; // 0x10
	uint16_t attrOffset; // 0x14
	uint8_t indexed; // 0x16
	uint8_t padding; // 0x17
	wchar_t attrName[4]; // 0x18
	uint32_t attrType; // 0x20
	uint32_t collationRule; // 0x24
	uint32_t indexAllocationSize; // 0x28
	uint8_t clustersPerIndexRecord; // 0x2C
	uint8_t padding2[3]; // 0x2D
	uint32_t offsetToFirstIndexEntry; // 0x30
	uint32_t totalSizeOfEntries; // 0x34
	uint32_t allocatedSizeOfEntries; // 0x38
	uint8_t flags; // 0x3C
	uint8_t padding3[3]; // 0x3D
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NTFS_AttrFileName {
	NTFS_AttrHeader header;
	uint64_t parentDirRef; // 0x10
	uint64_t createTime; // 0x18
	uint64_t modifyTime; // 0x20
	uint64_t mftChangeTime; // 0x28
	uint64_t accessedTime; // 0x30
	uint64_t allocatedSize; // 0x38
	uint64_t dataSize; // 0x40
	uint32_t flags; // 0x48
	uint32_t reparse; // 0x4C
	uint8_t nameLength; // 0x50
	uint8_t nameType; // 0x51
	uint16_t name[0x30]; // 0x52
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NTFS_AttrStandardInfo {
	NTFS_AttrHeader header;
	int32_t length; // 0x10
	uint16_t attrOffset; // 0x14
	uint8_t indexed; // 0x16
	uint8_t padding; // 0x17
	uint64_t createdTime; // 0x18
	uint64_t modifiedTime; // 0x20
	uint64_t mftChangedTime; // 0x28
	uint64_t accessedTime; // 0x2A
	uint32_t fileFlags; // 0x38
	uint32_t padding2; // 0x3C
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NTFS_IndexFile {
	uint8_t signature[4]; // 0x00 - INDX
	uint16_t fixupOffset; // 0x04
	uint16_t fixupSize; // 0x06
	uint64_t logSeqNum; // 0x08
	uint64_t indexVCN; // 0x10
	uint32_t indexEntryOffset; // 0x18
	uint32_t indexEntrySize; // 0x1C
	uint32_t indexEntryAllocSize; // 0x20
	uint8_t flags; // 0x24 - designate if the node has children
	uint8_t padding[3]; // 0x25
	uint16_t updateSeqNum; // 0x28
	uint8_t* others; // 0x2A
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NTFS_IndexEntry {
	uint64_t reference; // 0x00
uint16_t length; // 0x08
uint16_t offsetToEndName; // 0x0A
uint16_t flags; // 0x0C
uint8_t padding[2]; // 0x0E
uint64_t parentDirRef; // 0x10
uint64_t createTime; // 0x18
uint64_t modifyTime; // 0x20
uint64_t mftChangeTime; // 0x28
uint64_t accessTime; // 0x30
uint64_t allocatedSize; // 0x38
uint64_t dataSize; // 0x40
uint64_t fileFlags; // 0x48
uint8_t nameLength; // 0x50
uint8_t fileNamespace; // 0x51
wchar_t fileName; // 0x52
};
#pragma pack(pop)

