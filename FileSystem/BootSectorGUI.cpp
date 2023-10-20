#include "BootSectorGUI.h"

BootSectorGUI::BootSectorGUI(QWidget *parent, void* fat32BS)
	: QMainWindow(parent)
	, ui(new Ui::BootSectorGUIClass())
	, bootSector(fat32BS)
{
	ui->setupUi(this);
	initializeGUI();
}

BootSectorGUI::~BootSectorGUI()
{
	delete ui;
}

void BootSectorGUI::initializeGUI() {
	this->setWindowIcon(QIcon("./assets/folder_yellow.png"));

	ui->tabWidget->setTabText(0, "Chi tiết");
	ui->tabWidget->setTabText(1, "Byte thô");

	initializeRawData();
	initializeInfo();
}

void BootSectorGUI::initializeRawData() {

	uint8_t* p = (uint8_t*)bootSector;
	
	// display the bytes of the boot sector in hex to the tblRawData, 16 cols, 32 rows (512 bytes)
	for (int i = 0; i < 32; i++) {
		for (int j = 0; j < 16; j++) {
			// if the byte less than 16, add a 0 in front of it
			QTableWidgetItem* item;
			if (p[i * 16 + j] < 16) {
				item = new QTableWidgetItem("0" + QString::number(p[i * 16 + j], 16).toUpper());
			}
			else {
				item = new QTableWidgetItem(QString::number(p[i * 16 + j], 16).toUpper());
			}
			item->setTextAlignment(Qt::AlignCenter);
			ui->tblRawData->setItem(i, j, item);
		}
	}

	// make the column narrower to fit the view (do not need to scroll horizontally)
	for (int i = 0; i < 16; i++) {
		ui->tblRawData->setColumnWidth(i, 33);
	}

	// make both vertical and horizontal headers underlined font
	QFont font = ui->tblRawData->horizontalHeader()->font();
	font.setUnderline(true);
	ui->tblRawData->horizontalHeader()->setFont(font);
	ui->tblRawData->verticalHeader()->setFont(font);

	// make table not editable
	ui->tblRawData->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// make columns not resizable
	ui->tblRawData->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

}

void BootSectorGUI::initializeInfo() {
	// get table widget
	QTableWidget* tblInfo = ui->tblInfo;

	// make table not editable
	tblInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// make the columns larger to fit the view
	tblInfo->setColumnWidth(0, 250);
	tblInfo->setColumnWidth(1, 50);
	tblInfo->setColumnWidth(2, 250);

	// under each row, there is a horizontal line
	tblInfo->setShowGrid(true);

	// make columns not resizable
	tblInfo->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	FileSystem fs = readFileSystemType((uint8_t*)bootSector);
	if (fs == FileSystem::FAT32) {
		this->setWindowTitle("Thông tin boot sector FAT32");
		initializeFat32Info();
	}
	else if (fs == FileSystem::NTFS) {
		// set title
		this->setWindowTitle("Thông tin partition boot sector NTFS");
		initializeNTFSInfo();
	}
	else {
		throw "Hệ thống tập tin không được hỗ trợ!";
	}

	// make the second column horizontally centered
	for (int i = 0; i < tblInfo->rowCount(); i++) {
		tblInfo->item(i, 1)->setTextAlignment(Qt::AlignCenter);
	}

}

