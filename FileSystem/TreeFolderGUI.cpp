#include "TreeFolderGUI.h"

TreeFolderGUI::TreeFolderGUI(QWidget* parent, uint8_t* bootSector, std::string drive)
	: QMainWindow(parent)
	, ui(new Ui::TreeFolderGUIClass())
	, drive(drive)
{

	ui->setupUi(this);

	this->bootSector = new uint8_t[BOOT_SECTOR_SIZE] { 0 };
	memcpy(this->bootSector, bootSector, BOOT_SECTOR_SIZE);

	this->fileSystemType = getFileSystemType((uint8_t*)bootSector);

	initializeTreeFolder();
}

TreeFolderGUI::~TreeFolderGUI()
{
	delete ui;
	delete[] bootSector;
}

void TreeFolderGUI::initializeTreeFolder() {

	// Initialize current path
	drive[0] = toupper(drive[0]);
	currentPath = std::wstring(drive.begin(), drive.end()) + L"\\";
	ui->txtPath->setText(QString::fromStdWString(currentPath));

	// Some decoration for GUI
	ui->treeFolder->setColumnWidth(0, 200);
	ui->treeFolder->setColumnWidth(1, 60);
	ui->treeFolder->setColumnWidth(2, 90);
	ui->treeFolder->setColumnWidth(3, 125);
	ui->treeFolder->setColumnWidth(4, 125);
	ui->treeFolder->setColumnWidth(7, 90);
	ui->treeFolder->setColumnWidth(8, 20);

	// Initialize folder tree
	if (fileSystemType == FileSystem::FAT32) {
		this->setWindowTitle("Cây thư mục FAT32");
		initializeTreeFolderFAT32();
	}
	else if (fileSystemType == FileSystem::NTFS) {
		this->setWindowTitle("Cây thư mục NTFS");
		initializeTreeFolderNTFS();
	}
	else {
		throw "Hệ thống tập tin không được hỗ trợ!";
	}

}

void TreeFolderGUI::initializeTreeFolderFAT32() {

	// Add listener for double click event on file/folder
	connect(ui->treeFolder, &QTreeWidget::itemDoubleClicked, this, &TreeFolderGUI::onTreeItemDoubleClickedFAT32);

	// Show the root directory
	FAT32_BS* bootSe = (FAT32_BS*)this->bootSector;
	stackCluster.push(bootSe->rootClus);
	displayCurrentFolder(drive, bootSe->rootClus, bootSe);
	
}

void TreeFolderGUI::displayCurrentFolder(std::string drive, uint32_t cluster, FAT32_BS* bootSector) {
	ui->treeFolder->clear();

	// Read cluster chain
	std::vector<uint32_t> clusterChain = readClusterChainFat32(cluster, drive, bootSector);

	// Define data sector
	const int dataSector = bootSector->rsvdSec + bootSector->numFATs * bootSector->secPerFAT;
	// define stack to store long file name
	std::stack<FAT32_LFN_DirectoryEntry> stackLFN;

	// For each cluster in the cluster chain, read each sector in the cluster
	uint8_t* sectorBuffer = nullptr;
	for (uint32_t cluster : clusterChain) {
		int startSector = dataSector + (cluster - 2) * bootSector->secPerClus;
		int endSector = startSector + bootSector->secPerClus;
		
		while (startSector < endSector) {

			sectorBuffer = new uint8_t[bootSector->bytesPerSec] { 0 };
			readSector(drive, startSector * bootSector->bytesPerSec, sectorBuffer);
			
			// For each sector, read each directory entry (32 bytes)
			for (int offset = 0; offset < bootSector->bytesPerSec; offset += 32) {
				
				// Check if entry is empty or deleted
				if (sectorBuffer[offset] == 0xE5 || sectorBuffer[offset] == 0x00) {
					continue;
				}

				// Check if entry start with 0x05, replace it with 0xE5
				if (sectorBuffer[offset] == 0x05) {
					sectorBuffer[offset] = 0xE5;
				}

				// Read the entry
				FAT32_DirectoryEntry entry = *(FAT32_DirectoryEntry*)(sectorBuffer + offset);
				stackLFN.push(*(FAT32_LFN_DirectoryEntry*)&entry);

				// Check 
				if (entry.attr != 0x0F) {
					// Main entry
					std::wstring fileName = getFileNameFAT32(stackLFN);
					addItemToTreeFAT32(entry, fileName);
				}
			}

			delete[] sectorBuffer;
			startSector++;
		}
	}
}

void TreeFolderGUI::initializeTreeFolderNTFS() {
}

