#include <QtCore>
#include <iostream>

#include "annotationitem.h"
#include "annotationlink.h"
#include "annotationDefinitions.h"


annotationItem::annotationItem(annotationItemType type, const QString& id, const QString& name, const QString& description)
{
    this->TYPE = type;
    this->ID = id;
    this->NAME = name;
    this->DESCRIPTION << description;
}

annotationItem::annotationItem(annotationItemType type)
{
    this->TYPE = type;
    this->ID = "";
    this->NAME = "";
}

annotationItem::~annotationItem()
{
    //! important - each node must be removed in the collection - links also
    //qDeleteAll(this->CHILDREN); //! ATTENTION - THIS WILL FAIL
}

void annotationItem::addDescription(QString description) {
    this->DESCRIPTION << description.replace("|", "_");
}


//! general data access

QString annotationItem::getID() const { return(this->ID); }
QString annotationItem::getNAME() const { return(this->NAME); }
QStringList annotationItem::getDESCRIPTION() const { return(this->DESCRIPTION); }


//! somehow type-specific data access

// InterPro: all GO terms associated with it
// GO: the ID, alternative ID and to consider IDs
QStringList annotationItem::getAllTermsGO() const
{
    QStringList toSearch;
    QStringList out;
    if (this->TYPE == GENEONTOLOGY || this->TYPE == GENEONTOLOGYBP || this->TYPE == GENEONTOLOGYMF || this->TYPE == GENEONTOLOGYCC) { out << this->ID; }
    toSearch << "altID" << "consider" << "classificationID";
    foreach (QString search, toSearch) {
        if (this->OTHERDATA.contains(search)) {
            QList<QVariant> values = this->OTHERDATA.values(search);
            foreach (QVariant val, values) { out << val.toString(); }
        }
    }
    return(out);
}


//! type-specific data access - GO

QStringList annotationItem::getAlternativeID() const
{
    QStringList out;
    if (this->OTHERDATA.contains("altID")) {
        QList<QVariant> values = this->OTHERDATA.values("altID");
        foreach (QVariant val, values) { out << val.toString(); }
    }
    return(out);
}

QString annotationItem::getNamespace() const
{
    if (this->OTHERDATA.contains("nameSpace")) {
        QList<QVariant> values = this->OTHERDATA.values("nameSpace");
        return(values.at(0).toString()); // note that we assume only one entry
    } else {
        return(""); // this happens in the InterPro cases
    }
}

bool annotationItem::isObsolete() const
{
    if (this->OTHERDATA.contains("isObsolete")) {
        return(true); // i guess this is only existing if true
    } else {
        return(false);
    }
}


//! type-specific data access - InterPro
int annotationItem::getTaxonFrequency(const QString& taxon) const
{
    if (this->OTHERDATA.contains(taxon)) {
        QList<QVariant> values = this->OTHERDATA.values(taxon);
        return(values.at(0).toInt()); // note that we assume only one entry
    } else {
        return(-1); // this happens in the GO cases
    }
}

//! somehow type specific data access - Gene or Terms are mapped to each other (unidirectional)
QString annotationItem::getTableLines() const
{
    // type translator
    QHash<annotationItemType, QString> itemType2String;
    itemType2String.insert(NOITEMTYPE,"noType");
    itemType2String.insert(GENE,"gene");
    itemType2String.insert(GENEONTOLOGY,"go");
    itemType2String.insert(GENEONTOLOGYBP,"goBP");
    itemType2String.insert(GENEONTOLOGYMF,"goMF");
    itemType2String.insert(GENEONTOLOGYCC,"goCC");
    itemType2String.insert(INTERPROFAMILY,"iprFamily");
    itemType2String.insert(INTERPRODOMAIN,"iprDomain");
    itemType2String.insert(INTERPROREPEAT,"iprRepeat");
    itemType2String.insert(INTERPROCONSERVEDSITE,"iprConservedSite");
    itemType2String.insert(INTERPROBINDINGSITE,"iprBindingSite");
    itemType2String.insert(INTERPROACTIVESITE,"iprActiveSite");
    itemType2String.insert(INTERPROPTM,"iprPTM");
    itemType2String.insert(PFAM,"pfam");

    QHash<QString, QStringList> tempDatA;
    QHash<QString, QStringList> tempDatB;
    QStringList emptyList;
    QString typeStr;
    QString tempStr;

    foreach (annotationLink* link, this->LINKSOUT) {
        if (link->TYPE != MAP) { continue; } //! could also be uncommented
        typeStr = itemType2String.value(link->TO->TYPE);
        if (!tempDatA.contains(typeStr)) {
            tempDatA.insert(typeStr, emptyList);
            tempDatB.insert(typeStr, emptyList);
        }
        tempDatA[typeStr] << link->TO->ID;
        tempStr = link->TO->NAME;
        tempStr.append(": ");
        if (link->TO->DESCRIPTION.count() > 0) { tempStr.append(link->TO->DESCRIPTION.at(0)); }
        else { tempStr.append("noDescription"); }
        tempStr.remove("\n");
        tempDatB[typeStr] << tempStr;
    }

    QString out;
    QStringList outList;
    outList.reserve(50);
    QStringList tempListA;
    QStringList tempListB;
    QHash<QString, QStringList>::const_iterator iter = tempDatA.constBegin();
    while (iter != tempDatA.constEnd()) {
        typeStr = iter.key();
        tempListA = iter.value();
        tempListB = tempDatB.value(typeStr);
        outList << this->ID << "\t" << itemType2String.value(this->TYPE) << "\t" << typeStr << "\t" << tempListA.join(",") << "\t" << tempListB.join("|") << "\n";
        ++iter;
    }
    out = outList.join("");
    return(out);
}


//! internal modifiers
//void annotationItem::addParent(annotationItem* item) { if (!this->PARENTS.contains(item)) { this->PARENTS.append(item); } }
//void annotationItem::addChild(annotationItem* item) { if (!this->CHILDREN.contains(item)) { this->CHILDREN.append(item); } }
void annotationItem::addLinkOut(annotationLink* link) { if (!this->LINKSOUT.contains(link)) { this->LINKSOUT.append(link); } }
void annotationItem::addLinkIn(annotationLink* link) { if (!this->LINKSIN.contains(link)) { this->LINKSIN.append(link); } }
//void annotationItem::addOther(const QString& key, const QVariant& entry) { this->OTHERDATA.insert(key, entry); }

//! internal queries
bool annotationItem::findRegExp(const QRegExp& expression, const bool& doRecursive) const
{
    if (this->TYPE == GENE) {
        foreach (QString desc, this->DESCRIPTION) {
            if ( desc.contains(expression) ) { return(true); }
        }
        foreach (annotationLink* link, this->LINKSOUT) {
            if (link->TYPE == MAP) {
                if ( link->TO->findRegExp(expression, doRecursive) ) { return(true); }
            }
        }
    } else {
        if (this->NAME.contains(expression)) { return(true); }
        foreach (QString desc, this->DESCRIPTION) {
            if ( desc.contains(expression) ) { return(true); }
        }
        if (doRecursive) {
            foreach (annotationLink* link, this->LINKSOUT) {
                if (link->TYPE == PARENT) {
                    if ( link->TO->findRegExp(expression, doRecursive) ) { return(true); }
                }
            }
            //! THIS SHOULD NOT BE NECESSARY:
            foreach (annotationLink* link, this->LINKSIN) {
                if (link->TYPE == CHILD) {
                    if ( link->FROM->findRegExp(expression, doRecursive) ) {
                        qDebug() << "this is unexpected";
                        return(true);
                    }
                }
            }
        }
    }
    return(false);
}
