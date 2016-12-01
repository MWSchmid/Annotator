#ifndef ANNOTATIONDATAWRITER_H
#define ANNOTATIONDATAWRITER_H
#include <QtCore>
#include <QtXml>

#include "annotationDefinitions.h"

// forward declarations
class annotationModel;
class annotationItem;
class annotationLink;

class annotationDataWriter
{
public:
    annotationDataWriter(const annotationModel* model);
    //~annotationDataReader();
    bool writeFile(const QString &fileName);

private:
    //! member variables
    const annotationModel* MODEL;
    QXmlStreamWriter WRITER;
    float GRIDWIDTH;
    float GRIDMIDDLE;
    // translators for the most important things
    QHash<annotationLinkType, QString> LINKTYPE2STRING;
    QHash<annotationItemType, QString> ITEMTYPE2STRING;


    //! functions to write either a node (item) or an edge (link)
    void writeNode(annotationItem* item, const int& numID);
    void writeLink(annotationLink* link, const int& numIDfrom, const int& numIDto);
    //! write directly the style information for every node and link
    void writeNodeStyle(annotationItem* item, const int& numID);
    void writeLinkStyle(annotationLink* link, const int& numIDfrom, const int& numIDto);

};

#endif // ANNOTATIONDATAWRITER_H
