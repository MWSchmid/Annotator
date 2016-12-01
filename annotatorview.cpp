#include <QtGui>
#include <iostream>
#include <QtXml>

#include "annotatorview.h"
#include "ui_annotatorview.h"

#include "annotationDataStructure/annotationdatastructure.h"

annotatorView::annotatorView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::annotatorView)
{
    this->ui->setupUi(this);

    this->ui->linkPushButton->setEnabled(false);
    this->ui->exportCytoscapePushButton->setEnabled(false);
    this->ui->exportTablePushButton->setEnabled(false);
    this->loadedFiles = 0;
    this->requiredFiles = 6; // the number of files in the downloadAll and loadAll functions
    this->localPathToFiles = QDir::currentPath();

    // find terms or genes using regular expressions
    connect(this->ui->regExpPushButton, SIGNAL(clicked()), this, SLOT(addExpression()));
    connect(this->ui->regExpLineEdit, SIGNAL(returnPressed()), this, SLOT(addExpression()));
    connect(this->ui->searchListWidget, SIGNAL(currentTextChanged(QString)), this, SLOT(searchExpression(QString)));
    // select a specific term or gene from the list to display its description
    connect(this->ui->termPushButton, SIGNAL(clicked()), this, SLOT(selectTerm()));
    connect(this->ui->termLineEdit, SIGNAL(returnPressed()), this, SLOT(selectTerm()));

    // display descriptions for selected items
    connect(this->ui->genesAndTermsListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(getDescriptions()));

    // update the view if results or descriptions are retrieved
    connect(&this->worker, SIGNAL(retrievedTermsOrGenes(QString)), this, SLOT(updateResults(QString)));
    connect(&this->worker, SIGNAL(retrievedDescriptions(QString)), this, SLOT(updateDescriptions(QString)));

    // enable loading of a file if it was downloaded
    connect(&this->worker, SIGNAL(downloadFileDone(QString)), this, SLOT(addAvailableFile(QString)));

    // messages
    connect(&this->worker, SIGNAL(errorMessage(QString)), this, SLOT(printError(QString)));
    connect(&this->worker, SIGNAL(successMessage(QString)), this, SLOT(printSuccess(QString)));

    //! SIMPLE VERSION AND TESTING CONNECTIONS
    connect(this->ui->downloadPushButton, SIGNAL(clicked()), this, SLOT(downloadAll()));
    connect(this->ui->loadPushButton, SIGNAL(clicked()), this, SLOT(loadAll()));
    connect(this->ui->linkPushButton, SIGNAL(clicked()), this, SLOT(linkAll()));
    connect(this->ui->subSetPushButton, SIGNAL(clicked()), this, SLOT(specifySubset()));
    connect(this->ui->loadExpressionsPushButton, SIGNAL(clicked()), this, SLOT(loadExpressions()));
    connect(this->ui->saveExprPushButton, SIGNAL(clicked()), this, SLOT(saveExpressions()));
    connect(this->ui->saveResultsPushButton, SIGNAL(clicked()), this, SLOT(saveResultList()));
    connect(this->ui->exportCytoscapePushButton, SIGNAL(clicked()), this, SLOT(saveCytoscape()));
    connect(this->ui->exportTablePushButton, SIGNAL(clicked()), this, SLOT(exportTable()));
}

annotatorView::~annotatorView()
{
    workStep stop("STOPTHREAD", QVariant(), QVariant(), QVariant());
    this->worker.addWork(stop);
    this->worker.wait();
    delete this->ui;
}



//! PUBLIC SLOTS (mainly load and save)

void annotatorView::specifySubset()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("select a file"),
                                                    ".",
                                                    tr("simple list (*.txt)"));
    this->subSet.clear();
    QString text = "ERROR";
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly)) {
            std::cerr << "Can not open file for reading: "
                      << qPrintable(file.errorString()) << std::endl;
        } else {
            QTextStream in(&file);
            in.setCodec("UTF-8");
            text = in.readAll();
            this->subSet = text.split("\n");
            file.close();
        }
    }
}

void annotatorView::loadExpressions()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("select a file"),
                                                    ".",
                                                    tr("simple list (*.txt)"));
    QString text = "ERROR";
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly)) {
            std::cerr << "Can not open file for reading: "
                      << qPrintable(file.errorString()) << std::endl;
        } else {
            QTextStream in(&file);
            in.setCodec("UTF-8");
            text = in.readAll();
            this->expressions = text.split("\n");
            file.close();
        }
    }
    this->ui->searchListWidget->clear();
    this->ui->searchListWidget->addItems(this->expressions);
}

void annotatorView::saveExpressions()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("select a file"),
                                                    ".",
                                                    tr("simple list (*.txt)"));
    QString text = "ERROR";
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly | QFile::Text)) {
            std::cerr << "Can not open file for writing: "
                      << qPrintable(file.errorString()) << std::endl;
        } else {
            QTextStream out(&file);
            out.setCodec("UTF-8");
            text = this->expressions.join("\n");
            out << text;
            out.flush();
            file.close();
        }
    }
}

