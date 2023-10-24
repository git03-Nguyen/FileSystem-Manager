#pragma once

#include <QtWidgets/QMainWindow>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include "ui_MainGUI.h"

#include <vector>

#include "ReadSector.h"
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
    std::vector<QObject*> listGUI;

    // Initialize GUI components
    void initializeReadDisk();
    void initializeDisplayTree();

    // Function handle for button clicked
    void onBtnReadDiskClicked();
    void onBtnDisplayTreeClicked();
};