void TreeFolderGUI::addItemToTreeFAT32(const FAT32_DirectoryEntry& entry, std::wstring name) {
	// If name is ".", don't add it to tree
	if (name == L".") return;

	// Add items to tree: (name, type, size, time created, time modified, date accessed, attributes, sectors, cluster start):
	QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeFolder);
	item->setTextAlignment(2, Qt::AlignRight);
	
	// 1. Set name to item
	item->setText(0, QString::fromStdWString(name));

	// If name is "..", just add it to tree, don't add other properties
	if (name == L"..") {
		// sort this folder to the top
		ui->treeFolder->sortItems(0, Qt::AscendingOrder);
		return;
	}

	// 2. Get type
	std::string type;
	entry.attr & 0x10 ? type = "Thư mục" : type = "Tập tin";
	// Set type to item
	item->setText(1, QString::fromStdString(type));

	// 3. Get size
	std::string size;
	entry.attr & 0x10 ? size = "" : size = std::to_string(entry.fileSize) + " B";
	// Set size to item
	item->setText(2, QString::fromStdString(size));

	// 4.1. Get time created: 15-11: Hours(0–23), 10-5: Minutes(0–59), 4-0: Seconds / 2 (0–29)
	std::string timeCreated = std::to_string((entry.cTime >> 11) & 0x1F) + ":" + std::to_string((entry.cTime >> 5) & 0x3F) + ":" + std::to_string((entry.cTime & 0x1F) * 2);
	// 4.2. Get date created: 15-9: Year(0 = 1980, 119 = 2099 supported on FAT32 volumes, 0 = 1980, 127 = 2107 supported on exFAT volumes), 8-5: Month(1–12), 4-0: Day(1–31)
	std::string dateCreated = std::to_string(((entry.cDate >> 9) & 0x7F) + 1980) + "-" + std::to_string((entry.cDate >> 5) & 0xF) + "-" + std::to_string(entry.cDate & 0x1F);
	// 4.3. Get fine resolution
	if (entry.cTimeTenth > 100) timeCreated[6] += 1;
	// Set time created to item
	item->setText(3, QString::fromStdString(dateCreated + " " + timeCreated));

	// 5.1. Get time modified: 15-11: Hours(0–23), 10-5: Minutes(0–59), 4-0: Seconds / 2 (0–29)
	std::string timeModified = std::to_string((entry.wTime >> 11) & 0x1F) + ":" + std::to_string((entry.wTime >> 5) & 0x3F) + ":" + std::to_string((entry.wTime & 0x1F) * 2);
	// 5.2. Get date modified: 15-9: Year(0 = 1980, 119 = 2099 supported on FAT32 volumes, 0 = 1980, 127 = 2107 supported on exFAT volumes), 8-5: Month(1–12), 4-0: Day(1–31)
	std::string dateModified = std::to_string(((entry.wDate >> 9) & 0x7F) + 1980) + "-" + std::to_string((entry.wDate >> 5) & 0xF) + "-" + std::to_string(entry.wDate & 0x1F);
	// Set time modified to item
	item->setText(4, QString::fromStdString(dateModified + " " + timeModified));

	// 6. Get date accessed: 15-9: Year(0 = 1980, 119 = 2099 supported on FAT32 volumes, 0 = 1980, 127 = 2107 supported on exFAT volumes), 8-5: Month(1–12), 4-0: Day(1–31)
	std::string dateAccessed = std::to_string(((entry.aDate >> 9) & 0x7F) + 1980) + "-" + std::to_string((entry.aDate >> 5) & 0xF) + "-" + std::to_string(entry.aDate & 0x1F);
	// Set date accessed to item
	item->setText(5, QString::fromStdString(dateAccessed));

	// 7. Get attributes
	std::string attributes = "";
	if (entry.attr & 0x01) attributes += "Read-only, ";
	if (entry.attr & 0x02) attributes += "Hidden, ";
	if (entry.attr & 0x04) attributes += "System, ";
	if (entry.attr & 0x08) attributes += "Volume label, ";
	if (entry.attr & 0x10) attributes += "Subdirectory, ";
	if (entry.attr & 0x20) attributes += "Archive, ";
	if (entry.attr & 0x40) attributes += "Device, ";
	if (entry.attr & 0x80) attributes += "Reserved, ";
	attributes.erase(attributes.find_last_not_of(", ") + 1);
	// Set attributes to item
	item->setText(6, QString::fromStdString(attributes));

	// 8. Get sectors indexes
	std::string clusterStart = std::to_string(entry.firstClusHi << 16 | entry.firstClusLo);
	FAT32_BS* bootSector = (FAT32_BS*)this->bootSector;
	const int dataSector = bootSector->rsvdSec + bootSector->numFATs * bootSector->secPerFAT;

	std::vector<uint32_t> clusterChain = readClusterChainFat32(std::stoi(clusterStart) , drive, bootSector);
	if (clusterChain.empty()) return;
	std::string sectors = std::to_string(dataSector + (clusterChain[0] - 2) * bootSector->secPerClus);
	for (int i = 0; i < clusterChain.size() - 1; i++) {
		if (clusterChain[i] + 1 != clusterChain[i + 1]) {
			sectors += "-" + std::to_string(dataSector + (clusterChain[i] - 2) * bootSector->secPerClus + bootSector->secPerClus - 1) + ", ";
			sectors += std::to_string(dataSector + (clusterChain[i + 1] - 2) * bootSector->secPerClus);
		}
	}
	sectors += "-" + std::to_string(dataSector + (clusterChain[clusterChain.size() - 1] - 2) * bootSector->secPerClus + bootSector->secPerClus - 1);
	// Set sectors to item
	item->setText(7, QString::fromStdString(sectors));

	// 9. Get cluster start
	// Set cluster start to item
	item->setText(8, QString::fromStdString(clusterStart));

}

