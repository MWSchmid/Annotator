#include <QtCore>
#include <QtXml>
#include <qtiocompressor.h>
#include <iostream>

#include "annotationdatareader.h"
#include "annotationDefinitions.h"
#include "annotationmodel.h"
#include "annotationitem.h"
#include "annotationlink.h"


annotationDataReader::annotationDataReader(annotationModel* model)
{
    this->MODEL = model;

    /*
    // the things important for the nodes' OTHERDATA
    // geneontology
    this->XML2OTHER.insert(QString("alt_id"),"altID");
    this->XML2OTHER.insert(QString("namespace"),"nameSpace");
    this->XML2OTHER.insert(QString("is_obsolete"),"isObsolete");
    this->XML2OTHER.insert(QString("consider"),"consider");
    // interpro
    this->XML2OTHER.insert(QString("classification","classificationID");
    */
    // things important for the links
    // geneontology
    this->XML2LINKTYPE.insert(QString("is_a"), PARENT);
    this->XML2LINKTYPE.insert(QString("part_of"), PARTOF);
    this->XML2LINKTYPE.insert(QString("regulates"), REGULATES);
    this->XML2LINKTYPE.insert(QString("positively_regulates"), POSITIVELYREGULATES);
    this->XML2LINKTYPE.insert(QString("negatively_regulates"), NEGATIVELYREGULATES);
    // interpro NODE type
    this->XML2ITEMTYPE.insert(QString("Domain"), INTERPRODOMAIN);
    this->XML2ITEMTYPE.insert(QString("Repeat"), INTERPROREPEAT);
    this->XML2ITEMTYPE.insert(QString("Family"), INTERPROFAMILY);
    this->XML2ITEMTYPE.insert(QString("Conserved_site"), INTERPROCONSERVEDSITE);
    this->XML2ITEMTYPE.insert(QString("Binding_site"), INTERPROBINDINGSITE);
    this->XML2ITEMTYPE.insert(QString("Active_site"), INTERPROACTIVESITE);
    this->XML2ITEMTYPE.insert(QString("PTM"), INTERPROPTM);
    // interpro RELATIONS
    //this->XML2LINKTYPE.insert(QString("parent_list"), PARENT);
    //this->XML2LINKTYPE.insert(QString("child_list"), CHILD);
    //this->XML2LINKTYPE.insert(QString("contains"), CONTAINS);
    //this->XML2LINKTYPE.insert(QString("found_in"), FOUNDIN);
}

//annotationDataReader::~annotationDataReader(){}

bool annotationDataReader::readFile(const QString &fileName)
{
    // try to open the file and check for errors
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        std::cerr << "Error: QFile - Cannot open file " << qPrintable(fileName)
                  << ": " << qPrintable(file.errorString())
                  << std::endl;
        return(false);
    }

    //! use the QtIOCompressor (was a Qt Solution) to read the zipped files
    QtIOCompressor compressor(&file);
    compressor.setStreamFormat(QtIOCompressor::GzipFormat); //! TODO - CHECK FOR FORMAT AND HANDLE ERRORS
    if (!compressor.open(QIODevice::ReadOnly)) {
        std::cerr << "Error: QtIOCompressor - Cannot open file " << qPrintable(fileName)
                  << ": " << qPrintable(compressor.errorString())
                  << std::endl;
        return(false);
    }
    // use textstream ?
    //QTextStream textStream(&compressor);

    // start to read
    // set the device
    this->READER.setDevice(&compressor);

    // read the first element
    this->READER.readNext();

    // go through the file
    while (!this->READER.atEnd()) {
        if (this->READER.isStartElement()) {
            if (this->READER.name() == "interprodb") {
                this->readInterPro();
            }
            else if (this->READER.name() == "obo") {
                this->readGeneOntology();
            }
            else if (this->READER.name() == "database") {
                this->READER.readNext();
                if ((this->READER.name() == "name") && (this->READER.readElementText() == "PfamFamily")) {
                    while (!this->READER.atEnd()) {
                        this->READER.readNext();
                        if (this->READER.isStartElement() && this->READER.name() == "entries") {
                            this->readPFAM();
                        }
                    }
                }
            }
            else { this->READER.readNext(); }
        }
        else { this->READER.readNext(); }
    }

    // close compressor and file and check for errors
    compressor.close();
    file.close();

    //! check for errors
    if (this->READER.hasError()) {
        std::cerr << "Error: Failed to parse file "
                  << qPrintable(fileName) << ": "
                  << this->READER.error() << " - "
                  << qPrintable(this->READER.errorString()) << std::endl;
        return(false);
    }
    if (file.error() != QFile::NoError) {
        std::cerr << "Error: QFile - Cannot read file " << qPrintable(fileName)
                  << ": " << qPrintable(file.errorString())
                  << std::endl;
        return(false);
    }
    return(true);
}



//! common functions



void annotationDataReader::skipUnknownElement()
{
    this->READER.readNext();
    while (!this->READER.atEnd()) {
        // check if we have the end of an element
        if (this->READER.isEndElement()) {
            this->READER.readNext();
            break;
        }
        // if it is a start element within the unknown element, read over it
        if (this->READER.isStartElement()) {
            this->skipUnknownElement();
        } else {
            this->READER.readNext();
        }
    }
}