void annotatorView::saveResultList()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("select a file"),
                                                    ".",
                                                    tr("simple list (*.txt)"));
    QString text = "ERROR";
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly | QFile::Text)) {
            std::cerr << "Can not open file for writing: "
                      << qPrintable(file.errorString()) << std::endl;
        } else {
            QTextStream out(&file);
            out.setCodec("UTF-8");
            for (int i = 0; i < this->ui->genesAndTermsListWidget->count(); ++i) {
                text = this->ui->genesAndTermsListWidget->item(i)->data(Qt::DisplayRole).toString();
                out << text << "\n";
            }
            out.flush();
            file.close();
        }
    }
}

void annotatorView::saveCytoscape()
{
    // use the XGMML file format:
    // http://www.cs.rpi.edu/research/groups/pb/punin/public_html/XGMML/DOC/xgmml_schema.html
    // http://wiki.cytoscape.org/XGMML
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("select a file"),
                                                    ".",
                                                    tr("XGMML (*.xgmml)"));
    QString command = "SAVEDATA";
    workStep work(command, fileName, QVariant(), QVariant());
    this->worker.addWork(work);
}

void annotatorView::setLocalPathToFiles(QString localPathString)
{
    this->localPathToFiles = localPathString;
}

void annotatorView::downloadFile(QString urlName)
{
    QString command = "DOWNLOAD";
    QUrl url = QUrl(urlName);
    QString localPathString = this->localPathToFiles;
    bool doCompress = true;
    if (urlName.endsWith(".gz", Qt::CaseInsensitive)) {doCompress = false;}
    workStep work(command, url, localPathString, doCompress);
    this->worker.addWork(work);
}

void annotatorView::loadFile(QString fileName)
{
    QString command = "LOADFILE";
    QString localPathString = this->localPathToFiles;
    workStep work(command, fileName, localPathString, QVariant());
    this->worker.addWork(work);
}

void annotatorView::loadMappings(QString fileName)
{
    QString command = "LOADMAPPINGS";
    QString localPathString = this->localPathToFiles;
    workStep work(command, fileName, localPathString, QVariant());
    this->worker.addWork(work);
}

//! PRIVATE SLOTS

void annotatorView::printError(QString message)
{
    //this->ui->messageLabel->setText(message);
    qDebug() << message;
}

void annotatorView::printSuccess(QString message)
{
    if ((message == "DONE: model->readData()") || (message == "DONE: model->readMapping()")) {
        ++this->loadedFiles;
        if (this->loadedFiles == this->requiredFiles) {
            this->ui->linkPushButton->setEnabled(true);
            this->ui->loadPushButton->setEnabled(false);
            this->ui->downloadPushButton->setEnabled(false);
        }
    } else if (message == "DONE: model->linkAll()") {
        this->ui->linkPushButton->setEnabled(false);
        this->ui->exportCytoscapePushButton->setEnabled(true);
        this->ui->exportTablePushButton->setEnabled(true);
    }
    //this->ui->messageLabel->setText(message);
    qDebug() << message;
}

void annotatorView::selectTerm()
{
    QString termToSelect = this->ui->termLineEdit->text();
    QList<QListWidgetItem*> foundItems = this->ui->genesAndTermsListWidget->findItems(termToSelect, Qt::MatchFixedString);

    this->ui->genesAndTermsListWidget->clearSelection();

    if (foundItems.count() == 1) { this->ui->genesAndTermsListWidget->setCurrentItem(foundItems.first()); }
    else if (foundItems.count() > 1) { this->ui->termLineEdit->setText(QObject::tr("WARNING: found more than one entry")); }
    else { this->ui->termLineEdit->setText(QObject::tr("could not find: %1").arg(termToSelect)); }
}

void annotatorView::addAvailableFile(QString fileName)
{
    if (!this->availableFiles.contains(fileName)) {
        this->availableFiles << fileName;
        if (this->availableFiles.count() == this->requiredFiles) {
            this->ui->loadPushButton->setEnabled(true);
        }
    }
}

void annotatorView::addExpression()
{
   QString expression = this->ui->regExpLineEdit->text();
   if (!this->expressions.contains(expression)) {
       this->expressions << expression;
       this->ui->searchListWidget->addItem(expression);
   }
   QList<QListWidgetItem*> foundItems = this->ui->searchListWidget->findItems(expression, Qt::MatchFixedString);
   this->ui->searchListWidget->clearSelection();
   this->ui->searchListWidget->setCurrentItem(foundItems.first());
   this->ui->regExpLineEdit->setText("");
}

