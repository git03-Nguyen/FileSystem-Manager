#include "MainGUI.h"

MainGUI::MainGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainGUIClass())
{
    ui->setupUi(this);
    initializeFeatureButtons();
}

MainGUI::~MainGUI()
{
    delete ui;
}

void MainGUI::initializeFeatureButtons() {

    // For button ReadDisk
    QHBoxLayout* pLayout1 = new QHBoxLayout();
    QLabel* pIconLabel1 = new QLabel();
    QLabel* pTextLabel1 = new QLabel();

    pIconLabel1->setPixmap(QPixmap("./assets/floppydisk.png"));
    pIconLabel1->setFixedSize(50, 50);
    pIconLabel1->setScaledContents(true);

    pIconLabel1->setAlignment(Qt::AlignCenter);
    pIconLabel1->setMouseTracking(false);
    pIconLabel1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    pTextLabel1->setText("Đọc thông tin phân vùng từ Boot sector");
    pTextLabel1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    pTextLabel1->setWordWrap(true);
    pTextLabel1->setTextInteractionFlags(Qt::NoTextInteraction);
    pTextLabel1->setMouseTracking(false);
    pTextLabel1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pLayout1->addWidget(pIconLabel1);
    pLayout1->addWidget(pTextLabel1);
    pLayout1->setSpacing(20);
    pLayout1->setContentsMargins(10, 5, 5, 5);

    QPushButton* btnReadDisk = ui->btnReadDisk;
    btnReadDisk->setStyleSheet(
        "QPushButton {background-color: transparent; border: none; border-radius: 5px;}"
        "QPushButton:hover {background-color: #e6e6e6; border: 1px solid gray;}"
        "QPushButton:pressed {background-color: #cccccc;}"
    );
    btnReadDisk->setLayout(pLayout1);

    // do the same thing for btnDisplayTree
    QHBoxLayout* pLayout2 = new QHBoxLayout();
    QLabel* pIconLabel2 = new QLabel();
    QLabel* pTextLabel2 = new QLabel();

    pIconLabel2->setPixmap(QPixmap("./assets/folder_yellow.png"));
    pIconLabel2->setFixedSize(50, 50);
    pIconLabel2->setScaledContents(true);

    pIconLabel2->setAlignment(Qt::AlignCenter);
    pIconLabel2->setMouseTracking(false);
    pIconLabel2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    pTextLabel2->setText("Hiển thị cây thư mục và đọc tập tin");
    pTextLabel2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    pTextLabel2->setWordWrap(true);
    pTextLabel2->setTextInteractionFlags(Qt::NoTextInteraction);
    pTextLabel2->setMouseTracking(false);
    pTextLabel2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pLayout2->addWidget(pIconLabel2);
    pLayout2->addWidget(pTextLabel2);
    pLayout2->setSpacing(20);
    pLayout2->setContentsMargins(10, 5, 5, 5);

    QPushButton* btnDisplayTree = ui->btnDisplayTree;
    btnDisplayTree->setStyleSheet(
        "QPushButton {background-color: transparent; border: none; border-radius: 5px;}"
        "QPushButton:hover {background-color: #e6e6e6; border: 1px solid gray;}"
        "QPushButton:pressed {background-color: #cccccc;}"
    );
    btnDisplayTree->setLayout(pLayout2);

    // when click on btnReadDisk, open a dialog to enter drive letter
    connect(btnReadDisk, &QPushButton::clicked, this, &MainGUI::onBtnReadDiskClicked);

}

void MainGUI::onBtnReadDiskClicked() {
	QString drive = QInputDialog::getText(this, "Đọc thông tin phân vùng", "Nhập đường dẫn ổ đĩa (ví dụ: C:)");
	if (drive.isEmpty()) return;

	// convert QString to LPCWSTR
	std::wstring wstr = L"\\\\.\\" + drive.toStdWString();
    LPCWSTR lpcwstr = wstr.c_str();

	// read boot sector
	uint8_t bootSector[_SECTOR_SIZE] = { 0 };

    try {
        DWORD bytesRead = readSector(lpcwstr, 0, bootSector);
        // read file system type
        FileSystem fs = readFileSystemType(bootSector);
        if (fs == FileSystem::Others) {
            QMessageBox::critical(this, "Lỗi", "Hệ thống tập tin không được hỗ trợ!");
            return;
        }

        // parse boot sector into information
        FAT32_BS* fat32_bs = nullptr;
        NTFS_BS* ntfs_bs = nullptr;

        if (fs == FileSystem::FAT32) {
            fat32_bs = (FAT32_BS*)bootSector;
            // display information in a new dialog, make dialog modal
            FAT32BootSectorGUI* fat32BootSectorGUI = new FAT32BootSectorGUI(this, fat32_bs);
            fat32BootSectorGUI->setWindowModality(Qt::ApplicationModal);
            fat32BootSectorGUI->show();
        }
        else if (fs == FileSystem::NTFS) {
            ntfs_bs = (NTFS_BS*)bootSector;
        }
    }
    catch (const char* error) {
        std::string errorCode = "Mã lỗi: " + std::to_string(GetLastError()) + "  ";
        QMessageBox::critical(this, "Lỗi", (std::string(error) + " " + errorCode).c_str());
	}
    catch (...) {
        std::string error = "Mã lỗi: " + std::to_string(GetLastError()) + "  ";
        QMessageBox::critical(this, "Lỗi", ("Đọc không thành công! " + error).c_str());
    }
}