//! functions for reading different annotations



//! GENEONTOLOGY
void annotationDataReader::readGeneOntology()
{
    // pointer to a new item
    annotationItem* item = 0;
    // read one element and search all the terms
    this->READER.readNext();
    while ( !this->READER.atEnd() ) {
        if ( this->READER.isStartElement() ) {
            if (this->READER.name() == "term") {
                item = new annotationItem(GENEONTOLOGY);
                this->readGeneOntologyElement(item);
                this->MODEL->ITEMNODES.insert(item->ID, item);
            }
            else { this->skipUnknownElement(); }
        }
        else {
            this->READER.readNext();
        }
    }
}

void annotationDataReader::readGeneOntologyElement(annotationItem* item)
{
    while (this->READER.readNextStartElement()) {
        // directly extractable
        if      (this->READER.name() == "id")           { item->ID = this->READER.readElementText(); }
        else if (this->READER.name() == "name")         { item->NAME = this->READER.readElementText(); }
        else if (this->READER.name() == "alt_id")       { item->OTHERDATA.insert(QString("altID"), this->READER.readElementText()); }
        else if (this->READER.name() == "is_obsolete")  { item->OTHERDATA.insert(QString("isObsolete"), this->READER.readElementText()); }
        else if (this->READER.name() == "consider")     { item->OTHERDATA.insert(QString("consider"), this->READER.readElementText()); }
        else if (this->READER.name() == "namespace")    {
            item->OTHERDATA.insert(QString("nameSpace"), this->READER.readElementText());
            if (item->OTHERDATA.value("nameSpace") == "biological_process")         { item->TYPE = GENEONTOLOGYBP; }
            else if (item->OTHERDATA.value("nameSpace") == "molecular_function")    { item->TYPE = GENEONTOLOGYMF; }
            else if (item->OTHERDATA.value("nameSpace") == "cellular_component")    { item->TYPE = GENEONTOLOGYCC; }
            else                                                                    { item->TYPE = GENEONTOLOGY; }
        }
        else if (this->READER.name() == "is_a") {
            annotationLink* link = new annotationLink(PARENT, item->ID, this->READER.readElementText());
            this->MODEL->addLinkUnique(link);
        }
        // extractable via subelements
        else if (this->READER.name() == "def") {
            while (!this->READER.atEnd()) {
                this->READER.readNext();
                if ((this->READER.isStartElement()) && (this->READER.name() == "defstr")) { item->addDescription(this->READER.readElementText()); }
                if ((this->READER.isEndElement()) && (this->READER.name() == "def")) { break; }
            }
        }
        else if (this->READER.name() == "relationship") {
            annotationLink* link = new annotationLink(item->ID);
            while (!this->READER.atEnd()) {
                this->READER.readNext();
                if ((this->READER.isStartElement()) && (this->READER.name() == "type")) { link->TYPE = this->XML2LINKTYPE.value(this->READER.readElementText()); }
                if ((this->READER.isStartElement()) && (this->READER.name() == "to")) { link->TOID = this->READER.readElementText(); }
                if ((this->READER.isEndElement()) && (this->READER.name() == "relationship")) { break; }
            }
            if (link->TYPE != NOLINKTYPE) { this->MODEL->addLinkUnique(link); }
        }
        else {
            this->READER.skipCurrentElement();
        }
    }
    //qDebug() << item->ID;
}
/*
<entry entry_type="Pfam-A" accession="PF00002" id="7tm_2">
  <description>
<![CDATA[
7 transmembrane receptor (Secretin family)
]]>
  </description>
</entry>
*/
//! INTERPRO
void annotationDataReader::readInterPro()
{
    // pointer to a new item
    annotationItem* item = 0;
    //qDebug() << "start reading interpro elements";

    // read one element and search all the terms
    this->READER.readNext();
    while ( !this->READER.atEnd() ) {
        if ( this->READER.isStartElement() ) {
            if (this->READER.name() == "interpro") {
                if (this->XML2ITEMTYPE.contains(this->READER.attributes().value("type").toString())) {
                    item = new annotationItem(this->XML2ITEMTYPE.value(this->READER.attributes().value("type").toString()));
                    this->readInterProElement(item);
                    this->MODEL->ITEMNODES.insert(item->ID, item);
                } else {
                    qDebug() << "unknown interpro type " << this->READER.attributes().value("type").toString() << this->READER.attributes().value("id").toString();
                }
            }
            else { this->skipUnknownElement(); }
        }
        else {
            this->READER.readNext();
        }
    }
}

