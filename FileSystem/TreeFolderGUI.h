#pragma once

#include <QMainWindow>
#include <string>
#include "ui_TreeFolderGUI.h"

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

	void initializeTreeFolder();
	void initializeTreeFolderFAT32();
	void initializeTreeFolderNTFS();
	
	void addTreeFolderItem(QTreeWidgetItem* parent, std::string name, std::string type, std::string size, std::string created, std::string modified, std::string accessed);

	void addTreeFolderItemNTFS(QTreeWidgetItem* parent, std::string name, std::string type, std::string size, std::string created, std::string modified, std::string accessed);
	void addTreeFolderItemFAT32(QTreeWidgetItem* parent, std::string name, std::string type, std::string size, std::string created, std::string modified, std::string accessed);
	void addTreeFolderItemOthers(QTreeWidgetItem* parent, std::string name, std::string type, std::string size, std::string created, std::string modified, std::string accessed);

};
