#pragma once

#include <QMainWindow>
#include <QStandardItemModel>
#include "ui_FAT32BootSectorGUI.h"
#include "FAT32.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FAT32BootSectorGUIClass; };
QT_END_NAMESPACE

class FAT32BootSectorGUI : public QMainWindow
{
	Q_OBJECT

public:
	FAT32BootSectorGUI(QWidget *parent = nullptr, FAT32_BS* fat32_BS = nullptr);
	~FAT32BootSectorGUI();

private:
	Ui::FAT32BootSectorGUIClass *ui;
	FAT32_BS* fat32_BS;
	void initializeGUI();
	void initializeBootSectorInfo();
	void initializeFATInfo();
};
