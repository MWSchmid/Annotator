#ifndef ANNOTATORVIEW_H
#define ANNOTATORVIEW_H

//#include <QWidget>
#include <QtWidgets>

#include "annotationDataStructure/annotationdatastructure.h"

namespace Ui {
    class annotatorView;
}

class annotatorView : public QWidget
{
    Q_OBJECT

public:
    explicit annotatorView(QWidget *parent = 0);
    ~annotatorView();

private:
    Ui::annotatorView *ui;
    annotationDataStructure worker;
    QStringList expressions;
    QStringList subSet;
    QString localPathToFiles;
    QStringList availableFiles; //not really in use at the moment
    int loadedFiles; //number of loaded files - its rather a dummy thing
    int requiredFiles; // number of files that must be downloaded/loaded to link the rest (number Files in the downloadAll and loadAll functions)


public slots:
    void specifySubset();
    void saveExpressions();
    void loadExpressions();
    void saveResultList();
    void saveCytoscape();
    void downloadFile(QString urlName);
    void loadFile(QString fileName);
    void setLocalPathToFiles(QString localPathString);
    void loadMappings(QString fileName);
    //! simple testing
    void downloadAll();
    void loadAll();
    void linkAll();
    void exportTable();

private slots:
    void printError(QString message);
    void printSuccess(QString message);
    void addExpression();
    void searchExpression(QString expression);
    void selectTerm();
    void addAvailableFile(QString fileName);
    void updateResults(QString results);
    void getDescriptions();
    void updateDescriptions(QString descriptions);
};

#endif // ANNOTATORVIEW_H
