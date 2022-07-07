#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "scrambler.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QWidget>
#include <windows.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    bool nativeEvent(const QByteArray& eventType, void* message,
                     qintptr* result) override;
    QString get_user_name();
    void save_settings();

public slots:
    void encode_before_quit();

private slots:
    void process_tray_icon_activated(QSystemTrayIcon::ActivationReason reason);
    void process_catalog_button_clicked();
    void process_dec_catalog_button_clicked();
    void process_remember_button_clicked();
    void process_decrypt_button_clicked();

private:
    Ui::MainWindow* ui;
    QSystemTrayIcon* tray_icon;
    Scrambler* scrambler;
    QSettings* settings;
};
#endif // MAIN_WINDOW_H
