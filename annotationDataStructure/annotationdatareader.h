#ifndef ANNOTATIONDATAREADER_H
#define ANNOTATIONDATAREADER_H
#include <QtCore>
#include <QtXml>
#include <qtiocompressor.h>

#include "annotationDefinitions.h"

// forward declarations
class annotationModel;
class annotationItem;
class annotationLink;

class annotationDataReader
{
public:
    annotationDataReader(annotationModel* model);
    //~annotationDataReader();
    bool readFile(const QString &fileName);

private:
    //! member variables
    annotationModel* MODEL;
    QXmlStreamReader READER;
    QHash<QString, annotationLinkType> XML2LINKTYPE;
    QHash<QString, annotationItemType> XML2ITEMTYPE;
    //QHash<QString, QString> XML2OTHER;

    //! common functions
    void skipUnknownElement();

    //! functions for reading different annotations
    void readGeneOntology();
    void readGeneOntologyElement(annotationItem* item);
    void readInterPro();
    void readInterProElement(annotationItem* item);
    void readPFAM();
    void readPFAMElement(annotationItem* item);
};

#endif // ANNOTATIONDATAREADER_H
