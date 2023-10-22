#pragma once

#include <QMainWindow>
#include <string>
#include "ui_TreeFolderGUI.h"

#include <stack>

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
	void* bootSector;
	std::string drive;
	std::string currentPath;
	std::vector<std::wstring> nameForEachItem;


	void initializeTreeFolder();
	void initializeTreeFolderFAT32();
	void initializeTreeFolderNTFS();
	
	void addItemToTree(FAT32_DirectoryEntry* entry, std::wstring name = L"");

	std::wstring getNameLFN(std::stack<FAT32_LFN_DirectoryEntry> stackLFN);

};
