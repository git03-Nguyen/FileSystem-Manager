#include "BootSectorGUI.h"

BootSectorGUI::BootSectorGUI(QWidget *parent, uint8_t* bootSector)
	: QMainWindow(parent)
	, ui(new Ui::BootSectorGUIClass())
	, bootSector(nullptr)
{
	ui->setupUi(this);

	this->bootSector = new uint8_t[BOOT_SECTOR_SIZE];
	memcpy(this->bootSector, bootSector, BOOT_SECTOR_SIZE);
	
	initializeGUI();
}

BootSectorGUI::~BootSectorGUI()
{
	delete ui;
	delete[] bootSector;
}

void BootSectorGUI::initializeGUI() {

	ui->tabWidget->setTabText(0, "Chi tiết");
	initializeDetailGUI();

	ui->tabWidget->setTabText(1, "Byte thô");
	initializeRawDataGUI();

}

void BootSectorGUI::initializeRawDataGUI() {

	uint8_t* p = (uint8_t*)bootSector;
	
	// display the bytes of the boot sector
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
			// add the byte to the table
			item->setTextAlignment(Qt::AlignCenter);
			ui->tblRawData->setItem(i, j, item);
		}
	}

	// make the column narrower to fit the view (do not need to scroll horizontally)
	for (int i = 0; i < 16; i++) {
		ui->tblRawData->setColumnWidth(i, 33);
	}

	// Some decoration for the GUI
	QFont font = ui->tblRawData->horizontalHeader()->font();
	font.setUnderline(true);
	ui->tblRawData->horizontalHeader()->setFont(font);
	ui->tblRawData->verticalHeader()->setFont(font);
	ui->tblRawData->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tblRawData->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

}

void BootSectorGUI::initializeDetailGUI() {
	QTableWidget* tblDetails = ui->tblDetails;

	// Get the file system type
	FileSystem fs = getFileSystemType((uint8_t*)bootSector);
	if (fs == FileSystem::FAT32) {
		this->setWindowTitle("Thông tin boot sector FAT32");
		initializeFat32Info();
	}
	else if (fs == FileSystem::NTFS) {
		this->setWindowTitle("Thông tin partition boot sector NTFS");
		initializeNTFSInfo();
	}
	else {
		throw "Hệ thống tập tin không được hỗ trợ!";
	}

	// Some decoration for the GUI
	tblDetails->setColumnWidth(0, 250);
	tblDetails->setColumnWidth(1, 50);
	tblDetails->setColumnWidth(2, 250);
	tblDetails->setShowGrid(true);
	tblDetails->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tblDetails->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	for (int i = 0; i < tblDetails->rowCount(); i++) {
		tblDetails->item(i, 1)->setTextAlignment(Qt::AlignCenter);
	}

}

void BootSectorGUI::initializeFat32Info() {

	ui->groupBox->setTitle("Thông tin boot sector FAT32");
	FAT32_BS* bootSector = (FAT32_BS*)this->bootSector;

	// Add rows, each row has 3 columns (propery, offset, value)
	addDetailRow("Lệnh nhảy", offsetof(FAT32_BS, jmpBoot), toHexString(bootSector->jmpBoot, sizeof(bootSector->jmpBoot)));
	addDetailRow("OEM ID", offsetof(FAT32_BS, OEM_id), std::string(bootSector->OEM_id, sizeof(bootSector->OEM_id)));
	addDetailRow("Số byte/sector", offsetof(FAT32_BS, bytesPerSec), std::to_string(bootSector->bytesPerSec));
	addDetailRow("Số sector/cluster", offsetof(FAT32_BS, secPerClus), std::to_string(bootSector->secPerClus));
	addDetailRow("Số sector trước FAT", offsetof(FAT32_BS, rsvdSec), std::to_string(bootSector->rsvdSec));
	addDetailRow("Số bảng FAT", offsetof(FAT32_BS, numFATs), std::to_string(bootSector->numFATs));
	addDetailRow("Loại đĩa", offsetof(FAT32_BS, media), toHexString(&bootSector->media));
	addDetailRow("Số sector/track", offsetof(FAT32_BS, secPerTrk), std::to_string(bootSector->secPerTrk));
	addDetailRow("Số head", offsetof(FAT32_BS, numHeads), std::to_string(bootSector->numHeads));
	addDetailRow("Sector bắt đầu của ổ đĩa logic", offsetof(FAT32_BS, hiddSecs), std::to_string(bootSector->hiddSecs));
	addDetailRow("Tổng số sector trên phân vùng", offsetof(FAT32_BS, numSecs), std::to_string(bootSector->numSecs));
	addDetailRow("Số sector/FAT", offsetof(FAT32_BS, secPerFAT), std::to_string(bootSector->secPerFAT));
	addDetailRow("Flags", offsetof(FAT32_BS, extFlags), toHexString((uint8_t*)&bootSector->extFlags, sizeof(bootSector->extFlags)));
	addDetailRow("Phiên bản FAT32", offsetof(FAT32_BS, minFAT32Ver), std::to_string(bootSector->majFAT32Ver) + "." + std::to_string(bootSector->minFAT32Ver));
	addDetailRow("Cluster đầu tiên của Root Directory", offsetof(FAT32_BS, rootClus), std::to_string(bootSector->rootClus));
	addDetailRow("FS Information Sector", offsetof(FAT32_BS, infoFS), std::to_string(bootSector->infoFS));
	addDetailRow("Sector chứa backup boot sector", offsetof(FAT32_BS, backupBootSec), std::to_string(bootSector->backupBootSec));
	addDetailRow("Ký hiệu vật lý của đĩa (0: mềm, 80h: cứng)", offsetof(FAT32_BS, logicDrvNum), toHexString(&bootSector->logicDrvNum));
	addDetailRow("Ký hiệu FAT32", offsetof(FAT32_BS, extSig), toHexString(&bootSector->extSig));
	addDetailRow("Số serial của phân vùng", offsetof(FAT32_BS, partitionSerial), toHexString((uint8_t*)&bootSector->partitionSerial, sizeof(bootSector->partitionSerial)));
	addDetailRow("Tên Volume", offsetof(FAT32_BS, volName), std::string(bootSector->volName, sizeof(bootSector->volName)));
	addDetailRow("Loại FAT", offsetof(FAT32_BS, FATname), std::string(bootSector->FATname, sizeof(bootSector->FATname)));
	addDetailRow("Đoạn chương trình nạp boot", offsetof(FAT32_BS, executableCode), toHexString((uint8_t*)bootSector->executableCode, sizeof(bootSector->executableCode)));
	addDetailRow("Kết thúc boot sector", offsetof(FAT32_BS, bootRecSig), toHexString((uint8_t*)&bootSector->bootRecSig, sizeof(bootSector->bootRecSig)));
}