std::wstring TreeFolderGUI::getFileNameFAT32(std::stack<FAT32_LFN_DirectoryEntry>& stackLFN) {
	
	if (stackLFN.empty()) return L"";
	std::wstring name = L"";

	// Short file name
	if (stackLFN.size() == 1) {
		FAT32_DirectoryEntry entry = *(FAT32_DirectoryEntry*)&stackLFN.top();
		stackLFN.pop();
		std::string fileName = std::string(entry.name, sizeof(entry.name));
		std::string fileExt = std::string(entry.ext, sizeof(entry.ext));
		// Trim spaces
		fileName.erase(fileName.find_last_not_of(' ') + 1);
		fileExt.erase(fileExt.find_last_not_of(' ') + 1);
		// Convert to wstring (UTF-16)
		std::wstring wFileName = std::wstring(fileName.begin(), fileName.end());
		std::wstring wFileExt = std::wstring(fileExt.begin(), fileExt.end());
		// Combine name and extension
		wFileExt.empty() ? name = wFileName : name = wFileName + L"." + wFileExt;
	}
	// Long file name
	else {
		stackLFN.pop();
		while (!stackLFN.empty()) {
			// Read each LFN entry
			FAT32_LFN_DirectoryEntry entry = stackLFN.top();
			stackLFN.pop();
			// Convert to wstring (UTF-16)
			name += std::wstring((wchar_t*)entry.name1, sizeof(entry.name1) / sizeof(wchar_t));
			name += std::wstring((wchar_t*)entry.name2, sizeof(entry.name2) / sizeof(wchar_t));
			name += std::wstring((wchar_t*)entry.name3, sizeof(entry.name3) / sizeof(wchar_t));
		}
	}

	// My name string may contain 0xFFFF at the end, so I have to trim it
	name.erase(name.find_last_not_of(L'\xFFFF') + 1);
	return name;

}

