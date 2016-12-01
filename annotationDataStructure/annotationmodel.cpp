#include <QtCore>
#include <iostream>

#include "annotationmodel.h"
#include "annotationDefinitions.h"
#include "annotationitem.h"
#include "annotationlink.h"
#include "annotationdatareader.h"
#include "annotationdatawriter.h"

annotationModel::annotationModel()
{
    this->ITEMNODES.reserve(100000);
    this->LINKNODES.reserve(150000);
}

annotationModel::~annotationModel()
{
    // delete all the items, links, and mappings
    qDeleteAll(this->ITEMNODES);
    qDeleteAll(this->LINKNODES);
}

//! read data - only interpro or gene ontology
bool annotationModel::readData(const QString& fileName)
{
    bool rval = true;
    annotationDataReader reader(this);
    rval = reader.readFile(fileName);
    return(rval);
}

void annotationModel::addLinkUnique(annotationLink* link)
{
    QString linkName = link->FROMID + "|" + QString::number(link->TYPE) + "|" + link->TOID;
    if (!this->EXISTINGLINKS.contains(linkName)) {
        this->LINKNODES.append(link);
        this->EXISTINGLINKS << linkName;
    }
}

//! read gene to term mappings
bool annotationModel::readMapping(const QString &fileName)
{
    // open and check
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        std::cerr << "Can not open file for reading: "
                  << qPrintable(file.errorString()) << std::endl;
        return false;
    }

    QtIOCompressor compressor(&file);
    compressor.setStreamFormat(QtIOCompressor::GzipFormat); //! TODO - CHECK FOR FORMAT AND HANDLE ERRORS
    if (!compressor.open(QIODevice::ReadOnly)) {
        std::cerr << "Error: QtIOCompressor - Cannot open file " << qPrintable(fileName)
                  << ": " << qPrintable(compressor.errorString())
                  << std::endl;
        return false;
    }

    QTextStream in(&compressor);
    in.setCodec("UTF-8");

    // read linewise
    annotationItem* curItem;
    annotationLink* curLink;
    QString line;
    QStringList fields;
    QString untrimmedGene;
    QStringList trimmedGene;
    QString gene;
    QStringList terms;
    QRegExp termPattern("IPR\\d{6}|GO:\\d{7}|PF\\d{5}"); //interpro, go, and pfam (only PFAMA)
    int i;
    while(!in.atEnd()) {
        line = in.readLine();
        //fields = line.split(QRegExp("\\s+|\\.")); // split on white chars and take the gene (assumed to be in first column) - also on dot to get rid ov gene variants
        //gene = fields.takeFirst();
        fields = line.split("\t");
        untrimmedGene = fields.takeFirst();
        trimmedGene = untrimmedGene.split(QRegExp("\\s+|\\."));// split on white chars and take the gene (assumed to be in first column) - also on dot to get rid ov gene variants
        gene = trimmedGene.first();
        i = termPattern.indexIn(line); //this is NOT USELESS! IT SAYS THAT i IS UNUSED - TRUE, BUT THIS LINE IS NEEDED
        terms = termPattern.capturedTexts();
        if (!this->ITEMNODES.contains(gene)) {
            curItem = new annotationItem(GENE);
            curItem->ID = gene;
            this->ITEMNODES.insert(gene, curItem);
        }
        //write also some stuff into the description
        curItem = this->ITEMNODES.value(gene);
        // replace any empty string with "empty"
        for (int k = 0; k < fields.count(); ++k) {
            if (fields.at(k) == "") {
                fields[k] = "empty";
            }
        }
        curItem->addDescription(fields.join("\n\n"));
        //
        foreach (QString term, terms) {
            if (term == "") { continue; }
            // this one shall be bidirectional
            curLink = new annotationLink(MAP, gene, term);
            this->addLinkUnique(curLink);
            curLink = new annotationLink(MAP, term, gene);
            this->addLinkUnique(curLink);
        }
    }

    // close and return
    compressor.close();
    file.close();
    //qDebug() << "loaded file " << fileName;
    return(true);
}

//! link everything
bool annotationModel::linkAll()
{
    bool rval = true;
    float pos = 0;
    float neg = 0;

    annotationItem* curItem;
    foreach(annotationLink* link, this->LINKNODES) {
        if (this->ITEMNODES.contains(link->FROMID) && this->ITEMNODES.contains(link->TOID)) {
            curItem = this->ITEMNODES.value(link->FROMID);
            link->addFrom(curItem);
            curItem = this->ITEMNODES.value(link->TOID);
            link->addTo(curItem);
            ++pos;
        } else {
            //qDebug() << "skipping this ID pair: " << link->FROMID << " -> " << link->TOID;
            ++neg;
        }
    }
    float tot = pos+neg;
    rval = (neg/tot) < 0.1;
    qDebug() << "skipped " << neg << " of " << tot << " pairs. In my case it was mostly InterPro (deleted DB entries)";
    // one could erase the existing links if necessary
    //this->EXISTINGLINKS.clear();

    return(rval);
}

