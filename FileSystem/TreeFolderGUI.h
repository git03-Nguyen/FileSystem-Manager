#pragma once

#include <QMainWindow>
#include <string>
#include "ui_TreeFolderGUI.h"

#include <stack>
#include <iostream>

#include "FAT32_NTFS.h"
#include "ReadSector.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TreeFolderGUIClass; };
QT_END_NAMESPACE

class TreeFolderGUI : public QMainWindow
{
	Q_OBJECT

public:
	TreeFolderGUI(QWidget *parent = nullptr, void* bootSector = nullptr, std::string drive = "");
	~TreeFolderGUI();

private:
	Ui::TreeFolderGUIClass *ui;
	uint8_t* bootSector;
	std::string drive;
	std::wstring currentPath;
	std::stack<uint32_t> stackCluster;

	void initializeTreeFolder();

	void initializeTreeFolderFAT32();
	void displayCurrentFolderFAT32(std::string drive, uint32_t cluster, FAT32_BS* bootSector);

	void initializeTreeFolderNTFS();
	
	void addItemToTree(FAT32_DirectoryEntry* entry, std::wstring name = L"");

	std::wstring getNameLFN(std::stack<FAT32_LFN_DirectoryEntry> stackLFN);


	void onTreeItemDoubleClicked(QTreeWidgetItem* item, int column);

};
