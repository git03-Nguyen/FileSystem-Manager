#include "TreeFolderGUI.h"

TreeFolderGUI::TreeFolderGUI(QWidget* parent, void* bootSector, std::string drive)
	: QMainWindow(parent)
	, ui(new Ui::TreeFolderGUIClass())
	, drive(drive)
{

	ui->setupUi(this);
	this->setWindowTitle("Tree Folder");

	this->bootSector = new uint8_t[_SECTOR_SIZE] { 0 };
	memcpy(this->bootSector, bootSector, _SECTOR_SIZE);

	initializeTreeFolder();
}

TreeFolderGUI::~TreeFolderGUI()
{
	delete ui;
	delete[] bootSector;
}

void TreeFolderGUI::initializeTreeFolder() {
	drive[0] = toupper(drive[0]);
	currentPath = std::wstring(drive.begin(), drive.end()) + L"\\";
	ui->txtPath->setText(QString::fromStdWString(currentPath));

	ui->treeFolder->setColumnWidth(0, 200);
	ui->treeFolder->setColumnWidth(1, 60);
	ui->treeFolder->setColumnWidth(2, 90);
	ui->treeFolder->setColumnWidth(7, 70);

	FileSystem fileSystem = readFileSystemType((uint8_t*)bootSector);
	if (fileSystem == FileSystem::FAT32) {
		this->setWindowTitle("Cây thư mục FAT32");
		initializeTreeFolderFAT32();
	}/*
	else if (fileSystem == FileSystem::NTFS) {
		this->setWindowTitle("Cây thư mục NTFS");
		initializeTreeFolderNTFS();
	}*/
	else {
		throw "Hệ thống tập tin không được hỗ trợ!";
	}

}

void TreeFolderGUI::initializeTreeFolderFAT32() {

	// add double click event
	connect(ui->treeFolder, &QTreeWidget::itemDoubleClicked, this, &TreeFolderGUI::onTreeItemDoubleClicked);

	FAT32_BS* bootSe = (FAT32_BS*)bootSector;

	// open current cluster
	stackCluster.push(bootSe->rootClus);
	displayCurrentFolderFAT32(drive, bootSe->rootClus, bootSe);
	
}

void TreeFolderGUI::displayCurrentFolderFAT32(std::string drive, uint32_t cluster, FAT32_BS* bootSector) {
	// clear the tree, all items
	ui->treeFolder->clear();

	std::vector<uint32_t> clusterChain = readClusterChainFat32(cluster, drive, bootSector);

	// for each cluster in the cluster chain, read the directory entries
	const int dataSector = bootSector->rsvdSec + bootSector->numFATs * bootSector->secPerFAT;
	std::stack<FAT32_LFN_DirectoryEntry> stackLFN;

	for (uint32_t cluster : clusterChain) {
		int startSector = dataSector + (cluster - 2) * bootSector->secPerClus;
		int endSector = startSector + bootSector->secPerClus;
		for (; startSector < endSector; startSector++) {
			uint8_t* sector = new uint8_t[bootSector->bytesPerSec]{ 0 };
			readSector(drive, startSector, sector);

			// read directory entries
			// if the first byte of the entry is 0xE5, the entry is deleted, skip it
			// if the first byte of the entry is 0x00, the entry is empty, skip it
			// create a std::stack to store the entries (if long file name, push to stack)
			for (int offset = 0; offset < bootSector->bytesPerSec; offset += 32) {
				if (sector[offset] == 0xE5 || sector[offset] == 0x00) {
					continue;
				}

				if (sector[offset] == 0x05) {
					sector[offset] = 0xE5;
				}

				// read the entry
				FAT32_DirectoryEntry* entry = (FAT32_DirectoryEntry*)(sector + offset);
				if (entry->attr == 0x0F) {
					// long file name
					// push to stack
					stackLFN.push(*(FAT32_LFN_DirectoryEntry*)entry);
				}
				else {
					// short file name
					if (stackLFN.empty()) {
						// add to tree widget
						addItemToTree(entry);
					}
					else {
						std::wstring fileName = getNameLFN(stackLFN);
						addItemToTree(entry, fileName);
						//clear stack
						while (!stackLFN.empty()) {
							stackLFN.pop();
						}
					}
				}
			}
			delete[] sector;
		}
	}

}

void TreeFolderGUI::initializeTreeFolderNTFS() {
}

