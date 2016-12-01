#include <QtCore>
#include <iostream>

#include "annotationdatastructure.h"
#include "annotationDefinitions.h"
#include "annotationmodel.h"
#include "annotationitem.h"
#include "annotationlink.h"
#include "annotationdatareader.h"
#include "ftploader.h"
#include "httploader.h"

annotationDataStructure::annotationDataStructure()
{
    this->MODEL = new annotationModel();
    this->FTPLOADER = new ftpLoader(); //this as parent?
    this->HTTPLOADER = new httpLoader(); //this as parent?
    connect(this->FTPLOADER, SIGNAL(done(QString)), this, SLOT(downloadFileDoneUnlock(QString)));
    connect(this->HTTPLOADER, SIGNAL(done(QString)), this, SLOT(downloadFileDoneUnlock(QString)));
    this->start();
}

annotationDataStructure::~annotationDataStructure()
{
    // empty the queue of projects and add STOPTHREAD - this tells the thread to exit the forever loop
    {
        QMutexLocker locker(&this->MUTEX);
        while (!this->WORK.isEmpty()) {
            this->WORK.dequeue();
        }
        workStep work("STOPTHREAD", QVariant(), QVariant(), QVariant());
        this->WORK.enqueue(work);
        this->WORKADDED.wakeOne();
    }
    // wait before calling the base class destructor
    delete this->FTPLOADER;
    delete this->HTTPLOADER;
    delete this->MODEL;
    this->wait();
}

void annotationDataStructure::addWork(workStep work)
{
    QMutexLocker locker(&this->MUTEX);
    this->WORK.enqueue(work);
    this->WORKADDED.wakeOne();
}

/*
DOWNLOAD retrieves a file from an ftp server and loads it into the memory if requested
LOADFILE loads a local file into the memory (ie a compressed obo or interpro xml)
LOADMAPPINGS loads a mapping file. Optimal would be gene <tab> ID,ID,ID. However, it will split anything, take the first entry as gene and grep the rest for GO and INTERPRO terms
LINKALL will link the terms and genes.
GETTERMS counts all or a subset of terms that contain a given regular expression
COMPARESETS compares two lists of genes
*/

void annotationDataStructure::run()
{
    workStep work;
    forever {
        // check if there is something to process
        {
            QMutexLocker locker(&this->MUTEX);
            if (this->WORK.isEmpty()) {
                this->WORKADDED.wait(&this->MUTEX);
            }
            // take a project from the queue
            work = this->WORK.dequeue();
        }
        //qDebug() << "start working";
        // do what is requested
        {
            if          ( work.COMMAND == "STOPTHREAD" )        { break; }
            else if     ( work.COMMAND == "DOWNLOAD" )          { this->downloadFile(work); }
            else if     ( work.COMMAND == "LOADFILE" )          { this->loadFile(work); }
            else if     ( work.COMMAND == "LOADMAPPINGS" )      { this->loadMappings(work); }
            else if     ( work.COMMAND == "LINKALL" )           { this->linkTermsAndGenes(); }
            else if     ( work.COMMAND == "SAVEDATA" )          { this->saveData(work); }
            else if     ( work.COMMAND == "SAVETABLE" )          { this->saveTable(work); }
            else if     ( work.COMMAND == "GETTERMS" )          { this->getTermsOrGenes(work); }
            else if     ( work.COMMAND == "GETDESCRIPTIONS" )   { this->getDescriptions(work); }
            else if     ( work.COMMAND == "COMPARESET" )        { this->compareSets(work); }
            else { emit this->errorMessage("ERROR: annotationDataStructure::run() unknown command"); }
        }
        //qDebug() << "end working";
        // check if all are processed
        {
            QMutexLocker locker(&this->MUTEX);
            if (this->WORK.isEmpty()) {
                emit this->everythingProcessed();
            }
        }
    }
}

void annotationDataStructure::downloadFileDoneUnlock(QString fileName)
{
    //qDebug() << "emitting";
    emit this->downloadFileDone(fileName);
    //qDebug() << "unlocking";
    this->DOWNLOADMUTEX.unlock();
}

