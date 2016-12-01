#ifndef ANNOTATIONITEM_H
#define ANNOTATIONITEM_H
#include <QtCore>

#include "annotationDefinitions.h"

// forward declaration
class annotationLink;

class annotationItem
{
public:
    annotationItem(annotationItemType type, const QString& id, const QString& name, const QString& description);
    annotationItem(annotationItemType type);
    ~annotationItem();

    void addDescription(QString description);

    //! general data access
    QString getID() const;
    QString getNAME() const;
    QStringList getDESCRIPTION() const;

    //! somehow type-specific data access
    QStringList getAllTermsGO() const;
    // InterPro: all GO terms associated with it
    // GO: the ID, alternative ID and to consider IDs

    //! type-specific data access - GO
    QStringList getAlternativeID() const;
    QString getNamespace() const;
    bool isObsolete() const;

    //! type-specific data access - InterPro
    int getTaxonFrequency(const QString& taxon) const;

    //! somehow type specific data access - Gene or Terms are mapped to each other (unidirectional)
    QString getTableLines() const;

private:
    //! member variables
    annotationItemType TYPE;
    QString ID;
    QString NAME;
    QStringList DESCRIPTION;
    //QList<annotationItem*> PARENTS;
    //QList<annotationItem*> CHILDREN;
    QList<annotationLink*> LINKSOUT;
    QList<annotationLink*> LINKSIN;
    QMultiMap<QString, QVariant> OTHERDATA; // check if multimap is really a good thing

    //! internal modifiers
    //void addParent(annotationItem* item);
    //void addChild(annotationItem* item);
    void addLinkOut(annotationLink* link);
    void addLinkIn(annotationLink* link);
    //void addOther(const QString& key, const QVariant& entry);

    //! internal queries
    bool findRegExp(const QRegExp& expression, const bool& doRecursive) const;

    //! friend classes
    friend class annotationDataReader; // this is only here because we need the addOther function
    friend class annotationDataWriter;
    friend class annotationModel;
    friend class annotationLink;
};

#endif // ANNOTATIONITEM_H