void annotatorView::searchExpression(QString expression)
{
   QString command = "GETTERMS";
   QStringList subSet = this->subSet;
   QStringList options;
   options << "RECURSIVE";
   workStep work(command, expression, subSet, options);
   this->worker.addWork(work);
}

void annotatorView::getDescriptions()
{
    QString command = "GETDESCRIPTIONS";
    QStringList terms;
    QList<QListWidgetItem*> items = this->ui->genesAndTermsListWidget->selectedItems();
    foreach(QListWidgetItem* item, items) {
        terms << item->data(Qt::DisplayRole).toString();
    }
    QStringList addData;
    workStep work(command, terms, addData, QVariant());
    this->worker.addWork(work);
}

void annotatorView::updateResults(QString results)
{
    QStringList entries = results.split("\n");
    this->ui->genesAndTermsListWidget->clear();
    this->ui->genesAndTermsListWidget->addItems(entries);
    if (this->ui->genesAndTermsListWidget->count() > 0) {
        this->ui->genesAndTermsListWidget->setCurrentRow(0);
    }
}


void annotatorView::updateDescriptions(QString descriptions)
{
    this->ui->descriptionPlainTextEdit->setPlainText(descriptions);
}

//! SIMPLE VERSION AND TESTING
void annotatorView::downloadAll()
{
    this->ui->loadPushButton->setEnabled(false);
    // download all
    this->downloadFile(QString("ftp://ftp.geneontology.org/pub/go/godatabase/archive/go_daily-termdb.obo-xml.gz"));
    this->downloadFile(QString("ftp://ftp.ebi.ac.uk/pub/databases/interpro/interpro.xml.gz"));
    this->downloadFile(QString("ftp://ftp.ebi.ac.uk/pub/databases/Pfam/sitesearch/PfamFamily.xml.gz"));
    this->downloadFile(QString("ftp://ftp.arabidopsis.org/home/tair/Proteins/Domains/TAIR10_all.domains"));
    this->downloadFile(QString("ftp://ftp.arabidopsis.org/home/tair/Proteins/TAIR10_functional_descriptions"));
    this->downloadFile(QString("ftp://ftp.arabidopsis.org/home/tair/Ontologies/Gene_Ontology/ATH_GO_GOSLIM.txt"));
    //this->downloadFile(QString("ftp://ftp.plantbiology.msu.edu/pub/data/Eukaryotic_Projects/o_sativa/annotation_dbs/pseudomolecules/version_7.0/all.dir/all.pfam"));
    //this->downloadFile(QString("ftp://ftp.plantbiology.msu.edu/pub/data/Eukaryotic_Projects/o_sativa/annotation_dbs/pseudomolecules/version_7.0/all.dir/all.interpro"));
    //this->downloadFile(QString("ftp://ftp.plantbiology.msu.edu/pub/data/Eukaryotic_Projects/o_sativa/annotation_dbs/pseudomolecules/version_7.0/all.dir/all.GOSlim_assignment"));
}

void annotatorView::loadAll()
{
    // load all added files
    this->loadFile(QString("go_daily-termdb.obo-xml.gz"));
    this->loadFile(QString("interpro.xml.gz"));
    this->loadFile(QString("PfamFamily.xml.gz"));
    this->loadMappings(QString("TAIR10_all.domains.gz"));
    this->loadMappings(QString("TAIR10_functional_descriptions.gz"));
    this->loadMappings(QString("ATH_GO_GOSLIM.txt.gz"));
    //this->loadMappings(QString("all.pfam.gz"));
    //this->loadMappings(QString("all.interpro.gz"));
    //this->loadMappings(QString("all.GOSlim_assignment.gz"));
}

void annotatorView::linkAll()
{
    // link
    QString command = "LINKALL";
    workStep work(command, QVariant(), QVariant(), QVariant());
    this->worker.addWork(work);
}

void annotatorView::exportTable()
{
    //select a file
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("select a file"),
                                                    ".",
                                                    tr("simple table (*.txt)"));
    QString command = "SAVETABLE";
    workStep work(command, fileName, QVariant(), QVariant());
    this->worker.addWork(work);
}

// some repos
// GO, InterPro and PFAM
//ftp://ftp.geneontology.org/pub/go/godatabase/archive/go_daily-termdb.obo-xml.gz
//ftp://ftp.ebi.ac.uk/pub/databases/interpro/interpro.xml.gz
//ftp://ftp.sanger.ac.uk/pub/databases/Pfam/sitesearch/
//the post stuff here does not work: http://pfam.sanger.ac.uk/families?output=xml
// mapping
//ftp://ftp.arabidopsis.org/home/tair/Proteins/Domains/TAIR10_all.domains
//ftp://ftp.arabidopsis.org/home/tair/Proteins/TAIR10_functional_descriptions
//ftp://ftp.arabidopsis.org/home/tair/Ontologies/Gene_Ontology/ATH_GO_GOSLIM.txt.gz