//! worker functions
//! NERVIGE SACHE - getFile BLOCKIERT NICHT - HAT SO MAL GEKLAPPT
void annotationDataStructure::downloadFile(workStep work)
{
    // lock the MUTEX that nothing will be overridden.
    //qDebug() << "locking";
    this->DOWNLOADMUTEX.lock();

    // decode
    QUrl fileUrl = work.HELPER_A.toUrl();
    QString localPathString = work.HELPER_B.toString();
    bool doCompress = work.HELPER_C.toBool();
    //qDebug() << "starting to download " << fileUrl.path();

    // load the file
    if (fileUrl.scheme() == "ftp") {
        if ( !this->FTPLOADER->getFile(fileUrl, localPathString, doCompress) ) {
            emit this->errorMessage("ERROR: ftpLoader.getFile()");
        } else {
            emit this->successMessage("DONE: ftpLoader.getFile()");
        }
    } else if (fileUrl.scheme() == "http") {
        if ( !this->HTTPLOADER->getFile(fileUrl, localPathString, doCompress) ) {
            emit this->errorMessage("ERROR: httpLoader.getFile()");
        } else {
            emit this->successMessage("DONE: httpLoader.getFile()");
        }
    } else {
        emit this->errorMessage("ERROR: annotationDataStructure.downloadFile() - invalid protocol");
    }

    //qDebug() << "exiting function";
}


void annotationDataStructure::loadFile(workStep work)
{
    // decode
    QString fileName = work.HELPER_A.toString();
    QString localPathString = work.HELPER_B.toString();
    fileName.prepend("/");
    fileName.prepend(localPathString);
    fileName = QDir::toNativeSeparators(fileName);
    fileName = QDir::cleanPath(fileName);
    // load
    if (!this->MODEL->readData(fileName)) {
        emit this->errorMessage("ERROR: model->readData()");
    } else {
        emit this->successMessage("DONE: model->readData()");
    }
}

void annotationDataStructure::loadMappings(workStep work)
{
    // decode
    QString fileName = work.HELPER_A.toString();
    QString localPathString = work.HELPER_B.toString();
    fileName.prepend("/");
    fileName.prepend(localPathString);
    fileName = QDir::toNativeSeparators(fileName);
    fileName = QDir::cleanPath(fileName);

    // load
    if (!this->MODEL->readMapping(fileName)) {
        emit this->errorMessage("ERROR: model->readMapping()");
    } else {
        emit this->successMessage("DONE: model->readMapping()");
    }
}


void annotationDataStructure::linkTermsAndGenes()
{
    if (!this->MODEL->linkAll()) {
        emit this->errorMessage("ERROR: model->linkAll()");
    } else {
        emit this->successMessage("DONE: model->linkAll()");
    }
}

void annotationDataStructure::saveData(workStep work)
{
    // decode
    QString fileName = work.HELPER_A.toString();

    // write
    if (!this->MODEL->writeData(fileName)) {
        emit this->errorMessage("ERROR: model->writeData()");
    } else {
        emit this->successMessage("DONE: model->writeData()");
    }
}

void annotationDataStructure::saveTable(workStep work)
{
    // decode
    QString fileName = work.HELPER_A.toString();

    // write
    if (!this->MODEL->writeTable(fileName)) {
        emit this->errorMessage("ERROR: model->writeTable()");
    } else {
        emit this->successMessage("DONE: model->writeTable()");
    }
}

void annotationDataStructure::getTermsOrGenes(workStep work)
{
    // decode
    QRegExp expression(work.HELPER_A.toString(), Qt::CaseInsensitive);
    QStringList subset = work.HELPER_B.toStringList();
    QStringList options = work.HELPER_C.toStringList();
    bool doRecursive = options.at(0) == QString("RECURSIVE");

    // get them
    QStringList results = this->MODEL->extractTermsOrGenes(expression, subset, doRecursive);
    QString out = results.join("\n");

    //
    emit this->successMessage("DONE: getTermsOrGenes");
    emit this->retrievedTermsOrGenes(out);
}

void annotationDataStructure::getDescriptions(workStep work)
{
    // decode
    QStringList terms = work.HELPER_A.toStringList();
    QStringList addData = work.HELPER_B.toStringList();

    // get them
    QStringList results = this->MODEL->extractDescriptions(terms, addData);
    QString out = results.join("\n\n\n**********\n\n\n");

    //
    emit this->successMessage("DONE: getDescriptions");
    emit this->retrievedDescriptions(out);
}


void annotationDataStructure::compareSets(workStep work)
{
    // decode
    QStringList testSet = work.HELPER_A.toStringList();
    QStringList referenceSet = work.HELPER_B.toStringList();
    QStringList options = work.HELPER_C.toStringList();
    bool doRecursive = options.at(0) == "RECURSIVE";
    //QStringList nodeTypes = options.at(1).split(",");

    //
    emit this->errorMessage("ERROR: compareSets is not implemented");
}


