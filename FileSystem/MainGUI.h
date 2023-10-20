#pragma once

#include <QtWidgets/QMainWindow>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>

#include "ui_MainGUI.h"
#include "ReadSector.h"
#include "FAT32_NTFS.h"

#include "BootSectorGUI.h"
#include "TreeFolderGUI.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainGUIClass; };
QT_END_NAMESPACE

class MainGUI : public QMainWindow
{
    Q_OBJECT

public:
    MainGUI(QWidget *parent = nullptr);
    ~MainGUI();

private:
    Ui::MainGUIClass *ui;
    BootSectorGUI* bootSectorGUI;
    TreeFolderGUI* treeFolderGUI;
    void initializeFeatureButtons();
    void onBtnReadDiskClicked();
    void onBtnDisplayTreeClicked();
};
