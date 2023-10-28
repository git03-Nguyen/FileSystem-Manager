#pragma once


#include <QMainWindow>
#include <QMessageBox>
#include "ui_TreeFolderGUI.h"

#include <string>
#include <stack>

#include "ReadSector.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TreeFolderGUIClass; };
QT_END_NAMESPACE

class TreeFolderGUI : public QMainWindow
{
	Q_OBJECT

public:
	TreeFolderGUI(QWidget *parent = nullptr, uint8_t* bootSector = nullptr, std::string drive = "");
	~TreeFolderGUI();

private:
	Ui::TreeFolderGUIClass *ui;
	uint8_t* bootSector;
	FileSystem fileSystemType;
	std::string drive;
	std::wstring currentPath;
	std::stack<uint32_t> stackClusters; // used by FAT32
	std::stack<uint64_t> stackMftEntries; // used by NTFS

	// Initialize GUI
	void initializeTreeFolder();
	void initializeTreeFolderFAT32();
	void initializeTreeFolderNTFS();

	// Display current folder
	void displayCurrentFolder(std::string drive, uint32_t cluster, FAT32_BS* bootSector);
	void displayCurrentFolder(std::string drive, uint64_t entryNum, NTFS_BS* bootSector);

	// Helper to add file/folder to tree
	void addItemToTreeFAT32(const FAT32_DirectoryEntry& entry, std::wstring name = L"");
	void addItemToTreeNTFS(NTFS_MftEntry* entry, const std::wstring& fileName, int mftEntryNum);

	// Handle double click on file/folder
	void onTreeItemDoubleClickedFAT32(QTreeWidgetItem* item, int column);
	void onTreeItemDoubleClickedNTFS(QTreeWidgetItem* item, int column);

	// Helper to get name of file/folder FAT32
	std::wstring getFileNameFAT32(std::stack<FAT32_LFN_DirectoryEntry>& stackLFN);

	// Helper to open file/folder FAT32
	void openFileFAT32(QTreeWidgetItem* item);
	void openFolderFAT32(QTreeWidgetItem* item);

	// Helper to open file/folder NTFS
	void openFileNTFS(QTreeWidgetItem* item);
	void openFolderNTFS(QTreeWidgetItem* item);

};
