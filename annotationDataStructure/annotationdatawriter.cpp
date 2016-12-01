#include "annotationdatawriter.h"
#include <QtCore>
#include <QtXml>
#include <qtiocompressor.h>
#include <iostream>

#include "annotationdatawriter.h"
#include "annotationDefinitions.h"
#include "annotationmodel.h"
#include "annotationitem.h"
#include "annotationlink.h"

// http://wiki.cytoscape.org/XGMML
annotationDataWriter::annotationDataWriter(const annotationModel* model)
{
    this->MODEL = model;
    //2d to 1d:
    //index = x + (y * width)
    //1d to 2d:
    //x=index % width
    //y=index/width
    //
    float numNodes = static_cast<float>(this->MODEL->ITEMNODES.count());
    this->GRIDWIDTH = sqrt(numNodes);
    this->GRIDMIDDLE = this->GRIDWIDTH*50;

    // name the item types
    this->ITEMTYPE2STRING.insert(NOITEMTYPE,"noItemType");
    this->ITEMTYPE2STRING.insert(GENE,"gene");
    this->ITEMTYPE2STRING.insert(GENEONTOLOGY,"go");
    this->ITEMTYPE2STRING.insert(GENEONTOLOGYBP,"goBP");
    this->ITEMTYPE2STRING.insert(GENEONTOLOGYMF,"goMF");
    this->ITEMTYPE2STRING.insert(GENEONTOLOGYCC,"goCC");
    this->ITEMTYPE2STRING.insert(INTERPROFAMILY,"iprFamily");
    this->ITEMTYPE2STRING.insert(INTERPRODOMAIN,"iprDomain");
    this->ITEMTYPE2STRING.insert(INTERPROREPEAT,"iprRepeat");
    this->ITEMTYPE2STRING.insert(INTERPROCONSERVEDSITE,"iprConservedSite");
    this->ITEMTYPE2STRING.insert(INTERPROBINDINGSITE,"iprBindingSite");
    this->ITEMTYPE2STRING.insert(INTERPROACTIVESITE,"iprActiveSite");
    this->ITEMTYPE2STRING.insert(INTERPROPTM,"iprPTM");
    this->ITEMTYPE2STRING.insert(PFAM,"pfam");
    // name the link types
    this->LINKTYPE2STRING.insert(NOLINKTYPE,"noLinkType");
    this->LINKTYPE2STRING.insert(MAP,"map");
    this->LINKTYPE2STRING.insert(PARENT,"parent");
    this->LINKTYPE2STRING.insert(CHILD,"child");
    this->LINKTYPE2STRING.insert(PARTOF,"partOf");
    this->LINKTYPE2STRING.insert(REGULATES,"regulates");
    this->LINKTYPE2STRING.insert(POSITIVELYREGULATES,"positivelyRegulates");
    this->LINKTYPE2STRING.insert(NEGATIVELYREGULATES,"negativelyRegulates");
    this->LINKTYPE2STRING.insert(CONTAINS,"contains");
    this->LINKTYPE2STRING.insert(FOUNDIN,"foundIn");
}

