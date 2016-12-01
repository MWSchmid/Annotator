#include "ftploader.h"

#include <QtCore>
#include <QtNetwork>
#include <iostream>
#include <qtiocompressor.h>

ftpLoader::ftpLoader(QObject *parent)
    : QObject(parent)
{
    connect(&ftp, SIGNAL(done(bool)), this, SLOT(ftpDone(bool)));
}

ftpLoader::~ftpLoader()
{
    ftp.abort();
    if (file.exists()) {
        if (file.isOpen()) {
            file.close();
            file.remove();
        }
    }
}

QString ftpLoader::getLocalFile()
{
    return(file.fileName());
}

bool ftpLoader::getFile(const QUrl &url, const QString &localPathString, bool doCompress)
{
    if (!url.isValid()) {
        std::cerr << "Error: Invalid URL" << std::endl;
        return false;
    }

    if (url.scheme() != "ftp") {
        std::cerr << "Error: URL must start with 'ftp:'" << std::endl;
        return false;
    }

    if (url.path().isEmpty()) {
        std::cerr << "Error: URL has no path" << std::endl;
        return false;
    }

    QString localFileName = QFileInfo(url.path()).fileName();
    if (localFileName.isEmpty())
        localFileName = "ftpLoader.out";


    localFileName.prepend("/");
    localFileName.prepend(localPathString);
    if (doCompress) { localFileName.append(".gz"); }
    localFileName = QDir::toNativeSeparators(localFileName);
    localFileName = QDir::cleanPath(localFileName);

    file.setFileName(localFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        std::cerr << "Error: Cannot write file "
                  << qPrintable(file.fileName()) << ": "
                  << qPrintable(file.errorString()) << std::endl;
        return false;
    }

    compressor = new QtIOCompressor(&file);
    compressor->setStreamFormat(QtIOCompressor::GzipFormat); //! TODO - CHECK FOR FORMAT AND HANDLE ERRORS
    if (doCompress && (!compressor->open(QIODevice::WriteOnly))) {
        std::cerr << "Error: Cannot write file "
                  << qPrintable(file.fileName()) << ": "
                  << qPrintable(compressor->errorString()) << std::endl;
        return false;
    } else

    ftp.connectToHost(url.host(), url.port(21));
    ftp.login();
    if (doCompress) { ftp.get(url.path(), compressor); }
    else { ftp.get(url.path(), &file); }
    ftp.close();
    return true;
}

void ftpLoader::ftpDone(bool error)
{
    QString fileName = file.fileName();
    if (error) { qDebug() << "ERROR: " << qPrintable(ftp.errorString()); }
    else { qDebug() << "File downloaded as " << qPrintable(fileName); }
    if (compressor->isOpen()) { compressor->close(); }
    if (file.isOpen()) { file.close(); }
    emit done(fileName);
}
