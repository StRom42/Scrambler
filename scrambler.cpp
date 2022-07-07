#include "scrambler.h"

Scrambler::Scrambler(QString path, QString t_crypto_key, QObject* parent)
    : QObject(parent) {
    // инициализируем объект слежения за файлами
    file_watcher = new QFileSystemWatcher;

    // инициализируем кодировщик AES
    coder = new QAESEncryption(QAESEncryption::AES_256, QAESEncryption::ECB);
    crypto_key = QCryptographicHash::hash(t_crypto_key.toLocal8Bit(),
                                          QCryptographicHash::Sha256);

    // говорим, за какой папкой следить
    try {
        set_encrypt_path(path);
    } catch (std::logic_error) {
    }

    // вешаем на событие добавления файла функцию-обработчик
    connect(file_watcher, &QFileSystemWatcher::directoryChanged, this,
            &Scrambler::process_directory_modified);
}

Scrambler::~Scrambler() {
    delete file_watcher;
    delete coder;
}

void Scrambler::stop() {
    file_watcher->blockSignals(true);
}

void Scrambler::set_encrypt_path(QString path) {
    // инициализируем путь до шифруемой папки
    if (path.isEmpty()) {
        throw std::logic_error("Wrong path");
    }
    path_to_encoding_dir = QDir(path);

    // кодируем ее имя и создаем папку с шифрованным содержимым если ее нет
    QString encoding_dir_name = path_to_encoding_dir.dirName();
    QString encoded_dir_name = encode_name(encoding_dir_name);
    path_to_encoded_dir = QDir(path.chopped(encoding_dir_name.size()));

    if (!path_to_encoded_dir.cd(encoded_dir_name)) {
        if (!path_to_encoding_dir.exists()) {
            throw std::logic_error("Wrong path");
        }
        path_to_encoded_dir.mkdir(encoded_dir_name);
    }

    if (!file_watcher->directories().isEmpty()) {
        file_watcher->removePaths(file_watcher->directories());
    }

    file_watcher->addPath(path_to_encoding_dir.path());
    encode_files();
}

void Scrambler::set_decrypt_path(QString path) {
    // инициализируем путь до папки с шифрованным содержимым
    if (path.isEmpty()) {
        throw std::logic_error("Wrong path");
    }
    path_to_encoded_dir = QDir(path);

    // декодируем ее имя и создаем шифруемую папку если ее нет
    QString decoding_dir_name = path_to_encoded_dir.dirName();
    QString decoded_dir_name = decode_name(decoding_dir_name);
    path_to_encoding_dir = QDir(path.chopped(decoding_dir_name.size()));

    if (!path_to_encoding_dir.cd(decoded_dir_name)) {
        if (!path_to_encoded_dir.exists()) {
            throw std::logic_error("Wrong path");
        }
        path_to_encoding_dir.mkdir(decoded_dir_name);
    }

    decode_files();
}

QString Scrambler::get_path_to_encoding_dir() const {
    return path_to_encoding_dir.path();
}

QString Scrambler::get_path_to_decoding_dir() const {
    return path_to_encoded_dir.path();
}

QString Scrambler::encode_name(const QString& name) {
    return prefix + name;
    //    QByteArray encoded_sequence = coder->encode(name.toLocal8Bit(),
    //    crypto_key); QString encoded_name =
    //    QString::fromLocal8Bit(encoded_sequence); return encoded_name;
}

QString Scrambler::decode_name(const QString& name) {
    return name.mid(prefix.size());
    //    QByteArray decoded_sequence = coder->decode(name.toLocal8Bit(),
    //    crypto_key); QString decoded_name =
    //    QString::fromLocal8Bit(decoded_sequence); return decoded_name;
}

void Scrambler::encode_file(const QString& path) {
    // кодируем имя файла и создаем файл куда будем шифровать
    QFile encoding_file(path);
    QFileInfo info(encoding_file);
    if (!path.isEmpty() && encoding_file.open(QIODevice::ReadOnly)) {
        QString file_name = info.fileName();
        QString encoded_name = encode_name(file_name);
        QFile encoded_file(path_to_encoded_dir.filePath(encoded_name));
        if (!encoded_file.exists()) {
            encoded_file.open(QIODevice::WriteOnly);
            while (!encoding_file.atEnd()) {
                QByteArray encoding_chunk = encoding_file.read(max_chunk_len);
                QByteArray encoded_chunk =
                    coder->encode(encoding_chunk, crypto_key);
                encoded_file.write(encoded_chunk);
            }
        }
    }
}

void Scrambler::decode_file(const QString& path) {
    // декодируем имя файла и создаем файл куда будем дешифровать
    QFile decoding_file(path);
    QFileInfo info(decoding_file);
    if (!path.isEmpty() && decoding_file.open(QIODevice::ReadOnly)) {
        QString file_name = info.fileName();
        QString decoded_name = decode_name(file_name);
        QFile decoded_file(path_to_encoding_dir.filePath(decoded_name));

        if (!decoded_file.exists()) {
            decoded_file.open(QIODevice::WriteOnly);
            while (!decoding_file.atEnd()) {
                QByteArray decoding_chunk = decoding_file.read(max_chunk_len);
                QByteArray decoded_chunk =
                    coder->decode(decoding_chunk, crypto_key);
                decoded_file.write(decoded_chunk);
            }
        }
    }
}

void Scrambler::encode_files() {
    // кодируем файлы в шифруемой папке
    QDirIterator it(path_to_encoding_dir.path(), QStringList() << "*",
                    QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        encode_file(it.next());
    }
}

void Scrambler::decode_files() {
    // декодируем файлы в дешифруемой папке
    QDirIterator it(path_to_encoded_dir.path(), QStringList() << "*",
                    QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        decode_file(it.next());
    }
}

void Scrambler::process_directory_modified(const QString& path) {
    // функция-обработчик добавления файла
    Q_UNUSED(path);
    encode_files();
}