void BootSectorGUI::initializeFat32Info() {
	ui->groupBox->setTitle("Thông tin boot sector FAT32");
	FAT32_BS* bootSector = (FAT32_BS*)this->bootSector;

	// Add rows, each row has 3 columns (propery, offset, value)
	addInfoRow("Lệnh nhảy", offsetof(FAT32_BS, jmpBoot), toHexString(bootSector->jmpBoot, sizeof(bootSector->jmpBoot)));
	addInfoRow("OEM ID", offsetof(FAT32_BS, OEM_id), std::string(bootSector->OEM_id, sizeof(bootSector->OEM_id)));
	addInfoRow("Số byte/sector", offsetof(FAT32_BS, bytesPerSec), std::to_string(bootSector->bytesPerSec));
	addInfoRow("Số sector/cluster", offsetof(FAT32_BS, secPerClus), std::to_string(bootSector->secPerClus));
	addInfoRow("Số sector trước FAT", offsetof(FAT32_BS, rsvdSec), std::to_string(bootSector->rsvdSec));
	addInfoRow("Số bảng FAT", offsetof(FAT32_BS, numFATs), std::to_string(bootSector->numFATs));
	addInfoRow("Loại media", offsetof(FAT32_BS, media), toHexString(&bootSector->media));
	addInfoRow("Số sector/track", offsetof(FAT32_BS, secPerTrk), std::to_string(bootSector->secPerTrk));
	addInfoRow("Số head", offsetof(FAT32_BS, numHeads), std::to_string(bootSector->numHeads));
	addInfoRow("Số sector ẩn", offsetof(FAT32_BS, hiddSecs), std::to_string(bootSector->hiddSecs));
	addInfoRow("Tổng số sector trên phân vùng", offsetof(FAT32_BS, numSecs), std::to_string(bootSector->numSecs));
	addInfoRow("Số sector/FAT", offsetof(FAT32_BS, secPerFAT), std::to_string(bootSector->secPerFAT));
	addInfoRow("Flags", offsetof(FAT32_BS, extFlags), toHexString((uint8_t*)&bootSector->extFlags, sizeof(bootSector->extFlags)));
	addInfoRow("Phiên bản FAT32", offsetof(FAT32_BS, majFAT32Ver), std::to_string(bootSector->majFAT32Ver) + "." + std::to_string(bootSector->minFAT32Ver));
	addInfoRow("Cluster đầu tiên của Root Directory", offsetof(FAT32_BS, rootClus), std::to_string(bootSector->rootClus));
	addInfoRow("FS Information Sector", offsetof(FAT32_BS, infoFS), std::to_string(bootSector->infoFS));
	addInfoRow("Sector chứa backup boot sector", offsetof(FAT32_BS, backupBootSec), std::to_string(bootSector->backupBootSec));
	addInfoRow("Ký hiệu vật lý của đĩa (0: mềm, 80h: cứng)", offsetof(FAT32_BS, logicDrvNum), toHexString(&bootSector->logicDrvNum));
	addInfoRow("Ký hiệu FAT32", offsetof(FAT32_BS, extSig), toHexString(&bootSector->extSig));
	addInfoRow("Số serial của phân vùng", offsetof(FAT32_BS, partitionSerial), toHexString((uint8_t*)&bootSector->partitionSerial, sizeof(bootSector->partitionSerial)));
	addInfoRow("Tên Volume", offsetof(FAT32_BS, volName), std::string(bootSector->volName, sizeof(bootSector->volName)));
	addInfoRow("Loại FAT", offsetof(FAT32_BS, FATname), std::string(bootSector->FATname, sizeof(bootSector->FATname)));
	addInfoRow("Đoạn chương trình nạp boot", offsetof(FAT32_BS, executableCode), toHexString((uint8_t*)bootSector->executableCode, sizeof(bootSector->executableCode)));
	addInfoRow("Kết thúc boot sector", offsetof(FAT32_BS, bootRecSig), toHexString((uint8_t*)&bootSector->bootRecSig, sizeof(bootSector->bootRecSig)));
}

