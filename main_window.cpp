#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // загружаем настройки путей
    settings = new QSettings(QSettings::UserScope);
    if (settings->contains("enc_dir_path")) {
        QString enc_dir_path = settings->value("enc_dir_path").toString();
        ui->catalog_line_edit->setText(enc_dir_path);
    }
    if (settings->contains("dec_dir_path")) {
        QString dec_dir_path = settings->value("dec_dir_path").toString();
        ui->decrypt_catalog_line_edit->setText(dec_dir_path);
    }

    // берем за ключ шифрования - имя текущего пользователя
    QString crypto_key = get_user_name();

    // делаем иконку в трее
    tray_icon = new QSystemTrayIcon(this);
    tray_icon->setIcon(this->style()->standardIcon(QStyle::SP_ComputerIcon));
    tray_icon->show();

    // инициализируем авто-шифратор
    scrambler = new Scrambler("", crypto_key);

    // вешаем на события  их обработчики
    connect(tray_icon, &QSystemTrayIcon::activated, this,
            &MainWindow::process_tray_icon_activated);
    connect(ui->catalog_choose_button, &QPushButton::clicked, this,
            &MainWindow::process_catalog_button_clicked);
    connect(ui->catalog_choose_button_2, &QPushButton::clicked, this,
            &MainWindow::process_dec_catalog_button_clicked);
    connect(ui->remember_button, &QPushButton::clicked, this,
            &MainWindow::process_remember_button_clicked);
    connect(ui->decrypt_button, &QPushButton::clicked, this,
            &MainWindow::process_decrypt_button_clicked);
}

MainWindow::~MainWindow() {
    delete ui;
    delete tray_icon;
    delete scrambler;
}

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message,
                             qintptr* result) {
    // обрабатывем событие нажатия WIN+L под кодом 800

    Q_UNUSED(eventType)
    Q_UNUSED(result)

    MSG* msg = reinterpret_cast<MSG*>(message);

    if (msg->message == 800) {
        this->close();
        return true;
    }
    return false;
}

QString MainWindow::get_user_name() {
    // получаем имя текущего пользователя
    QString user_name = "";
    TCHAR ac_user_name[500] = {0};
    DWORD user_name_size = sizeof(ac_user_name);
    if (GetUserNameW(ac_user_name, &user_name_size)) {
        user_name = QString::fromWCharArray(ac_user_name);
    }
    return user_name;
}

void MainWindow::save_settings() {
    // сохраняем настройки путей
    QString enc_dir_path = scrambler->get_path_to_encoding_dir();
    settings->setValue("enc_dir_path", enc_dir_path);
    ui->catalog_line_edit->setText(enc_dir_path);

    QString dec_dir_path = scrambler->get_path_to_decoding_dir();
    settings->setValue("dec_dir_path", dec_dir_path);
    ui->decrypt_catalog_line_edit->setText(dec_dir_path);
    settings->sync();
}

void MainWindow::encode_before_quit() {
    // выполняем удаление шифруемой папки перед выходом из программы
    QString enc_dir_path = scrambler->get_path_to_encoding_dir();
    QDir enc_dir(enc_dir_path);
    enc_dir.removeRecursively();
}

void MainWindow::process_tray_icon_activated(
    QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Trigger: {
        !isVisible() ? show() : hide();
    } break;
    default: break;
    }
}

void MainWindow::process_catalog_button_clicked() {
    QString path = QFileDialog::getExistingDirectory(
        this, tr("Выберите каталог для шифрации"), "D:/",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->catalog_line_edit->setText(path);
}

void MainWindow::process_dec_catalog_button_clicked() {
    QString path = QFileDialog::getExistingDirectory(
        this, tr("Выберите каталог для шифрации"), "D:/",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->decrypt_catalog_line_edit->setText(path);
}

void MainWindow::process_remember_button_clicked() {
    QString path = ui->catalog_line_edit->text();
    try {
        scrambler->set_encrypt_path(path);
        save_settings();
    } catch (std::logic_error) {
        QMessageBox::critical(this, "Неверный путь",
                              "Путь, указанный Вами не существует");
    }
}

void MainWindow::process_decrypt_button_clicked() {
    QString path = ui->decrypt_catalog_line_edit->text();
    try {
        scrambler->set_decrypt_path(path);
        save_settings();
    } catch (std::logic_error) {
        QMessageBox::critical(this, "Неверный путь",
                              "Путь, указанный Вами не существует");
    }
}
