#ifndef SCRAMBLER_H
#define SCRAMBLER_H

#include "qaesencryption.h"
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QObject>
#include <QSettings>

class Scrambler : public QObject {
    Q_OBJECT
public:
    explicit Scrambler(QString path = "", QString t_crypto_key = "",
                       QObject* parent = nullptr);
    ~Scrambler();

    void stop();

    void set_encrypt_path(QString path);
    void set_decrypt_path(QString path);

    QString get_path_to_encoding_dir() const;
    QString get_path_to_decoding_dir() const;

private:
    QString encode_name(const QString& name);
    QString decode_name(const QString& name);

    void encode_file(const QString& path);
    void decode_file(const QString& path);

    void encode_files();
    void decode_files();

private slots:
    void process_directory_modified(const QString& path);

private:
    QDir path_to_encoding_dir;
    QDir path_to_encoded_dir;
    QByteArray crypto_key;
    QFileSystemWatcher* file_watcher;
    QAESEncryption* coder;
    qint64 max_chunk_len{1024};
    QString prefix{"encoded_"};
    QString suffix{"_encoded"};
};

#endif // SCRAMBLER_H
