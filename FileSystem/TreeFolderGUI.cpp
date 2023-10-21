#include "TreeFolderGUI.h"

TreeFolderGUI::TreeFolderGUI(QWidget* parent, void* bootSector, std::string drive)
	: QMainWindow(parent)
	, ui(new Ui::TreeFolderGUIClass())
	, bootSector(bootSector)
	, drive(drive)
{

	ui->setupUi(this);
	this->setWindowTitle("Tree Folder");

	initializeTreeFolder();
}

TreeFolderGUI::~TreeFolderGUI()
{
	delete ui;
}

void TreeFolderGUI::initializeTreeFolder() {
	FileSystem fileSystem = readFileSystemType((uint8_t*)bootSector);
	if (fileSystem == FileSystem::FAT32) {
		initializeTreeFolderFAT32();
	}
	else if (fileSystem == FileSystem::NTFS) {
		initializeTreeFolderNTFS();
	}
	else {
		throw "Hệ thống tập tin không được hỗ trợ!";
	}

}

void TreeFolderGUI::initializeTreeFolderFAT32() {
	FAT32_BS* bootSe = (FAT32_BS*)bootSector;
	uint32_t rootCluster = bootSe->rootClus;
	std::vector<uint32_t> clusterChain = readClusterChainFat32(rootCluster, drive, bootSe);

	// for each cluster in the cluster chain, read the directory entries
	bool loop = true;
	const int dataSector = bootSe->rsvdSec + bootSe->numFATs * bootSe->secPerFAT;
	std::stack<FAT32_LFN_DirectoryEntry> stackLFN;

	for (uint32_t cluster : clusterChain) {
		int startSector = dataSector + (cluster - 2) * bootSe->secPerClus;
		int endSector = startSector + bootSe->secPerClus;
		for (; startSector < endSector; startSector++) {
			uint8_t* sector = new uint8_t[bootSe->bytesPerSec] { 0 };
			readSector(drive, startSector, sector);

			// read directory entries
			// if the first byte of the entry is 0xE5, the entry is deleted, skip it
			// if the first byte of the entry is 0x00, the entry is empty, skip it
			// create a std::stack to store the entries (if long file name, push to stack)
			for (int offset = 0; offset < bootSe->bytesPerSec; offset += 32) {
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
		name = wFileName + L"." + wFileExt;
	}

	// my name string may contain 0xFFFF at the end, so I have to trim it
	name.erase(name.find_last_not_of(L'\xFFFF') + 1);

	// get type
	std::string type = "";
	if (entry->attr & 0x10) {
		type = "Thư mục";
	}
	else {
		type = "Tập tin";
	}

	// get size
	std::string size = std::to_string(entry->fileSize);

	// get date created
	std::string dateCreated = "";
	
	// get date modified
	std::string dateModified = "";

	// get date accessed
	std::string dateAccessed = "";

	// get attributes
	std::string attributes = toHexString(&entry->attr, sizeof(entry->attr));

	// get cluster start
	std::string clusterStart = std::to_string(entry->firstClusHi << 16 | entry->firstClusLo);

	// add to tree widget
	QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeFolder);
	item->setText(0, QString::fromStdWString(name));
	item->setText(1, QString::fromStdString(type));
	item->setText(2, QString::fromStdString(size));
	item->setText(3, QString::fromStdString(dateCreated));
	item->setText(4, QString::fromStdString(dateModified));
	item->setText(5, QString::fromStdString(dateAccessed));
	item->setText(6, QString::fromStdString(attributes));
	item->setText(7, QString::fromStdString(clusterStart));





	
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