bool annotationDataWriter::writeFile(const QString &fileName)
{
    if (fileName.isEmpty()) { return(false); }

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QFile::Text)) {
        std::cerr << "Can not open file for writing: "
                  << qPrintable(file.errorString()) << std::endl;
        return(false);
    }

    this->WRITER.setDevice(&file);
    this->WRITER.setAutoFormatting(true);

    // write the header and start the graph element
    this->WRITER.writeStartDocument();
    this->WRITER.writeStartElement("graph");
    this->WRITER.writeAttribute("label", "p502annotatorDefault");
    this->WRITER.writeAttribute("xmlns:dc","http://purl.org/dc/elements/1.1/");
    this->WRITER.writeAttribute("xmlns:xlink","http://www.w3.org/1999/xlink");
    this->WRITER.writeAttribute("xmlns:rdf","http://www.w3.org/1999/02/22-rdf-syntax-ns#");
    this->WRITER.writeAttribute("xmlns:cy","http://www.cytoscape.org");
    this->WRITER.writeAttribute("xmlns","http://www.cs.rpi.edu/XGMML");
    this->WRITER.writeAttribute("directed", "1");
    //
    this->WRITER.writeStartElement("att");
    this->WRITER.writeAttribute("name", "documentVersion");
    this->WRITER.writeAttribute("value", "1.1");
    this->WRITER.writeEndElement();
    //
    this->WRITER.writeStartElement("att");
    this->WRITER.writeAttribute("name", "networkMetadata");
    this->WRITER.writeStartElement("rdf:RDF");
    this->WRITER.writeStartElement("rdf:Description");
    this->WRITER.writeAttribute("rdf:about","ttp://www.cytoscape.org/");
    this->WRITER.writeTextElement("dc:type","Protein-Protein Interaction");
    this->WRITER.writeTextElement("dc:description","N/A");
    this->WRITER.writeTextElement("dc:identifier","N/A");
    this->WRITER.writeTextElement("dc:date",QDateTime::currentDateTime().toString());
    this->WRITER.writeTextElement("dc:title","p502annotatorDefault");
    this->WRITER.writeTextElement("dc:source","http://www.cytoscape.org/");
    this->WRITER.writeTextElement("dc:format","Cytoscape-XGMML");
    this->WRITER.writeEndElement();
    //
    this->WRITER.writeEndElement();
    this->WRITER.writeEndElement();
    //
    this->WRITER.writeStartElement("att");
    this->WRITER.writeAttribute("type", "string");
    this->WRITER.writeAttribute("name", "backgroundColor");
    this->WRITER.writeAttribute("value", "#ffffff");
    this->WRITER.writeEndElement();
    this->WRITER.writeStartElement("att");
    this->WRITER.writeAttribute("type", "real");
    this->WRITER.writeAttribute("name", "GRAPH_VIEW_ZOOM");
    this->WRITER.writeAttribute("value", "1.0");
    this->WRITER.writeEndElement();
    this->WRITER.writeStartElement("att");
    this->WRITER.writeAttribute("type", "real");
    this->WRITER.writeAttribute("name", "GRAPH_VIEW_CENTER_X");
    this->WRITER.writeAttribute("value", QString::number(this->GRIDMIDDLE));
    this->WRITER.writeEndElement();
    this->WRITER.writeStartElement("att");
    this->WRITER.writeAttribute("type", "real");
    this->WRITER.writeAttribute("name", "GRAPH_VIEW_CENTER_Y");
    this->WRITER.writeAttribute("value", QString::number(this->GRIDMIDDLE));
    this->WRITER.writeEndElement();
    this->WRITER.writeStartElement("att");
    this->WRITER.writeAttribute("type", "boolean");
    this->WRITER.writeAttribute("name", "NODE_SIZE_LOCKED");
    this->WRITER.writeAttribute("value", "true");
    this->WRITER.writeEndElement();
    this->WRITER.writeStartElement("att");
    this->WRITER.writeAttribute("type", "string");
    this->WRITER.writeAttribute("name", "__layoutAlgorithm");
    this->WRITER.writeAttribute("value", "grid");
    this->WRITER.writeAttribute("cy:hidden", "true");
    this->WRITER.writeEndElement();

    // nodes
    QHash<QString,int> strToNum;
    QString curNode;
    annotationItem* curItem;
    int numID = 0;
    QHash<QString, annotationItem*>::const_iterator iter = this->MODEL->ITEMNODES.constBegin();
    while (iter != this->MODEL->ITEMNODES.constEnd()) {
        ++numID;
        curNode = iter.key();
        if (!strToNum.contains(curNode)) { strToNum.insert(curNode, numID); }
        curItem = iter.value();
        this->writeNode(curItem, numID);
        ++iter;
    }

    // edges
    int failedLinks = 0;
    int numIDfrom;
    int numIDto;
    foreach (annotationLink* curLink, this->MODEL->LINKNODES) {
        if (strToNum.contains(curLink->FROMID) && strToNum.contains(curLink->TOID)) {
            numIDfrom = strToNum.value(curLink->FROMID);
            numIDto = strToNum.value(curLink->TOID);
            this->writeLink(curLink, numIDfrom, numIDto);
        } else {
            ++failedLinks;
        }
    }

    // close the graph element and the file
    this->WRITER.writeEndElement(); // graph
    this->WRITER.writeEndDocument();
    file.close();

    qDebug() << failedLinks << " edges were invalid.";
    return(true);
}