void TreeFolderGUI::addItemToTree(FAT32_DirectoryEntry* entry, std::wstring name) {
	// add item to tree widget
	// each item have 7 columns: name, type, size, date created, date modified, date accessed, attributes, cluster start (read from entry struct)

	// get name
	if (name == L"") {
		std::string fileName = std::string(entry->name, sizeof(entry->name));
		std::string fileExt = std::string(entry->ext, sizeof(entry->ext));
		// trim spaces in end of string
		fileName.erase(fileName.find_last_not_of(' ') + 1);
		fileExt.erase(fileExt.find_last_not_of(' ') + 1);
		// convert to wstring
		std::wstring wFileName = std::wstring(fileName.begin(), fileName.end());
		std::wstring wFileExt = std::wstring(fileExt.begin(), fileExt.end());
		if (wFileExt.empty()) {
			name = wFileName;
		}
		else {
			name = wFileName + L"." + wFileExt;
		}
	}

	// my name string may contain 0xFFFF at the end, so I have to trim it
	name.erase(name.find_last_not_of(L'\xFFFF') + 1);
	if (name == L".") {
		return;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeFolder);
	item->setText(0, QString::fromStdWString(name));


	// get type
	std::string type = "";
	if (entry->attr & 0x10) {
		type = "Thư mục";
	}
	else {
		type = "Tập tin";
	}
	
	if (name == L"..") {
		return;
	}

	// get size
	std::string size;
	if (entry->attr & 0x10) {
		size = "";
	}
	else {
		size = std::to_string(entry->fileSize) + " B";
	}

	// get time created
	std::string timeCreated = "";
	/*15 - 11	Hours(0–23)
			10 - 5	Minutes(0–59)
					4 - 0	Seconds / 2 (0–29)*/
	timeCreated += std::to_string((entry->cTime >> 11) & 0x1F) + ":";
	timeCreated += std::to_string((entry->cTime >> 5) & 0x3F) + ":";
	timeCreated += std::to_string((entry->cTime & 0x1F) * 2);

	// get date created
	std::string dateCreated = "";
	/*15 - 9	Year(0 = 1980, 119 = 2099 supported on FAT32 volumes, 0 = 1980, 127 = 2107 supported on exFAT volumes)
				8 - 5	Month(1–12)
									4 - 0	Day(1–31)*/
	dateCreated += std::to_string(((entry->cDate >> 9) & 0x7F) + 1980) + "-";
	dateCreated += std::to_string((entry->cDate >> 5) & 0xF) + "-";
	dateCreated += std::to_string(entry->cDate & 0x1F);

	// at 0x0D, there is create time, fine resolution: 10ms, so I have to add it to timeCreated
	uint16_t fineResolution = entry->cTimeTenth;
	if (fineResolution != 0) {
		// if > 100, add 1 to seconds
		if (fineResolution > 100) {
			timeCreated[6] += 1;
		}
	}

	std::string dateTimeCreated = dateCreated + " " + timeCreated;
	
	// get time modified
	std::string timeModified = "";
	/*15 - 11	Hours(0–23)
				10 - 5	Minutes(0–59)
									4 - 0	Seconds / 2 (0–29)*/
	timeModified += std::to_string((entry->wTime >> 11) & 0x1F) + ":";
	timeModified += std::to_string((entry->wTime >> 5) & 0x3F) + ":";
	timeModified += std::to_string((entry->wTime & 0x1F) * 2);

	// get date modified
	std::string dateModified = "";
	/*15 - 9	Year(0 = 1980, 119 = 2099 supported on FAT32 volumes, 0 = 1980, 127 = 2107 supported on exFAT volumes)
					8 - 5	Month(1–12)
														4 - 0	Day(1–31)*/
	dateModified += std::to_string(((entry->wDate >> 9) & 0x7F) + 1980) + "-";
	dateModified += std::to_string((entry->wDate >> 5) & 0xF) + "-";
	dateModified += std::to_string(entry->wDate & 0x1F);

	std::string dateTimeModified = dateModified + " " + timeModified;

	// get date accessed
	std::string dateAccessed = "";
	/*15 - 9	Year(0 = 1980, 119 = 2099 supported on FAT32 volumes, 0 = 1980, 127 = 2107 supported on exFAT volumes)
						8 - 5	Month(1–12)
																				4 - 0	Day(1–31)*/
	dateAccessed += std::to_string(((entry->aDate >> 9) & 0x7F) + 1980) + "-";
	dateAccessed += std::to_string((entry->aDate >> 5) & 0xF) + "-";
	dateAccessed += std::to_string(entry->aDate & 0x1F);

	// get attributes
	std::string attributes = "";
	if (entry->attr & 0x01) {
		attributes += "Read-only, ";
	}
	if (entry->attr & 0x02) {
		attributes += "Hidden, ";
	}
	if (entry->attr & 0x04) {
		attributes += "System, ";
	}
	if (entry->attr & 0x08) {
		attributes += "Volume label, ";
	}
	if (entry->attr & 0x10) {
		attributes += "Directory, ";
	}
	if (entry->attr & 0x20) {
		attributes += "Archive, ";
	}
	if (entry->attr & 0x40) {
		attributes += "Device, ";
	}
	if (entry->attr & 0x80) {
		attributes += "Reserved, ";
	}
	attributes.erase(attributes.find_last_not_of(", ") + 1);

	// get cluster start
	std::string clusterStart = std::to_string(entry->firstClusHi << 16 | entry->firstClusLo);



	item->setText(1, QString::fromStdString(type));
	item->setText(2, QString::fromStdString(size));
	item->setText(3, QString::fromStdString(dateTimeCreated));
	item->setText(4, QString::fromStdString(dateTimeModified));
	item->setText(5, QString::fromStdString(dateAccessed));
	item->setText(6, QString::fromStdString(attributes));
	item->setText(7, QString::fromStdString(clusterStart));

	item->setTextAlignment(2, Qt::AlignRight);

	
}