bool annotationModel::writeData(const QString& fileName) const
{
    bool rval = true;
    annotationDataWriter writer(this);
    rval = writer.writeFile(fileName);
    return(rval);
}

bool annotationModel::writeTable(const QString& fileName) const
{
    // how to format this table: gene nodetypeleft nodetyperight terms
    if (fileName.isEmpty()) { return(false); }


    // write out
    QString text = "ERROR";

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QFile::Text)) {
        std::cerr << "Can not open file for writing: "
                  << qPrintable(file.errorString()) << std::endl;
        return(false);
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    annotationItem* curItem;
    QHash<QString, annotationItem*>::const_iterator iter = this->ITEMNODES.constBegin();
    while (iter != this->ITEMNODES.constEnd()) {
        curItem = iter.value();
        if (curItem->TYPE == GENE) {
            text = curItem->getTableLines();
            out << text;
        }
        ++iter;
    }
    out.flush();
    file.close();
    return(true);
}

QStringList annotationModel::getAllTerms() const
{
    QStringList out;
    out.reserve(100000);
    annotationItem* curItem;
    QHash<QString, annotationItem*>::const_iterator iter = this->ITEMNODES.constBegin();
    while (iter != this->ITEMNODES.constEnd()) {
        curItem = iter.value();
        if ((curItem->TYPE != GENE) && (curItem->TYPE != NOITEMTYPE)) { out.append(curItem->ID); }
        ++iter;
    }
    return(out);
}

QStringList annotationModel::getAllGenes() const
{
    QStringList out;
    out.reserve(100000);
    annotationItem* curItem;
    QHash<QString, annotationItem*>::const_iterator iter = this->ITEMNODES.constBegin();
    while (iter != this->ITEMNODES.constEnd()) {
        curItem = iter.value();
        if (curItem->TYPE == GENE) { out.append(curItem->ID); }
        ++iter;
    }
    return(out);
}

//! retrieve genes or terms based on RegExp
QStringList annotationModel::extractTermsOrGenes(const QRegExp& expression, const QStringList& subSet, bool doRecursive) const
{
    QStringList out;
    QStringList toSearch;

    // note that recursive only makes sense if there is a subset given at all
    if (subSet.count() == 0) {
        doRecursive = false;
        toSearch = this->ITEMNODES.keys();
    } else {
        toSearch = subSet;
    }

    // iterate and grep
    annotationItem* curItem;
    int notFound = 0;
    foreach (QString id, toSearch) {
        if (id == "") { continue; }
        if (!this->ITEMNODES.contains(id)) {
            std::cerr << "unknown node id: " << id.toStdString() << std::endl << std::flush;
            ++notFound;
        }
        else {
            curItem = this->ITEMNODES.value(id);
            if (curItem->findRegExp(expression, doRecursive)) { out.append(curItem->ID); }
        }
    }

    // print to std::err if not found
    if (notFound > 0) { std::cerr << "could not find " << notFound << " terms/genes in the database." << std::endl << std::flush; }

    //
    out.sort();
    return(out);
}

//! retrieve gene or term descriptions
QStringList annotationModel::extractDescriptions(const QStringList& terms, const QStringList& addData) const
{
    QStringList out;
    QStringList curDesc;

    annotationItem* curItem;
    foreach (QString term, terms) {
        curDesc.clear();
        curDesc << term;
        if (!this->ITEMNODES.contains(term)) { continue; }
        curItem = this->ITEMNODES.value(term);
        curDesc << curItem->getDESCRIPTION();
        foreach (QString otherDatum, addData) {
            if (curItem->OTHERDATA.contains(otherDatum)) {
                QList<QVariant> values = curItem->OTHERDATA.values(otherDatum);
                foreach (QVariant value, values) {
                    out << value.toString().trimmed();
                }
            }
        }
        out << curDesc.join("\n\n==========\n\n");
    }

    //
    return(out);
}

//! internal translators

QStringList annotationModel::genesToTerms(const QStringList& genes) const
{
    QStringList out;
    out.reserve(20000);
    //
    annotationItem* curItem;
    foreach (QString gene, genes) {
        if (!this->ITEMNODES.contains(gene)) { continue; }
        curItem = this->ITEMNODES.value(gene);
        foreach (annotationLink* link, curItem->LINKSOUT) {
            if (link->TYPE == MAP) {
                out.append(link->TOID); //! hm - at the moment it would be anyway always this type
            }
        }
    }
    //
    return(out);
}

QStringList annotationModel::termsToGenes(const QStringList& terms) const
{
    QStringList out;
    out.reserve(20000);
    //
    annotationItem* curItem;
    foreach (QString term, terms) {
        if (!this->ITEMNODES.contains(term)) { continue; }
        curItem = this->ITEMNODES.value(term);
        foreach (annotationLink* link, curItem->LINKSOUT) {
            if (link->TYPE == MAP) {
                out.append(link->TOID);
            }
        }
    }
    //
    return(out);
}



