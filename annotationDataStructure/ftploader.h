#ifndef FTPLOADER_H
#define FTPLOADER_H

#include <QtCore>
#include <QFile>
#include <QFtp>
#include <qtiocompressor.h>

//! most is taken from the ftpget example in the qt book chapter 15

class QUrl;

class ftpLoader : public QObject
{
    Q_OBJECT

public:
    ftpLoader(QObject *parent = 0);
    ~ftpLoader();

    bool getFile(const QUrl &url, const QString &localPathString, bool doCompress);
    QString getLocalFile();

signals:
    void done(QString fileName);

private slots:
    void ftpDone(bool error);

private:
    QFtp ftp;
    QFile file;
    QtIOCompressor *compressor;
};

#endif // FTPLOADER_H
