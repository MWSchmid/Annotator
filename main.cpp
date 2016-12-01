#include <QtWidgets/QApplication>
#include "annotatorview.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    annotatorView w;
    QString curPath;
    if (argc == 2) {
        curPath = argv[1];
    } else {
        QString path = QString(argv[0]);
        QString end = path.split("/").last();
        path.remove(path.length() - end.length(), end.length());
        QDir dir(path);
        curPath = dir.dirName();
        std::cerr << "no local path given - took the default: " << curPath.toStdString() << std::endl << std::flush;
    }
    w.setLocalPathToFiles(curPath);
    w.show();

    return a.exec();
}