void annotationDataReader::readInterProElement(annotationItem* item)
{
    // read the entries for the item
    item->ID = this->READER.attributes().value("id").toString();

    // read the rest
    while (this->READER.readNextStartElement()) {
        // directly extractable
        if (this->READER.name() == "name") { item->NAME = this->READER.readElementText(); }
        // extractable via subelements
        //! NOTE that there would be also a child list - but I guess it is unnecessary
        else if (this->READER.name() == "abstract") {
            //qDebug() << "trying to read description";
            while (!this->READER.atEnd()) {
                this->READER.readNext();
                if ((this->READER.isStartElement()) && (this->READER.name() == "p")) { item->addDescription(this->READER.readElementText(QXmlStreamReader::SkipChildElements)); }
                if ((this->READER.isEndElement()) && (this->READER.name() == "abstract")) { break; }
            }
            //qDebug() << "done with description";
        }
        else if (this->READER.name() == "parent_list") {
            //qDebug() << "trying to read parent list";
            while (!this->READER.atEnd()) {
                this->READER.readNext();
                if ((this->READER.isStartElement()) && (this->READER.name() == "rel_ref")) {
                    annotationLink* link = new annotationLink(PARENT, item->ID, this->READER.attributes().value("ipr_ref").toString());
                    this->MODEL->addLinkUnique(link);
                }
                if ((this->READER.isEndElement()) && (this->READER.name() == "parent_list")) { break; }
            }
            //qDebug() << "done with parent";
        }
        else if (this->READER.name() == "child_list") {
            //qDebug() << "trying to read child list";
            while (!this->READER.atEnd()) {
                this->READER.readNext();
                if ((this->READER.isStartElement()) && (this->READER.name() == "rel_ref")) {
                    annotationLink* link = new annotationLink(CHILD, item->ID, this->READER.attributes().value("ipr_ref").toString());
                    this->MODEL->addLinkUnique(link);
                }
                if ((this->READER.isEndElement()) && (this->READER.name() == "child_list")) { break; }
            }
            //qDebug() << "done with parent";
        }
        else if (this->READER.name() == "contains") {
            //qDebug() << "trying to read contains list";
            while (!this->READER.atEnd()) {
                this->READER.readNext();
                if ((this->READER.isStartElement()) && (this->READER.name() == "rel_ref")) {
                    annotationLink* link = new annotationLink(CONTAINS, item->ID, this->READER.attributes().value("ipr_ref").toString());
                    this->MODEL->addLinkUnique(link);
                }
                if ((this->READER.isEndElement()) && (this->READER.name() == "contains")) { break; }
            }
            //qDebug() << "done with contains list";
        }
        else if (this->READER.name() == "found_in") {
            //qDebug() << "trying to read found_in list";
            while (!this->READER.atEnd()) {
                this->READER.readNext();
                if ((this->READER.isStartElement()) && (this->READER.name() == "rel_ref")) {
                    annotationLink* link = new annotationLink(FOUNDIN, item->ID, this->READER.attributes().value("ipr_ref").toString());
                    this->MODEL->addLinkUnique(link);
                }
                if ((this->READER.isEndElement()) && (this->READER.name() == "found_in")) { break; }
            }
            //qDebug() << "done with found_in list";
        }
        else if (this->READER.name() == "taxonomy_distribution") {
            //qDebug() << "trying to read tax dist list";
            while (!this->READER.atEnd()) {
                this->READER.readNext();
                if ((this->READER.isStartElement()) && (this->READER.name() == "taxon_data")) {
                    item->OTHERDATA.insert(this->READER.attributes().value("name").toString(), this->READER.attributes().value("protein_count").toString());
                }
                if ((this->READER.isEndElement()) && (this->READER.name() == "taxonomy_distribution")) { break; }
            }
            //qDebug() << "done with tax dist list";
        }
        else if (this->READER.name() == "class_list") {
            //qDebug() << "trying to read class list";
            while (!this->READER.atEnd()) {
                this->READER.readNext();
                if ((this->READER.isStartElement()) && (this->READER.name() == "classification")) { item->OTHERDATA.insert(QString("classificationID"),this->READER.attributes().value("id").toString()); }
                if ((this->READER.isEndElement()) && (this->READER.name() == "class_list")) { break; }
            }
            //qDebug() << "done with class list";
        }
        else { this->READER.skipCurrentElement(); }
    }
    //qDebug() << item->ID;
}


//! PFAM - note that I did not implement any additional data here - PFAM could by the way also be directly linked to GO and InterPro
void annotationDataReader::readPFAM()
{
    // pointer to a new item
    annotationItem* item = 0;
    // read one element and search all the terms
    this->READER.readNext();
    while ( !this->READER.atEnd() ) {
        if ( this->READER.isStartElement() ) {
            if (this->READER.name() == "entry") {
                item = new annotationItem(PFAM);
                this->readPFAMElement(item);
                this->MODEL->ITEMNODES.insert(item->ID, item);
            }
            else { this->skipUnknownElement(); }
        }
        else {
            this->READER.readNext();
        }
    }
}

void annotationDataReader::readPFAMElement(annotationItem* item)
{
    // attribute fields from the entry stuff
    item->NAME = this->READER.attributes().value("id").toString();
    item->ID = this->READER.attributes().value("acc").toString();

    while (this->READER.readNextStartElement()) {
        // directly extractable
        if      (this->READER.name() == "description")  { item->addDescription(this->READER.readElementText()); }
        else { this->READER.skipCurrentElement(); }
    }
}
