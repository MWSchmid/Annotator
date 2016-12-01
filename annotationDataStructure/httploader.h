#ifndef HTTPLOADER_H
#define HTTPLOADER_H

#include <QFile>
#include <QHttp>
#include <qtiocompressor.h>

//! most is taken from the httpget example in the qt book chapter 15

class QUrl;

class httpLoader : public QObject
{
    Q_OBJECT

public:
    httpLoader(QObject *parent = 0);
    ~httpLoader();

    bool getFile(const QUrl &url, const QString &localPathString, bool doCompress);
    QString getLocalFile();

signals:
    void done(QString fileName);

private slots:
    void httpDone(bool error);

private:
    QHttp http;
    QFile file;
    QtIOCompressor *compressor;
};

#endif // HTTPLOADER_H
