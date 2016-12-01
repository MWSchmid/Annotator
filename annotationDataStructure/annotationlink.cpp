#include <QtCore>
#include <iostream>

#include "annotationlink.h"
#include "annotationitem.h"
#include "annotationDefinitions.h"

annotationLink::annotationLink(annotationLinkType type, annotationItem* from, annotationItem* to)
{
    this->TYPE = type;
    this->FROM = from;
    this->TO = to;
    this->FROMID = "";
    this->TOID = "";
}

annotationLink::annotationLink(annotationLinkType type, QString from, QString to)
{
    this->TYPE = type;
    this->FROM = 0;
    this->TO = 0;
    this->FROMID = from;
    this->TOID = to;
}

annotationLink::annotationLink(QString from)
{
    this->TYPE = NOLINKTYPE;
    this->FROM = 0;
    this->TO = 0;
    this->FROMID = from;
    this->TOID = "";
}

annotationLink::annotationLink()
{
    this->TYPE = NOLINKTYPE;
    this->FROM = 0;
    this->TO = 0;
    this->FROMID = "";
    this->TOID = "";
}

annotationLink::~annotationLink()
{
    //! delete them in the collection
    this->FROM = 0; //! ATTENTION - THIS MAY FAIL
    this->TO = 0; //! ATTENTION - THIS MAY FAIL
}

//! add a link
void annotationLink::addFrom(annotationItem* item)
{
    this->FROM = item;
    item->addLinkOut(this);
}

void annotationLink::addTo(annotationItem* item)
{
    this->TO = item;
    item->addLinkIn(this);
}
