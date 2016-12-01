#ifndef ANNOTATIONDEFINITIONS_H
#define ANNOTATIONDEFINITIONS_H

//! this file simply defines the types of nodes and interactions implemented in the annotationDataStructure

// the NOxxxxTYPE are empty types that mark unfinished nodes
enum annotationItemType { NOITEMTYPE,
                          GENE,
                          GENEONTOLOGY,
                          GENEONTOLOGYBP,
                          GENEONTOLOGYMF,
                          GENEONTOLOGYCC,
                          INTERPROFAMILY,
                          INTERPRODOMAIN,
                          INTERPROREPEAT,
                          INTERPROCONSERVEDSITE,
                          INTERPROBINDINGSITE,
                          INTERPROACTIVESITE,
                          INTERPROPTM,
                          PFAM };
enum annotationLinkType { NOLINKTYPE,
                          MAP,
                          PARENT,
                          CHILD,
                          PARTOF,
                          REGULATES,
                          POSITIVELYREGULATES,
                          NEGATIVELYREGULATES,
                          CONTAINS,
                          FOUNDIN };

//! the nodes (and their name in the XML if given)
/*
  GENE:
    gene - has only a name and MAP type links
  GENEONTOLOGY:
    GeneOntology: term in any namespace (old - do not use anymore)
  GENEONTOLOGYBP:
    GeneOntology: term in namespace biological_process
  GENEONTOLOGYMF:
    GeneOntology: term in namespace molecular_function
  GENEONTOLOGYCC:
    GeneOntology: term in namespace cellular_component
  INTERPROFAMILY:
    InterPro: Family
  INTERPRODOMAIN:
    InterPro: Domain
  INTERPROREPEAT:
    InterPro: Repeat
  INTERPROCONSERVEDSITE:
    InterPro: Conserved_site
  INTERPROBINDINGSITE:
    InterPro: Binding_site
  INTERPROACTIVESITE:
    InterPro: Active_site
  INTERPROPTM:
    InterPro: PTM
  PFAM:
    PFAM: family
  */

//! the relations (and their name in the XML)
/*
  MAP:
    gene to term and vice versa
  PARENT:
    used in all
    GeneOntology: is_a
    InterPro: parent_list - rel_ref
  CHILD:
    InterPro: child_list - rel_ref
  PARTOF:
    GeneOntology: relationship - part_of
  REGULATES:
    GeneOntology: relationship - regulates
  POSITIVELYREGULATES:
    GeneOntology: relationship - positively_regulates
  NEGATIVELYREGULATES:
    GeneOntology: relationship - negatively_regulates
  CONTAINS:
    InterPro: contains - rel_ref
  FOUNDIN:
    InterPro: found in - rel_ref
  */


#endif // ANNOTATIONDEFINITIONS_H