std::wstring TreeFolderGUI::getNameLFN(std::stack<FAT32_LFN_DirectoryEntry> stackLFN) {
	std::wstring name = L"";
	while (!stackLFN.empty()) {
		FAT32_LFN_DirectoryEntry entry = stackLFN.top();
		stackLFN.pop();

		// get name
		std::wstring wName = std::wstring((wchar_t*)entry.name1, sizeof(entry.name1) / sizeof(wchar_t));
		wName += std::wstring((wchar_t*)entry.name2, sizeof(entry.name2) / sizeof(wchar_t));
		wName += std::wstring((wchar_t*)entry.name3, sizeof(entry.name3) / sizeof(wchar_t));

		name += wName;
	}

	return name;
}

void TreeFolderGUI::onTreeItemDoubleClicked(QTreeWidgetItem* item, int column) {
	std::string clusterStart = item->text(7).toStdString();
	std::wstring wName = item->text(0).toStdWString();
	wName.erase(std::remove(wName.begin(), wName.end(), '\0'), wName.end());

	if (item->text(1) == "Tập tin") {
		// open file
		// read cluster chain
		std::vector<uint32_t> clusterChain = readClusterChainFat32(std::stoi(clusterStart), drive, (FAT32_BS*)this->bootSector);
		FAT32_BS* bootSector = (FAT32_BS*)this->bootSector;
		const int dataSector = bootSector->rsvdSec + bootSector->numFATs * bootSector->secPerFAT;

		// clear txtPreview
		ui->txtPreview->clear();

		// read file	
		bool loop = true;
		int fileSize = std::stoi(item->text(2).toStdString());
		bool utf8 = false;
		bool utf16_little = false;
		bool utf16_big = false;
		
		for (uint32_t cluster : clusterChain) {



			int startSector = dataSector + (cluster - 2) * bootSector->secPerClus;
			int endSector = startSector + bootSector->secPerClus;

			for (; startSector < endSector && loop; startSector++) {
				uint8_t* sector = new uint8_t[bootSector->bytesPerSec]{ 0 };
				readSector(drive, startSector, sector);

				// read text data until run out of byte in file size property
				// using the best algorithm to read text data
				// read until run out of byte in sector
				// if run out of byte in sector, read next sector
				// if run out of byte in file size property, stop reading
				if (!utf8 && !utf16_little && !utf16_big) {
					if (sector[0] == 0xFF && sector[1] == 0xFE) {
						utf16_little = true;
						ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)sector + 1, (bootSector->bytesPerSec - 2) / 2)));
					}
					else if (sector[0] == 0xFE && sector[1] == 0xFF) {
						utf16_big = true;
						sector += 2;
						ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)sector + 1, (bootSector->bytesPerSec - 2) / 2)));
					} 
					else {
						utf8 = true;
						ui->txtPreview->insertPlainText(QString::fromStdString(std::string((char*)sector, bootSector->bytesPerSec)));
					}
					fileSize -= bootSector->bytesPerSec;
					if (fileSize <= 0) {
						loop = false;
					}
					delete[] sector;
					continue;
				}
				
				if (utf8) {
					ui->txtPreview->insertPlainText(QString::fromStdString(std::string((char*)sector, bootSector->bytesPerSec)));
				}
				else if (utf16_little) {
					ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)sector, bootSector->bytesPerSec / 2)));
				}
				else if (utf16_big) {
					ui->txtPreview->insertPlainText(QString::fromStdWString(std::wstring((wchar_t*)sector, bootSector->bytesPerSec / 2)));
				}

				fileSize -= bootSector->bytesPerSec;
				if (fileSize <= 0) {
					loop = false;
				}

				delete[] sector;
			}
		}


	} ///////////////////////////////////////////////////////
	else {
		uint32_t cluster;

		if (wName == L"..") {
			// remove the last folder in current path
			stackCluster.pop();
			cluster = stackCluster.top();
			currentPath.erase(currentPath.find_last_of(L"\\"));
			currentPath.erase(currentPath.find_last_of(L"\\"));
			currentPath += L"\\";
		}
		else {
			// add name to current path
			cluster = std::stoi(clusterStart);
			currentPath = currentPath + wName + L"\\";
			stackCluster.push(cluster);
		};


		// for testing, output to txtPreview: cluster start, name
		ui->txtPreview->setText(QString::fromStdWString(std::wstring(clusterStart.begin(), clusterStart.end()) + L" " + wName));

		// if type is folder, open it
		if (item->text(1) == "Thư mục" || wName == L"..") {

			displayCurrentFolderFAT32(drive, cluster, (FAT32_BS*)this->bootSector);
			ui->txtPath->setText(QString::fromStdWString(currentPath));
		}
	}

	

}
