#ifndef ANNOTATIONMODEL_H
#define ANNOTATIONMODEL_H
#include <QtCore>

#include "annotationDefinitions.h"


//forward declarations
class annotationItem;
class annotationLink;
class annotationDataReader;
class annotationDataWriter;

class annotationModel
{
public:
    annotationModel();
    ~annotationModel();

    // read annotation data of a certain type (interpro or geneontology)
    bool readData(const QString& fileName);
    // read gene to term mappings
    bool readMapping(const QString& fileName);
    // link everything
    bool linkAll();
    // write out a XGMML file
    bool writeData(const QString& fileName) const;
    // write out a table
    bool writeTable(const QString& fileName) const;
    // retrieve genes or terms based on RegExp
    QStringList extractTermsOrGenes(const QRegExp& expression, const QStringList& subSet, bool doRecursive) const;
    // get description string
    QStringList extractDescriptions(const QStringList& terms, const QStringList& addData) const;
    // all terms/genes (via ID in ITEMNODES)
    QStringList getAllTerms() const;
    QStringList getAllGenes() const;

private:
    //! member variables
    QHash<QString,annotationItem*> ITEMNODES; // the items itself (terms and descriptions)
    QList<annotationLink*> LINKNODES; // the relationship between the terms
    QSet<QString> EXISTINGLINKS; // to avoid duplicated links

    //! internal translators
    QStringList genesToTerms(const QStringList& genes) const;
    QStringList termsToGenes(const QStringList& terms) const;

    //! add a link uniquely
    void addLinkUnique(annotationLink* link);

    //! friends
    friend class annotationDataReader;
    friend class annotationDataWriter;
};

#endif // ANNOTATIONMODEL_H