void BootSectorGUI::initializeNTFSInfo() {
	ui->groupBox->setTitle("Thông tin partition boot sector NTFS");
	NTFS_BS* bootSector = (NTFS_BS*)this->bootSector;

	// Add rows, each row has 3 columns (propery, offset, value)
	addDetailRow("Lệnh nhảy", offsetof(NTFS_BS, jmpBoot), toHexString(bootSector->jmpBoot, sizeof(bootSector->jmpBoot)));
	addDetailRow("OEM ID", offsetof(NTFS_BS, OEM_id), std::string(bootSector->OEM_id, sizeof(bootSector->OEM_id)));
	addDetailRow("Số byte/sector", offsetof(NTFS_BS, bytesPerSec), std::to_string(bootSector->bytesPerSec));
	addDetailRow("Số sector/cluster", offsetof(NTFS_BS, secPerClus), std::to_string(bootSector->secPerClus));
	addDetailRow("Loại đĩa", offsetof(NTFS_BS, media), toHexString(&bootSector->media));
	addDetailRow("Số sector/track", offsetof(NTFS_BS, secPerTrk), std::to_string(bootSector->secPerTrk));
	addDetailRow("Số head", offsetof(NTFS_BS, numHeads), std::to_string(bootSector->numHeads));
	addDetailRow("Sector bắt đầu của ổ đĩa logic", offsetof(NTFS_BS, hiddSecs), std::to_string(bootSector->hiddSecs));
	addDetailRow("Tổng số sector trên phân vùng", offsetof(NTFS_BS, numSecs), std::to_string(bootSector->numSecs));
	addDetailRow("Cluster đầu tiên của MFT", offsetof(NTFS_BS, clusOfMFT), std::to_string(bootSector->clusOfMFT));
	addDetailRow("Cluster đầu tiên của Mirror MFT", offsetof(NTFS_BS, clusOfMirrMFT), std::to_string(bootSector->clusOfMirrMFT));
	// if szFileRecord is positive, it is the number of clusters, if negative, it is 2^abs(value) bytes
	if ((int)bootSector->szFileRecord > 0) {
		addDetailRow("Kích thước File Record Segment (cluster)", offsetof(NTFS_BS, szFileRecord), std::to_string(bootSector->szFileRecord));
	}
	else {
		addDetailRow("Kích thước File Record Segment (byte)", offsetof(NTFS_BS, szFileRecord), std::to_string((int)pow(2, abs(bootSector->szFileRecord))));
	}
	// if szIndexBuff is positive, it is the number of clusters, if negative, it is 2^abs(value) bytes
	if ((int)bootSector->szIndexBuff > 0) {
		addDetailRow("Kích thước Index Buffer (cluster)", offsetof(NTFS_BS, szIndexBuff), std::to_string(bootSector->szIndexBuff));
	}
	else {
		addDetailRow("Kích thước Index Buffer (byte)", offsetof(NTFS_BS, szIndexBuff), std::to_string((int)pow(2, abs(bootSector->szIndexBuff))));
	}
	addDetailRow("Số serial của phân vùng", offsetof(NTFS_BS, volumeSerial), toHexString((uint8_t*)&bootSector->volumeSerial, sizeof(bootSector->volumeSerial)));
	addDetailRow("Đoạn chương trình nạp boot", offsetof(NTFS_BS, executableCode), toHexString((uint8_t*)bootSector->executableCode, sizeof(bootSector->executableCode)));
	addDetailRow("Kết thúc boot sector", offsetof(NTFS_BS, bootRecSig), toHexString((uint8_t*)&bootSector->bootRecSig, sizeof(bootSector->bootRecSig)));

}

void BootSectorGUI::addDetailRow(std::string name, int offset, std::string value) {
	QTableWidget* tblDetails = ui->tblDetails;

	// add new row: (name, offset, value) to the table
	int row = tblDetails->rowCount();
	tblDetails->insertRow(row);
	tblDetails->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(name)));

	// offset column is in hex like 0x000, so we need to convert it to string
	if (offset < 16) {
		tblDetails->setItem(row, 1, new QTableWidgetItem("0x00" + QString::number(offset, 16).toUpper()));
	}
	else if (offset < 256) {
		tblDetails->setItem(row, 1, new QTableWidgetItem("0x0" + QString::number(offset, 16).toUpper()));
	}
	else {
		tblDetails->setItem(row, 1, new QTableWidgetItem("0x" + QString::number(offset, 16).toUpper()));
	}

	tblDetails->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(value)));

}

