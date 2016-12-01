#include "httploader.h"

#include <QtCore>
#include <QtNetwork>
#include <iostream>
#include <qtiocompressor.h>

httpLoader::httpLoader(QObject *parent)
    : QObject(parent)
{
    connect(&http, SIGNAL(done(bool)), this, SLOT(httpDone(bool)));
}

httpLoader::~httpLoader()
{
    http.abort();
    if (file.exists()) {
        if (file.isOpen()) {
            file.close();
            file.remove();
        }
    }
}

QString httpLoader::getLocalFile()
{
    return(file.fileName());
}

bool httpLoader::getFile(const QUrl &url, const QString &localPathString, bool doCompress)
{
    if (!url.isValid()) {
        std::cerr << "Error: Invalid URL" << std::endl;
        return false;
    }

    if (url.scheme() != "http") {
        std::cerr << "Error: URL must start with 'http:'" << std::endl;
        return false;
    }

    if (url.path().isEmpty()) {
        std::cerr << "Error: URL has no path" << std::endl;
        return false;
    }

    QString localFileName = QFileInfo(url.path()).fileName();
    if (localFileName.isEmpty())
        localFileName = "httpLoader.out";


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
    }

    http.setHost(url.host(), url.port(80));
    if (url.hasQuery()) {
        if (doCompress) { http.post(url.path(), QUrl::toPercentEncoding(url.query()), compressor); }
        else { http.post(url.path(), QUrl::toPercentEncoding(url.query()), &file); }
    } else {
        if (doCompress) { http.get(url.path(), compressor); }
        else { http.get(url.path(), &file); }
    }
    http.close();
    return true;
}

void httpLoader::httpDone(bool error)
{
    QString fileName = file.fileName();
    if (error) { qDebug() << "ERROR: " << qPrintable(http.errorString()); }
    else { qDebug() << "File downloaded as " << qPrintable(fileName); }
    if (compressor->isOpen()) { compressor->close(); }
    if (file.isOpen()) { file.close(); }
    emit done(fileName);
}
