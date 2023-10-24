#pragma once

#include <QMainWindow>
#include <QStandardItemModel>
#include "ui_BootSectorGUI.h"

#include "ReadSector.h"

QT_BEGIN_NAMESPACE
namespace Ui { class BootSectorGUIClass; };
QT_END_NAMESPACE

class BootSectorGUI : public QMainWindow
{
	Q_OBJECT

public:
	BootSectorGUI(QWidget *parent = nullptr, uint8_t* bootSector = nullptr);
	~BootSectorGUI();

private:
	Ui::BootSectorGUIClass *ui;
	uint8_t* bootSector;

	// Initialize GUI
	void initializeGUI();
	void initializeRawDataGUI();
	void initializeDetailGUI();

	// Print the data to the GUI
	void initializeFat32Info();
	void initializeNTFSInfo();
	
	// Helper function to add a row to the detail table
	void addDetailRow(std::string name, int offset, std::string value);

};