void annotationDataWriter::writeNode(annotationItem* item, const int& numID)
{
    this->WRITER.writeStartElement("node");
    this->WRITER.writeAttribute("label", item->ID);
    this->WRITER.writeAttribute("id", QString::number(numID));
    this->WRITER.writeStartElement("att");
    this->WRITER.writeAttribute("type", "string");
    this->WRITER.writeAttribute("name", "nodeType");
    this->WRITER.writeAttribute("value", this->ITEMTYPE2STRING.value(item->TYPE));
    this->WRITER.writeEndElement();
    // <give some formatting>
    this->writeNodeStyle(item, numID);
    // </give some formatting>
    this->WRITER.writeEndElement();
}

void annotationDataWriter::writeLink(annotationLink* link, const int& numIDfrom, const int& numIDto)
{
    //QStringList labelEntries;
    //labelEntries << link->FROMID << this->LINKTYPE2STRING.value(link->TYPE) << link->TOID;
    //QString labelEntry = labelEntries.join("|");
    QString labelEntry = link->FROMID + " (pp) " + link->TOID;
    this->WRITER.writeStartElement("edge");
    this->WRITER.writeAttribute("label",labelEntry);
    this->WRITER.writeAttribute("source", QString::number(numIDfrom));
    this->WRITER.writeAttribute("target", QString::number(numIDto));
    this->WRITER.writeStartElement("att");
    this->WRITER.writeAttribute("type", "string");
    this->WRITER.writeAttribute("name", "interaction");
    this->WRITER.writeAttribute("value", this->LINKTYPE2STRING.value(link->TYPE));
    this->WRITER.writeEndElement();
    // <give some formatting>
    this->writeLinkStyle(link, numIDfrom, numIDto);
    // </give some formatting>
    this->WRITER.writeEndElement();
}