void BootSectorGUI::initializeNTFSInfo() {
	ui->groupBox->setTitle("Thông tin partition boot sector NTFS");
	NTFS_BS* bootSector = (NTFS_BS*)this->bootSector;

	// Add rows, each row has 3 columns (propery, offset, value)
	addInfoRow("Lệnh nhảy", offsetof(NTFS_BS, jmpBoot), toHexString(bootSector->jmpBoot, sizeof(bootSector->jmpBoot)));
	addInfoRow("OEM ID", offsetof(NTFS_BS, OEM_id), std::string(bootSector->OEM_id, sizeof(bootSector->OEM_id)));
	addInfoRow("Số byte/sector", offsetof(NTFS_BS, bytesPerSec), std::to_string(bootSector->bytesPerSec));
	addInfoRow("Số sector/cluster", offsetof(NTFS_BS, secPerClus), std::to_string(bootSector->secPerClus));
	addInfoRow("Loại media", offsetof(NTFS_BS, media), toHexString(&bootSector->media));
	addInfoRow("Số sector/track", offsetof(NTFS_BS, secPerTrk), std::to_string(bootSector->secPerTrk));
	addInfoRow("Số head", offsetof(NTFS_BS, numHeads), std::to_string(bootSector->numHeads));
	addInfoRow("Số sector ẩn", offsetof(NTFS_BS, hiddSecs), std::to_string(bootSector->hiddSecs));
	addInfoRow("Tổng số sector trên phân vùng", offsetof(NTFS_BS, numSecs), std::to_string(bootSector->numSecs));
	addInfoRow("Cluster đầu tiên của MFT", offsetof(NTFS_BS, clusOfMFT), std::to_string(bootSector->clusOfMFT));
	addInfoRow("Cluster đầu tiên của Mirror MFT", offsetof(NTFS_BS, clusOfMirrMFT), std::to_string(bootSector->clusOfMirrMFT));
	// if szFileRecord is positive, it is the number of clusters, if negative, it is 2^abs(value) bytes
	if ((int)bootSector->szFileRecord > 0) {
		addInfoRow("Kích thước File Record Segment (cluster)", offsetof(NTFS_BS, szFileRecord), std::to_string(bootSector->szFileRecord));
	}
	else {
		addInfoRow("Kích thước File Record Segment (byte)", offsetof(NTFS_BS, szFileRecord), std::to_string((int)pow(2, abs(bootSector->szFileRecord))));
	}
	// if szIndexBuff is positive, it is the number of clusters, if negative, it is 2^abs(value) bytes
	if ((int)bootSector->szIndexBuff > 0) {
		addInfoRow("Kích thước Index Buffer (cluster)", offsetof(NTFS_BS, szIndexBuff), std::to_string(bootSector->szIndexBuff));
	}
	else {
		addInfoRow("Kích thước Index Buffer (byte)", offsetof(NTFS_BS, szIndexBuff), std::to_string((int)pow(2, abs(bootSector->szIndexBuff))));
	}
	addInfoRow("Số serial của phân vùng", offsetof(NTFS_BS, volumeSerial), toHexString((uint8_t*)&bootSector->volumeSerial, sizeof(bootSector->volumeSerial)));
	addInfoRow("Đoạn chương trình nạp boot", offsetof(NTFS_BS, executableCode), toHexString((uint8_t*)bootSector->executableCode, sizeof(bootSector->executableCode)));
	addInfoRow("Kết thúc boot sector", offsetof(NTFS_BS, bootRecSig), toHexString((uint8_t*)&bootSector->bootRecSig, sizeof(bootSector->bootRecSig)));

}

void BootSectorGUI::addInfoRow(std::string name, int offset, std::string value) {
	// get table widget
	QTableWidget* tblInfo = ui->tblInfo;

	// add new row: name to first column, offset to second column, value to third column 
	int row = tblInfo->rowCount();
	tblInfo->insertRow(row);

	tblInfo->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(name)));
	// offset is in hex like 0x000, so we need to convert it to string, if less than 16, add a 0 in front of it, if less than 256, add a 0 in front of it
	if (offset < 16) {
		tblInfo->setItem(row, 1, new QTableWidgetItem("0x00" + QString::number(offset, 16).toUpper()));
	}
	else if (offset < 256) {
		tblInfo->setItem(row, 1, new QTableWidgetItem("0x0" + QString::number(offset, 16).toUpper()));
	}
	else {
		tblInfo->setItem(row, 1, new QTableWidgetItem("0x" + QString::number(offset, 16).toUpper()));
	}
	tblInfo->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(value)));

}

