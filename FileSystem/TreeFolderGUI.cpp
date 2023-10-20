#include "TreeFolderGUI.h"

TreeFolderGUI::TreeFolderGUI(QWidget *parent, void* bootSector)
	: QMainWindow(parent)
	, ui(new Ui::TreeFolderGUIClass())
	, bootSector(bootSector)
{
	ui->setupUi(this);
	// set title
	this->setWindowTitle("Tree Folder");
}

TreeFolderGUI::~TreeFolderGUI()
{
	delete ui;
}
