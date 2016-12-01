#ifndef ANNOTATIONDATASTRUCTURE_H
#define ANNOTATIONDATASTRUCTURE_H
#include <QtCore>

#include "annotationDefinitions.h"
//#include "annotationmodel.h"


//! we also need a mini-class that contains commands and infos to do them
/*
  possible COMMANDS are:

  STOPTHREAD interrupts the thread

  DOWNLOAD retrieves a file from an ftp server and emits the local copy
  >> HELPER_A: QUrl
  >> HELPER_B: QString (local Path - a directory where it will be written in)
  >> HELPER_C: bool (doCompress - for http only - tells if the file shall be compressed)

  LOADFILE loads a local file into the memory (ie a compressed obo or interpro xml)
  >> HELPER_A: QString
  >> HELPER_B: QString (local Path - the directory there the files were saved)

  SAVEDATA writes a XGMML file of everything (well - it does not write the descriptions due to the size)
  >> HELPER_A: QString

  SAVETABLE writes a txt file of everything (gene nodetype terms)
  >> HELPER_A: QString

  LOADMAPPINGS loads a mapping file. Optimal would be gene <tab> ID,ID,ID. However, it will split anything, take the first entry as gene and grep the rest for GO and INTERPRO terms. mappings via multiple lines are also allowed
  >> HELPER_A: QString
  >> HELPER_B: QString (local Path - the directory there the files were saved)

  LINKALL will link the terms and genes.

  GETTERMS counts all or a subset of terms that contain a given regular expression
  >> HELPER_A: QString (will be used to make QRegExp)
  >> HELPER_B: QStringList (subset of TERMs or GENEs)
  >> HELPER_C: QStringList (NONRECURSIVE or RECURSIVE --- [NODETYPES -not implemented yet])
  NOTE: will return a list of TERMs or GENEs containing the regular expression

  GETDESCRIPTONS gets the descriptions of selected terms or genes
  >> HELPER_A: QStringList (list of TERMs or GENEs)
  >> HELPER_B: QStringList (list with identifiers from the additional data)
  NOTE: will return a list of descriptions

  COMPARESETS compares two lists of genes - NOT IMPLEMENTED YET
  >> HELPER_A: QStringList (test set)
  >> HELPER_B: QStringList (reference set)
  >> HELPER_C: QStringList (NONRECURSIVE or RECURSIVE --- [NODETYPES -not implemented yet])
*/


class workStep {
public:
    QString COMMAND; // MUST HAVE
    QVariant HELPER_A; // this variables can hold a lot of data types. Depending on the command...
    QVariant HELPER_B; // this variable can hold a lot of data types. Depending on the command...
    QVariant HELPER_C; // this variable can hold a lot of data types. Depending on the command...
    workStep() {
        this->COMMAND = "";
        this->HELPER_A = QVariant();
        this->HELPER_B = QVariant();
        this->HELPER_C = QVariant();
    }
    workStep(QString command, QVariant helper_a, QVariant helper_b, QVariant helper_c) {
        this->COMMAND = command;
        this->HELPER_A = helper_a;
        this->HELPER_B = helper_b;
        this->HELPER_C = helper_c;
    }
};


//forward declarations
class annotationItem;
class annotationLink;
class annotationDataReader;
class annotationModel;
class ftpLoader;
class httpLoader;

class annotationDataStructure  : public QThread
{
    Q_OBJECT
private:
    //! variables used for the thread control
    QQueue<workStep> WORK;
    QWaitCondition WORKADDED;
    QMutex MUTEX;
    annotationModel* MODEL;
    QMutex DOWNLOADMUTEX;
    ftpLoader* FTPLOADER;
    httpLoader* HTTPLOADER;

public:
    annotationDataStructure();
    ~annotationDataStructure();

    // add some work to the list
    void addWork(workStep work);

protected:
    void run();

private slots:
    void downloadFileDoneUnlock(QString fileName);

private:
    //! worker functions
    void downloadFile(workStep work);
    void loadFile(workStep work);//the interpro and go xmls
    void loadMappings(workStep work);
    void linkTermsAndGenes();
    void saveData(workStep work);
    void saveTable(workStep work);
    void getTermsOrGenes(workStep work);
    void getDescriptions(workStep work);
    void compareSets(workStep work);

signals:
    void downloadFileDone(QString fileName);
    void loadFileDone(QString fileName);
    void retrievedTermsOrGenes(QString termsOrGenes);
    void retrievedDescriptions(QString descriptions);
    void errorMessage(QString message);
    void successMessage(QString message);
    void everythingProcessed();
    void idleAgain();
};



#endif // ANNOTATIONDATASTRUCTURE_H