void TreeFolderGUI::openFileFAT32(QTreeWidgetItem* item) {
	uint32_t clusterStart = std::stoi(item->text(8).toStdString());
	std::wstring wName = item->text(0).toStdWString();
	wName.erase(std::remove(wName.begin(), wName.end(), '\0'), wName.end());

	// Read cluster chain
	std::vector<uint32_t> clusterChain = readClusterChainFat32(clusterStart, drive, (FAT32_BS*)this->bootSector);
	if (clusterChain.empty()) return;
	FAT32_BS* bootSector = (FAT32_BS*)this->bootSector;
	const int dataSector = bootSector->rsvdSec + bootSector->numFATs * bootSector->secPerFAT;
	const int clusterSize = bootSector->bytesPerSec * bootSector->secPerClus;

	// read file	
	bool remaining = true;
	int64_t fileSize = std::stoi(item->text(2).toStdString());
	bool utf8 = false, utf16_little = false, utf16_big = false;
	uint8_t* sectorBuffer = nullptr;

	// Define the encoding of the file (UTF-8, UTF-16 little endian, UTF-16 big endian) in the first cluster
	int startSector = dataSector + (clusterChain[0] - 2) * bootSector->secPerClus;
	int endSector = startSector + bootSector->secPerClus;
	while (startSector < endSector && remaining) {
		sectorBuffer = new uint8_t[bootSector->bytesPerSec]{ 0 };
		fileSize -= readSector(drive, startSector * bootSector->bytesPerSec, sectorBuffer);

		// In the first sector, determine the encoding of the file
		if (!utf8 && !utf16_little && !utf16_big) {
			if (sectorBuffer[0] == 0xFF && sectorBuffer[1] == 0xFE) {
				utf16_little = true;
				//  add text to text preview
				ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)sectorBuffer + 1, (bootSector->bytesPerSec - 2) / 2)));
			}
			else if (sectorBuffer[0] == 0xFE && sectorBuffer[1] == 0xFF) {
				utf16_big = true;
				//  add text to text preview
				ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)sectorBuffer + 1, (bootSector->bytesPerSec - 2) / 2)));
			}
			else {
				utf8 = true;
				ui->txtPreview->insertPlainText(QString::fromStdString(std::string((char*)sectorBuffer, bootSector->bytesPerSec)));
			}
		}
		else {
			// From the second sector, read the file with the encoding defined above
			if (utf8) ui->txtPreview->insertPlainText(QString::fromStdString(std::string((char*)sectorBuffer, bootSector->bytesPerSec)));
			else if (utf16_little) ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)sectorBuffer, bootSector->bytesPerSec / 2)));
			else if (utf16_big) ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)sectorBuffer, bootSector->bytesPerSec / 2)));
		}

		// If file size remaining is greater than 0, continue reading file
		remaining = fileSize > 0;

		delete[] sectorBuffer;
		startSector++;
	}

	// For each cluster in the cluster chain, read each sector in the cluster
	for (int i = 1; i < clusterChain.size(); i++) {
		uint32_t cluster = clusterChain[i];

		// If it is not the last cluster, read the whole cluster
		if (i != clusterChain.size() - 1) {
			uint64_t offset = (uint64_t)clusterSize * (uint64_t)(cluster - 2) + (uint64_t)dataSector * (uint64_t)bootSector->bytesPerSec;
			uint8_t* clusterBuffer = new uint8_t[clusterSize]{ 0 };
			fileSize -= readCluster(drive, offset, clusterBuffer, clusterSize);

			if (utf8) ui->txtPreview->insertPlainText(QString::fromStdString(std::string((char*)clusterBuffer, clusterSize)));
			else if (utf16_little) ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)clusterBuffer, clusterSize / 2)));
			else if (utf16_big) ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)clusterBuffer, clusterSize / 2)));

			delete[] clusterBuffer;
		}
		// If it is the last cluster, read the remaining bytes
		else {
			int startSector = dataSector + (cluster - 2) * bootSector->secPerClus;
			int endSector = startSector + ceil((double)fileSize / (double)bootSector->bytesPerSec);

			while (startSector < endSector) {
				sectorBuffer = new uint8_t[bootSector->bytesPerSec]{ 0 };
				fileSize -= readSector(drive, startSector * bootSector->bytesPerSec, sectorBuffer);

				// Read the file with the encoding defined above
				if (utf8) ui->txtPreview->insertPlainText(QString::fromStdString(std::string((char*)sectorBuffer, bootSector->bytesPerSec)));
				else if (utf16_little) ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)sectorBuffer, bootSector->bytesPerSec / 2)));
				else if (utf16_big) ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)sectorBuffer, bootSector->bytesPerSec / 2)));

				delete[] sectorBuffer;
				startSector++;
			}
		}
	}

}

void TreeFolderGUI::openFolderFAT32(QTreeWidgetItem* item) {

	std::wstring fileName = item->text(0).toStdWString();
	fileName.erase(std::remove(fileName.begin(), fileName.end(), '\0'), fileName.end());
	uint32_t cluster;

	// Open parent folder
	if (fileName == L"..") {
		// Pop out the last folder in current path
		this->stackCluster.pop();
		cluster = stackCluster.top();
		currentPath.erase(currentPath.find_last_of(L"\\"));
		currentPath.erase(currentPath.find_last_of(L"\\"));
		currentPath += L"\\";
	}
	// Open child folder
	else {
		// Add folder to current path
		cluster = std::stoi(item->text(8).toStdString());
		currentPath = currentPath + fileName + L"\\";
		stackCluster.push(cluster);
	};

	// Display current folder
	displayCurrentFolder(drive, cluster, (FAT32_BS*)this->bootSector);
	ui->txtPath->setText(QString::fromStdWString(currentPath));

}

void TreeFolderGUI::onTreeItemDoubleClickedFAT32(QTreeWidgetItem* item, int column) {
	ui->txtPreview->clear();

	if (item->text(1) == "Tập tin") {
		// If file, read file content
		std::wstring fileName = item->text(0).toStdWString();
		if (fileName.find(L".txt") == std::string::npos) {
			ui->txtPreview->insertPlainText("Không hỗ trợ đọc tập tin này!");
			std::wstring fullPath = currentPath + fileName;
			ShellExecute(0, 0, L"rundll32.exe", (L"shell32.dll,OpenAs_RunDLL " + fullPath).c_str(), 0, SW_SHOW);
		}
		else {
			openFileFAT32(item);
		}
	}
	else {
		// If folder, open folder
		openFolderFAT32(item);
	}
}
