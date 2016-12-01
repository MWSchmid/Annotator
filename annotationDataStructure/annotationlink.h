#ifndef ANNOTATIONLINK_H
#define ANNOTATIONLINK_H
#include <QtCore>

#include "annotationDefinitions.h"

// forward declaration
class annotationItem;

class annotationLink
{
public:
    annotationLink(annotationLinkType type, annotationItem* from, annotationItem* to);
    annotationLink(annotationLinkType type, QString from, QString to);
    annotationLink(QString from);
    annotationLink();
    ~annotationLink();

private:
    //! member variables
    annotationLinkType TYPE;
    annotationItem* FROM;
    annotationItem* TO;
    QString FROMID;
    QString TOID;

    //! add links
    void addFrom(annotationItem* item);
    void addTo(annotationItem* item);

    //! friend classes
    friend class annotationDataReader;
    friend class annotationDataWriter;
    friend class annotationModel;
    friend class annotationItem;
};

#endif // ANNOTATIONLINK_H
