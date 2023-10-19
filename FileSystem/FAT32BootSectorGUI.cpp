#include "FAT32BootSectorGUI.h"

FAT32BootSectorGUI::FAT32BootSectorGUI(QWidget *parent, FAT32_BS* fat32BS)
	: QMainWindow(parent)
	, ui(new Ui::FAT32BootSectorGUIClass())
	, fat32_BS(fat32BS)
{
	ui->setupUi(this);
	initializeGUI();
}

FAT32BootSectorGUI::~FAT32BootSectorGUI()
{
	delete ui;
}

void FAT32BootSectorGUI::initializeGUI() {
	// make the tab 1 active
	QTabWidget* tabWidget = ui->tabWidget;
	tabWidget->setCurrentIndex(0);

	initializeBootSectorInfo();
	initializeFATInfo();
}

void FAT32BootSectorGUI::initializeBootSectorInfo() {
	uint8_t* p = (uint8_t*)fat32_BS;

	// get the table view
	QTableView* tblRawData = ui->tblRawData;
	
	// how should we display the byte array into the gui like a hex editor?
	// we can use QTableView
	// okay, let's try it
	// first, we need to create a model
	QStandardItemModel* model = new QStandardItemModel();
	model->setColumnCount(16);
	model->setRowCount(16);

	// set the header
	for (int i = 0; i < 16; i++) {
		model->setHeaderData(i, Qt::Horizontal, QString::number(i, 16).toUpper());
		model->setHeaderData(i, Qt::Vertical, QString::number(i * 16, 16).toUpper());
	}

	// set the data
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++, p++) {
			// if < 16, then we need to add a 0 before the number
			if (*p < 16) {
				model->setData(model->index(i, j), "0" + QString::number(*p, 16).toUpper());
			}
			else {
				model->setData(model->index(i, j), QString::number(*p, 16).toUpper());
			}
		}
	}

	// set the model
	tblRawData->setModel(model);

	// set the size of each column
	for (int i = 0; i < 16; i++) {
		tblRawData->setColumnWidth(i, 30);
	}

	// set the size of each row
	for (int i = 0; i < 16; i++) {
		tblRawData->setRowHeight(i, 20);
	}

	// set the font
	QFont font;
	font.setFamily("Consolas");
	font.setPointSize(10);
	tblRawData->setFont(font);

	// set the alignment
	tblRawData->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
	tblRawData->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

	// set the border
	tblRawData->setShowGrid(false);
	tblRawData->setFrameShape(QFrame::NoFrame);
	tblRawData->setFrameShadow(QFrame::Plain);

	// set the selection mode
	tblRawData->setSelectionMode(QAbstractItemView::NoSelection);

	// set the background color
	tblRawData->setStyleSheet("QTableView {background-color: #ffffff;}");

	// set the text color
	tblRawData->setStyleSheet("QTableView {color: #000000;}");

	// set the header background color
	tblRawData->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color: #ffffff;}");
	tblRawData->verticalHeader()->setStyleSheet("QHeaderView::section {background-color: #ffffff;}");

	// set the header text color
	tblRawData->horizontalHeader()->setStyleSheet("QHeaderView::section {color: #000000;}");
	tblRawData->verticalHeader()->setStyleSheet("QHeaderView::section {color: #000000;}");

	// set the header border
	tblRawData->horizontalHeader()->setStyleSheet("QHeaderView::section {border: 1px solid #000000;}");
	tblRawData->verticalHeader()->setStyleSheet("QHeaderView::section {border: 1px solid #000000;}");

	// set the header size
	tblRawData->horizontalHeader()->setDefaultSectionSize(30);
	tblRawData->verticalHeader()->setDefaultSectionSize(20);

	// set the header font
	QFont headerFont;
	headerFont.setFamily("Consolas");
	headerFont.setPointSize(10);
	tblRawData->horizontalHeader()->setFont(headerFont);
	tblRawData->verticalHeader()->setFont(headerFont);

	// set the header alignment
	tblRawData->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
	tblRawData->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

	// set the header selection mode
	tblRawData->horizontalHeader()->setSelectionMode(QAbstractItemView::NoSelection);
	tblRawData->verticalHeader()->setSelectionMode(QAbstractItemView::NoSelection);

	// add margin to tab to make table center
	QWidget* tab = ui->tab;
	QHBoxLayout* layout = new QHBoxLayout(tab);
	layout->setContentsMargins(20, 20, 20, 20);
	layout->addWidget(tblRawData);
	tab->setLayout(layout);

	// make it overflow y but not overflow x
	tblRawData->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	tblRawData->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	// set the size of the table
	tblRawData->setFixedSize(16 * 30 + 2, 16 * 20 + 2);

	// set the size of the tab
	tab->setFixedSize(16 * 30 + 2 + 40, 16 * 20 + 2 + 40);

	// set the size of the window
	this->setFixedSize(16 * 30 + 2 + 40 + 20, 16 * 20 + 2 + 40 + 20);

	// set the title
	this->setWindowTitle("Boot Sector");

	// set the icon
	this->setWindowIcon(QIcon("./assets/folder_yellow.png"));





	



}

void FAT32BootSectorGUI::initializeFATInfo() {
}
