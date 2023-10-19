#pragma once

#include <QtWidgets/QMainWindow>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>

#include "ui_MainGUI.h"
#include "ReadSector.h"
#include "FAT32.h"
#include "NTFS.h"

#include "FAT32BootSectorGUI.h"

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
    void initializeFeatureButtons();
    void onBtnReadDiskClicked();
};
