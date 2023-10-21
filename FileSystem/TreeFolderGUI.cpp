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

	// make tblProperties read only
	ui->tblProperties->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void TreeFolderGUI::initializeTreeFolderFAT32() {


}

void TreeFolderGUI::initializeTreeFolderNTFS() {
}