//! NOT REQUIRED BUT WOULD BE NICE - get directly the formatting of the nodes
void annotationDataWriter::writeNodeStyle(annotationItem* item, const int& numID)
{
    QXmlStreamAttributes attributes;
    float xPos = (numID % static_cast<int>(this->GRIDWIDTH))*100.0;
    float yPos = (numID / this->GRIDWIDTH)*100.0;

    switch (item->TYPE) {
    case NOITEMTYPE:
        attributes << QXmlStreamAttribute("type", "V");
        attributes << QXmlStreamAttribute("h", "10.0");
        attributes << QXmlStreamAttribute("w", "10.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "0.2");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case GENE:
        attributes << QXmlStreamAttribute("type", "ELLIPSE");
        attributes << QXmlStreamAttribute("h", "30.0");
        attributes << QXmlStreamAttribute("w", "30.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "1");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case GENEONTOLOGY:
        attributes << QXmlStreamAttribute("type", "RECTANGLE");
        attributes << QXmlStreamAttribute("h", "30.0");
        attributes << QXmlStreamAttribute("w", "30.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "1");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case GENEONTOLOGYBP:
        attributes << QXmlStreamAttribute("type", "RECTANGLE");
        attributes << QXmlStreamAttribute("h", "30.0");
        attributes << QXmlStreamAttribute("w", "30.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "1");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case GENEONTOLOGYMF:
        attributes << QXmlStreamAttribute("type", "RECTANGLE");
        attributes << QXmlStreamAttribute("h", "30.0");
        attributes << QXmlStreamAttribute("w", "30.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "1");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case GENEONTOLOGYCC:
        attributes << QXmlStreamAttribute("type", "RECTANGLE");
        attributes << QXmlStreamAttribute("h", "30.0");
        attributes << QXmlStreamAttribute("w", "30.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "1");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case INTERPROFAMILY:
        attributes << QXmlStreamAttribute("type", "DIAMOND");
        attributes << QXmlStreamAttribute("h", "30.0");
        attributes << QXmlStreamAttribute("w", "30.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "1");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case INTERPRODOMAIN:
        attributes << QXmlStreamAttribute("type", "DIAMOND");
        attributes << QXmlStreamAttribute("h", "25.0");
        attributes << QXmlStreamAttribute("w", "25.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "0.8");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case INTERPROREPEAT:
        attributes << QXmlStreamAttribute("type", "DIAMOND");
        attributes << QXmlStreamAttribute("h", "20.0");
        attributes << QXmlStreamAttribute("w", "20.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "0.6");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case INTERPROCONSERVEDSITE:
        attributes << QXmlStreamAttribute("type", "DIAMOND");
        attributes << QXmlStreamAttribute("h", "20.0");
        attributes << QXmlStreamAttribute("w", "20.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "0.6");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case INTERPROBINDINGSITE:
        attributes << QXmlStreamAttribute("type", "DIAMOND");
        attributes << QXmlStreamAttribute("h", "20.0");
        attributes << QXmlStreamAttribute("w", "20.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "0.6");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case INTERPROACTIVESITE:
        attributes << QXmlStreamAttribute("type", "DIAMOND");
        attributes << QXmlStreamAttribute("h", "20.0");
        attributes << QXmlStreamAttribute("w", "20.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "0.6");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case INTERPROPTM:
        attributes << QXmlStreamAttribute("type", "DIAMOND");
        attributes << QXmlStreamAttribute("h", "20.0");
        attributes << QXmlStreamAttribute("w", "20.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "0.6");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    case PFAM:
        attributes << QXmlStreamAttribute("type", "TRIANGLE");
        attributes << QXmlStreamAttribute("h", "30.0");
        attributes << QXmlStreamAttribute("w", "30.0");
        attributes << QXmlStreamAttribute("x", QString::number(xPos, 'f', 1));
        attributes << QXmlStreamAttribute("y", QString::number(yPos, 'f', 1));
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("width", "0");
        attributes << QXmlStreamAttribute("outline", "#000000");
        attributes << QXmlStreamAttribute("cy:nodeTransparency", "1");
        attributes << QXmlStreamAttribute("cy:nodeLabelFont", "Dialog.bold-0-12");
        attributes << QXmlStreamAttribute("cy:borderLineType", "solid");
        break;
    default:
        qDebug() << "unknown item type";
        break;
    }

    this->WRITER.writeStartElement("graphics");
    this->WRITER.writeAttributes(attributes);
    this->WRITER.writeEndElement();
}


void annotationDataWriter::writeLinkStyle(annotationLink* link, const int& numIDfrom, const int& numIDto)
{
    QXmlStreamAttributes attributes;

    switch (link->TYPE) {
    case NOLINKTYPE:
        attributes << QXmlStreamAttribute("width", "1");
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("cy:sourceArrow", "0");
        attributes << QXmlStreamAttribute("cy:targetArrow", "0");
        attributes << QXmlStreamAttribute("cy:sourceArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:targetArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:edgeLabelFont", "Default-0-10");
        attributes << QXmlStreamAttribute("cy:edgeLineType", "DASHED");
        attributes << QXmlStreamAttribute("cy:curved", "STRAIGHT_LINES");
        break;
    case MAP:
        attributes << QXmlStreamAttribute("width", "2");
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("cy:sourceArrow", "0");
        attributes << QXmlStreamAttribute("cy:targetArrow", "0");
        attributes << QXmlStreamAttribute("cy:sourceArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:targetArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:edgeLabelFont", "Default-0-10");
        attributes << QXmlStreamAttribute("cy:edgeLineType", "SOLID");
        attributes << QXmlStreamAttribute("cy:curved", "STRAIGHT_LINES");
        break;
    case PARENT:
        attributes << QXmlStreamAttribute("width", "2");
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("cy:sourceArrow", "0");
        attributes << QXmlStreamAttribute("cy:targetArrow", "9");
        attributes << QXmlStreamAttribute("cy:sourceArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:targetArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:edgeLabelFont", "Default-0-10");
        attributes << QXmlStreamAttribute("cy:edgeLineType", "SOLID");
        attributes << QXmlStreamAttribute("cy:curved", "STRAIGHT_LINES");
        break;
    case CHILD:
        attributes << QXmlStreamAttribute("width", "2");
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("cy:sourceArrow", "9");
        attributes << QXmlStreamAttribute("cy:targetArrow", "0"); // like this, it looks like a parent edge
        attributes << QXmlStreamAttribute("cy:sourceArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:targetArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:edgeLabelFont", "Default-0-10");
        attributes << QXmlStreamAttribute("cy:edgeLineType", "SOLID");
        attributes << QXmlStreamAttribute("cy:curved", "STRAIGHT_LINES");
        break;
    case PARTOF:
        attributes << QXmlStreamAttribute("width", "2");
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("cy:sourceArrow", "0");
        attributes << QXmlStreamAttribute("cy:targetArrow", "9");
        attributes << QXmlStreamAttribute("cy:sourceArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:targetArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:edgeLabelFont", "Default-0-10");
        attributes << QXmlStreamAttribute("cy:edgeLineType", "SOLID");
        attributes << QXmlStreamAttribute("cy:curved", "STRAIGHT_LINES");
        break;
    case REGULATES:
        attributes << QXmlStreamAttribute("width", "2");
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("cy:sourceArrow", "0");
        attributes << QXmlStreamAttribute("cy:targetArrow", "12");
        attributes << QXmlStreamAttribute("cy:sourceArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:targetArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:edgeLabelFont", "Default-0-10");
        attributes << QXmlStreamAttribute("cy:edgeLineType", "SOLID");
        attributes << QXmlStreamAttribute("cy:curved", "STRAIGHT_LINES");
        break;
    case POSITIVELYREGULATES:
        attributes << QXmlStreamAttribute("width", "2");
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("cy:sourceArrow", "0");
        attributes << QXmlStreamAttribute("cy:targetArrow", "3");
        attributes << QXmlStreamAttribute("cy:sourceArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:targetArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:edgeLabelFont", "Default-0-10");
        attributes << QXmlStreamAttribute("cy:edgeLineType", "SOLID");
        attributes << QXmlStreamAttribute("cy:curved", "STRAIGHT_LINES");
        break;
    case NEGATIVELYREGULATES:
        attributes << QXmlStreamAttribute("width", "2");
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("cy:sourceArrow", "0");
        attributes << QXmlStreamAttribute("cy:targetArrow", "15");
        attributes << QXmlStreamAttribute("cy:sourceArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:targetArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:edgeLabelFont", "Default-0-10");
        attributes << QXmlStreamAttribute("cy:edgeLineType", "SOLID");
        attributes << QXmlStreamAttribute("cy:curved", "STRAIGHT_LINES");
        break;
    case CONTAINS:
        attributes << QXmlStreamAttribute("width", "2");
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("cy:sourceArrow", "9");
        attributes << QXmlStreamAttribute("cy:targetArrow", "0");
        attributes << QXmlStreamAttribute("cy:sourceArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:targetArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:edgeLabelFont", "Default-0-10");
        attributes << QXmlStreamAttribute("cy:edgeLineType", "SOLID");
        attributes << QXmlStreamAttribute("cy:curved", "STRAIGHT_LINES");
        break;
    case FOUNDIN:
        attributes << QXmlStreamAttribute("width", "2");
        attributes << QXmlStreamAttribute("fill", "#000000");
        attributes << QXmlStreamAttribute("cy:sourceArrow", "0");
        attributes << QXmlStreamAttribute("cy:targetArrow", "9");
        attributes << QXmlStreamAttribute("cy:sourceArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:targetArrowColor", "#000000");
        attributes << QXmlStreamAttribute("cy:edgeLabelFont", "Default-0-10");
        attributes << QXmlStreamAttribute("cy:edgeLineType", "SOLID");
        attributes << QXmlStreamAttribute("cy:curved", "STRAIGHT_LINES");
        break;
    default:
        qDebug() << "unknown link type";
        break;
    }

    this->WRITER.writeStartElement("graphics");
    this->WRITER.writeAttributes(attributes);
    this->WRITER.writeEndElement();
}











//! an example of a minimal xgmml that is working
/*
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<graph label="small example"
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
    xmlns:cy="http://www.cytoscape.org"
    xmlns="http://www.cs.rpi.edu/XGMML"
    directed="1">
  <node label="A" id="1">
    <att name="size" type="integer" value="24"/>
    <att name="confirmed" type="boolean" value="true"/>
  </node>
  <node label="B" id="2">
    <att name="size" type="integer" value="16"/>
    <att name="confirmed" type="boolean" value="false"/>
  </node>
  <node label="C" id="3">
    <att name="size" type="integer" value="13"/>
    <att name="confirmed" type="boolean" value="true"/>
  </node>
  <edge label="A-B" source="1" target="2">
    <att name="weight" type="integer" value="7"/>
  </edge>
  <edge label="B-C" source="2" target="3">
    <att name="weight" type="integer" value="8"/>
  </edge>
  <edge label="C-A" source="3" target="1">
    <att name="weight" type="integer" value="4"/>
  </edge>
</graph>
*/

//! node and edge example from a session export
/*
<node label="IPR009082" id="-3744">
  <att type="string" name="Description" value="Signal transduction histidine kinase, homodimeric;Molecular Function: signal transducer activity (GO:0004871), Biological Process: signal transduction (GO:0007165"/>
  <att type="string" name="InterProDomain" value="IPR009082"/>
  <att type="string" name="canonicalName" value="IPR009082"/>
  <att type="list" name="cytoscape.alias.list">
    <att type="string" name="cytoscape.alias.list" value="IPR009082"/>
  </att>
  <att type="string" name="nodeName" value="IPR009082"/>
  <att type="string" name="nodeType" value="domain"/>
  <graphics type="DIAMOND" h="40.0" w="40.0" x="403.4834289550781" y="808.4581298828125" fill="#999999" width="0" outline="#000000" cy:nodeTransparency="1.0" cy:nodeLabelFont="Courier 10 Pitch Bold-0-18" cy:borderLineType="solid"/>
</node>
*/

/*
<edge label="AT3G04580 (gd) IPR003594" source="-35179" target="-2317">
  <att type="string" name="canonicalName" value="AT3G04580 (gd) IPR003594"/>
  <att type="string" name="interaction" value="gd" cy:editable="false"/>
  <graphics width="5" fill="#cccccc" cy:sourceArrow="0" cy:targetArrow="0" cy:sourceArrowColor="#000000" cy:targetArrowColor="#000000" cy:edgeLabelFont="SanSerif-0-10" cy:edgeLineType="SOLID" cy:curved="STRAIGHT_LINES"/>
</edge>
*/
