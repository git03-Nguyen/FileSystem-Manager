#include "MainGUI.h"

MainGUI::MainGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainGUIClass())
    , bootSectorGUI(nullptr)
    , treeFolderGUI(nullptr)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon("./assets/os.png"));

    initializeReadDisk();
    initializeDisplayTree();

    // The authors are: 
    // 1. Nguyễn Đình Ánh - 21120171
    // 2. Nguyễn Tuấn Đạt - 21120172
    // 3. Nguyễn Trần Thiên Phúc - 21120114
    // Đồ án môn học Hệ điều hành
    // Đề tài: Đọc thông tin phân vùng và hiển thị cây thư mục gốc
    connect(ui->actionInfo, &QAction::triggered, this, [=]() {
		QMessageBox::information(this, "Thông tin", "Đồ án môn học Hệ điều hành\nĐề tài: Hệ thống quản lý tập tin\n\nNhóm sinh viên thực hiện:\n1. Nguyễn Đình Ánh - 21120171\n2. Nguyễn Tuấn Đạt - 21120172\n3. Nguyễn Trần Thiên Phúc - 21120114");
	});

}

MainGUI::~MainGUI()
{
    delete ui;
    delete bootSectorGUI;
    delete treeFolderGUI;
    for (auto& gui : listGUI) delete gui;
}

void MainGUI::initializeReadDisk() {

    // Style for button ReadDisk
    QHBoxLayout* pLayout = new QHBoxLayout();
    QLabel* pIconLabel = new QLabel();
    QLabel* pTextLabel = new QLabel();
    listGUI.push_back(pIconLabel);
    listGUI.push_back(pTextLabel);
    listGUI.push_back(pLayout);

    pIconLabel->setPixmap(QPixmap("./assets/floppydisk.png"));
    pIconLabel->setFixedSize(50, 50);
    pIconLabel->setScaledContents(true);
    pIconLabel->setAlignment(Qt::AlignCenter);
    pIconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    pTextLabel->setText("Đọc thông tin phân vùng từ Boot sector");
    pTextLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    pTextLabel->setWordWrap(true);
    pTextLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    pTextLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pLayout->addWidget(pIconLabel);
    pLayout->addWidget(pTextLabel);
    pLayout->setSpacing(20);
    pLayout->setContentsMargins(10, 5, 5, 5);

    ui->btnReadDisk->setStyleSheet(
        "QPushButton {background-color: transparent; border: none; border-radius: 5px;}"
        "QPushButton:hover {background-color: #e6e6e6; border: 1px solid gray;}"
        "QPushButton:pressed {background-color: #cccccc;}"
    );

    ui->btnReadDisk->setLayout(pLayout);

    // when click on btnReadDisk, call function handle onBtnReadDiskClicked
    connect(ui->btnReadDisk, &QPushButton::clicked, this, &MainGUI::onBtnReadDiskClicked);
}

void MainGUI::initializeDisplayTree() {

    // Style for button DisplayTree
    QHBoxLayout* pLayout = new QHBoxLayout();
    QLabel* pIconLabel = new QLabel();
    QLabel* pTextLabel = new QLabel();
    listGUI.push_back(pIconLabel);
    listGUI.push_back(pTextLabel);
    listGUI.push_back(pLayout);

    pIconLabel->setPixmap(QPixmap("./assets/folder_yellow.png"));
    pIconLabel->setFixedSize(50, 50);
    pIconLabel->setScaledContents(true);
    pIconLabel->setAlignment(Qt::AlignCenter);
    pIconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    pTextLabel->setText("Hiển thị cây thư mục và đọc tập tin");
    pTextLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    pTextLabel->setWordWrap(true);
    pTextLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    pTextLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pLayout->addWidget(pIconLabel);
    pLayout->addWidget(pTextLabel);
    pLayout->setSpacing(20);
    pLayout->setContentsMargins(10, 5, 5, 5);

    ui->btnDisplayTree->setStyleSheet(
        "QPushButton {background-color: transparent; border: none; border-radius: 5px;}"
        "QPushButton:hover {background-color: #e6e6e6; border: 1px solid gray;}"
        "QPushButton:pressed {background-color: #cccccc;}"
    );

    ui->btnDisplayTree->setLayout(pLayout);

    // when click on btnDisplayTree, call function handle onBtnDisplayTreeClicked
    connect(ui->btnDisplayTree, &QPushButton::clicked, this, &MainGUI::onBtnDisplayTreeClicked);
}

