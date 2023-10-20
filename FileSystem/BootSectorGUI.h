#pragma once

#include <QMainWindow>
#include <QStandardItemModel>
#include "ui_BootSectorGUI.h"

#include "FAT32_NTFS.h"
#include "ReadSector.h"

QT_BEGIN_NAMESPACE
namespace Ui { class BootSectorGUIClass; };
QT_END_NAMESPACE

class BootSectorGUI : public QMainWindow
{
	Q_OBJECT

public:
	BootSectorGUI(QWidget *parent = nullptr, void* bootSector = nullptr);
	~BootSectorGUI();

private:
	Ui::BootSectorGUIClass *ui;
	void* bootSector;
	void initializeGUI();
	void initializeRawData();
	
	void initializeInfo();
	void initializeFat32Info();
	void initializeNTFSInfo();
	
	void addInfoRow(std::string name, int offset, std::string value);
};
