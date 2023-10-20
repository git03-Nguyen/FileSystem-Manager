#pragma once

#include <QMainWindow>
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
	TreeFolderGUI(QWidget *parent = nullptr, void* bootSector = nullptr);
	~TreeFolderGUI();

private:
	Ui::TreeFolderGUIClass *ui;
	void* bootSector;
};