void MainGUI::onBtnReadDiskClicked() {
    // Open a dialog to enter drive letter
	QString drive = QInputDialog::getText(this, "Đọc thông tin phân vùng", "Nhập đường dẫn ổ đĩa (ví dụ: F:)");
	if (drive.isEmpty()) return;

    try {
        // read boot sector
        uint8_t bootSector[BOOT_SECTOR_SIZE] = { 0 };
        DWORD bytesRead = readByte(drive.toStdString(), 0, bootSector);

        // read file system type
        FileSystem fs = getFileSystemType(bootSector);
        if (fs == FileSystem::UNKNOWN) {
            QMessageBox::critical(this, "Lỗi", "Hệ thống tập tin không được hỗ trợ!");
            return;
        }

        // display information of partition in a new dialog
        if (bootSectorGUI) delete bootSectorGUI;
        bootSectorGUI = new BootSectorGUI(this, bootSector);
        bootSectorGUI->setWindowModality(Qt::ApplicationModal);
        bootSectorGUI->show();
    
    }
    catch (const char* error) {
        int errorCode = GetLastError();
        if (errorCode == 5) {
            QMessageBox::critical(this, "Lỗi", "Vui lòng khởi chạy lại dưới quyền admin!");
        }
        else {
            std::string errorStr = "Mã lỗi: " + std::to_string(errorCode) + "  ";
            QMessageBox::critical(this, "Lỗi", (std::string(error) + " " + errorStr).c_str());
        }
	}
    catch (...) {
        std::string error = "Mã lỗi: " + std::to_string(GetLastError()) + "  ";
        QMessageBox::critical(this, "Lỗi", ("Đọc không thành công! " + error).c_str());
    }
}

void MainGUI::onBtnDisplayTreeClicked() {
    // Open a dialog to enter drive letter
    QString drive = QInputDialog::getText(this, "Hiển thị cây thư mục gốc", "Nhập đường dẫn ổ đĩa (ví dụ: F:)");
    if (drive.isEmpty()) return;

    try {
        // read boot sector
        uint8_t bootSector[BOOT_SECTOR_SIZE] = { 0 };
        DWORD bytesRead = readByte(drive.toStdString(), 0, bootSector);

        // read file system type
        FileSystem fs = getFileSystemType(bootSector);
        if (fs == FileSystem::UNKNOWN) {
            QMessageBox::critical(this, "Lỗi", "Hệ thống tập tin không được hỗ trợ!");
            return;
        }

        // display tree folder in a new dialog
        if (treeFolderGUI) delete treeFolderGUI;
        treeFolderGUI = new TreeFolderGUI(this, bootSector, drive.toStdString());
        treeFolderGUI->setWindowModality(Qt::ApplicationModal);
        treeFolderGUI->show();

    }
    catch (const char* error) {
        int errorCode = GetLastError();
        if (errorCode == 5) {
            QMessageBox::critical(this, "Lỗi", "Vui lòng khởi chạy lại dưới quyền admin!");
        }
        else {
            std::string errorStr = "Mã lỗi: " + std::to_string(errorCode) + "  ";
            QMessageBox::critical(this, "Lỗi", (std::string(error) + " " + errorStr).c_str());
        }
    }
    catch (...) {
        std::string error = "Mã lỗi: " + std::to_string(GetLastError()) + "  ";
        QMessageBox::critical(this, "Lỗi", ("Đọc không thành công! " + error).c_str());
    }


}


