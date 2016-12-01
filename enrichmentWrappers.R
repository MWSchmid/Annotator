f.obj2txt <- function(x, file = paste(deparse(substitute(x)), ".txt", sep = "")) {
  #
  # ARGUMENTS:
  #  x:          R data object to save as ascii
  #  filename:   Name of file to save. Default is name of x with ".txt" extension
  tmp.wid = getOption("width")  # save current width
  options(width = 10000)        # increase output width
  sink(file)                    # redirect output to file
  print(x)                      # print the object
  sink()                        # cancel redirection
  options(width = tmp.wid)      # restore linewidth
  return(invisible(NULL))       # return (nothing) from function
}

###############################################################################
## generic comparison functions
###############################################################################

f.generic.enrichment.test <- function(soi, uni, map, termDesc, rdir, prefix) 
{
  require("xtable")
  # this function tests for enrichment (two-sided) of terms in a set of interest using fishers exact test
  # soi <character vector>: the set of interest
  # uni <character vector>: the set to compare with
  # map <named list containing character vectors>: the annotation. Names must be given. Each entry may contain several terms as a character vector
  # termDesc <named vector>: contains the descriptions of the terms
  # rdir <directory>: a directory where the results are stored
  # prefix <string>: a prefix for the file
  # check for presence of annotation
  soiAnn <- intersect(soi, names(map))
  uniAnn <- intersect(uni, names(map))
  soiNumNotAnn <- length(soi) - length(soiAnn)
  uniNumNotAnn <- length(uni) - length(uniAnn)
  cat(paste("genes of int without annotation terms: ", soiNumNotAnn, "\n", sep = ''))
  cat(paste("genes in ref without annotation terms: ", uniNumNotAnn, "\n", sep = ''))
  # extract all available terms
  soiTermsUnique <- unique(unlist(map[soiAnn]))
  uniTermsUnique <- unique(unlist(map[uniAnn]))
  if (length(soiTermsUnique) == 1) {
    cat("THIS WILL EVENTUALLY THROW AN ERROR - WITH ONE TERM SOMETHING WEIRD IS HAPPENING\n")
    cat("RETURNING NULL\n")
    return(NULL)
  }
  cat(paste("terms in genes of int: ", length(soiTermsUnique), "\n", sep = ''))
  cat(paste("terms in genes in uni: ", length(uniTermsUnique), "\n", sep = ''))
  termsBoth <- intersect(soiTermsUnique, uniTermsUnique)
  termsTotal <- union(soiTermsUnique, uniTermsUnique)
  cat(paste("terms in genes of int and reference: ", length(termsBoth), "\n", sep = ''))
  cat(paste("terms in total: ", length(termsTotal), "\n", sep = ''))
  # get a table where each row holds a vector for a contingency table for fisher.test()
  contingencyTable <- matrix(0, nrow = length(termsTotal), ncol = 4)
  rownames(contingencyTable) <- termsTotal
  colnames(contingencyTable) <- c("soiTermsInGroup","soiTermsOutGroup","uniTermsInGroup","uniTermsOutGroup")
  soiTerms <- unlist(map[soiAnn])
  uniTerms <- unlist(map[uniAnn])
  soiTermsInGroup <- table(soiTerms)
  uniTermsInGroup <- table(uniTerms)
  soiTermsOutGroup <- length(soiTerms) - soiTermsInGroup
  uniTermsOutGroup <- length(uniTerms) - uniTermsInGroup
  contingencyTable[names(soiTermsInGroup), "soiTermsInGroup"] <- soiTermsInGroup
  contingencyTable[names(uniTermsInGroup), "uniTermsInGroup"] <- uniTermsInGroup
  contingencyTable[names(soiTermsOutGroup), "soiTermsOutGroup"] <- soiTermsOutGroup
  contingencyTable[names(uniTermsOutGroup), "uniTermsOutGroup"] <- uniTermsOutGroup
  # only test the ones that were found in set of interest
  contingencyTable <- contingencyTable[soiTermsUnique,]
  # remove the ones with very low counts?
  ## contingencyTable <- contingencyTable[ (contingencyTable[,"soiTermsInGroup"] + contingencyTable[,"uniTermsInGroup"]) > 5, ] CHECK
  pValues <- apply(contingencyTable, 1, function(x) fisher.test(matrix(x, nrow = 2))$p.value)
  pValuesAdj <- p.adjust(pValues, method = "BH")
  expectedTerms <- contingencyTable[,"uniTermsInGroup"]/contingencyTable[,"uniTermsOutGroup"]*contingencyTable[,"soiTermsOutGroup"] ## NOTE that this only makes sense if term was at all in the set of interest
  if (length(termDesc) > 0) {
    out <- cbind(contingencyTable[,"soiTermsInGroup"], expectedTerms, pValues, pValuesAdj, termDesc[rownames(contingencyTable)])
  } else {
    out <- cbind(contingencyTable[,"soiTermsInGroup"], expectedTerms, pValues, pValuesAdj, rep("noDescription", nrow(contingencyTable)))
  }	
  rownames(out) <- rownames(contingencyTable)
  colnames(out) <- c("observed", "expected", "pValue", "FDR", "description")
  out <- out[order(out[,"FDR"]),]
  # write the table in normal format and tex format
  write.table(out, file = file.path(rdir, paste(prefix, "genericTestResults.txt", sep = "_")), sep = "\t", quote = FALSE)
  modOut <- data.frame(out, stringsAsFactors = FALSE)
  modOut$expected <- signif(as.numeric(modOut$expected), 2)
  modOut$pValue <- signif(as.numeric(modOut$pValue), 4)
  modOut$FDR <- signif(as.numeric(modOut$FDR), 4)
  f.obj2txt(xtable(modOut, caption = prefix), file.path(rdir, paste(prefix, "genericTestResults.tex", sep = "_")))
  # return the table
  return(out)
}

###############################################################################
## topGO wrappers
###############################################################################

f.topGO.wrapper <- function(universe, interest, x2go, resultdir, prefix = '', onto = "BP") {
  require("topGO")
  require("xtable")
  # create geneList object (named factors)
  list.com <- factor(as.integer(universe %in% interest))
  names(list.com) <- universe
  # Test for BP terms - therefore:
  GOdata <- new("topGOdata", ontology = onto, allGenes = list.com, annot = annFUN.gene2GO, gene2GO = x2go)
  # Working with the topGOdata object (-> see documentation) run the tests
  result <- runTest(GOdata, algorithm = "weight", statistic = "fisher")
  #resultclassic <- runTest(GOdata, algorithm = "classic", statistic = "fisher")
  # Summarise the results - the tables above contain the same results but are ordered differently. Once by the test statistic using the WEIGHT method (T2) and once by the test statistic using the CLASSIC methog (T2.cla)
  # note: WEIGHT decorrelates the graph, CLASSIC not. In both cases, the test is FISHERs exact. The arguments are explained in topGO-Ref.pdf
  topNodesToPrint <- 50 ## NOTE that this only works if there are more then 50 GOterms
  result.table <- GenTable(GOdata, weight = result, orderBy = "weight", topNodes = topNodesToPrint)
  #result.table <- GenTable(GOdata, weight = result, classic = resultclassic, orderBy = "weight", ranksOf = "classic", topNodes = topNodesToPrint)
  #result.table.cla <- GenTable(GOdata, weight = result, classic = resultclassic, orderBy = "classic", ranksOf = "weight", topNodes = 50)
  # Write the tables as txt files (as I did)
  suf_str <- paste(prefix,onto,"GOresult.txt",sep = '_')
  write.table(result.table,file=file.path(resultdir,suf_str),quote=FALSE,sep="\t",row.names=FALSE,col.names=TRUE,qmethod="escape")
  #modOut <- GenTable(GOdata, classic = resultclassic, weight = result, orderBy = "weight", topNodes = topNodesToPrint)
  modOut <- GenTable(GOdata, weight = result, orderBy = "weight", topNodes = topNodesToPrint)
  goIDnames <- modOut[,"GO.ID"]
  modOut <- modOut[,c("Significant", "Expected", "weight", "Term")]
  rownames(modOut) <- goIDnames
  f.obj2txt(xtable(modOut, caption = prefix), file.path(resultdir, paste(prefix,onto,"GOresult.tex",sep = '_')))
  #suf_str <- paste(prefix,onto,"GOresultClassic.txt",sep = '_')
  #write.table(result.table.cla,file=file.path(resultdir,suf_str),quote=TRUE,sep="\t",row.names=TRUE,col.names=TRUE,qmethod="escape")
  # return the GOdata
  return(GOdata)
}

f.topGO.ensembl <- function(universe, interest, resultdir, prefix = '', onto = "BP", dbToUse = "org.Mm.eg.db") {
  require("topGO")
  require("biomaRt")
  require(dbToUse, character.only = TRUE)
  require("xtable")
  # create geneList object (named factors)
  list.com <- factor(as.integer(universe %in% interest))
  names(list.com) <- universe
  # Test for BP terms - therefore:
  GOdata <- new("topGOdata", ontology = onto, allGenes = list.com, annot = annFUN.org, mapping=dbToUse)
  # Working with the topGOdata object (-> see documentation) run the tests
  result <- runTest(GOdata, algorithm = "weight", statistic = "fisher")
  #resultclassic <- runTest(GOdata, algorithm = "classic", statistic = "fisher")
  # Summarise the results - the tables above contain the same results but are ordered differently. Once by the test statistic using the WEIGHT method (T2) and once by the test statistic using the CLASSIC methog (T2.cla)
  # note: WEIGHT decorrelates the graph, CLASSIC not. In both cases, the test is FISHERs exact. The arguments are explained in topGO-Ref.pdf
  topNodesToPrint <- 50 ## NOTE that this only works if there are more then 50 GOterms
  result.table <- GenTable(GOdata, weight = result, orderBy = "weight", topNodes = topNodesToPrint)
  #result.table <- GenTable(GOdata, weight = result, classic = resultclassic, orderBy = "weight", ranksOf = "classic", topNodes = topNodesToPrint)
  #result.table.cla <- GenTable(GOdata, weight = result, classic = resultclassic, orderBy = "classic", ranksOf = "weight", topNodes = 50)
  # Write the tables as txt files (as I did)
  suf_str <- paste(prefix,onto,"GOresult.txt",sep = '_')
  write.table(result.table,file=file.path(resultdir,suf_str),quote=FALSE,sep="\t",row.names=FALSE,col.names=TRUE,qmethod="escape")
  #modOut <- GenTable(GOdata, classic = resultclassic, weight = result, orderBy = "weight", topNodes = topNodesToPrint)
  modOut <- GenTable(GOdata, weight = result, orderBy = "weight", topNodes = topNodesToPrint)
  goIDnames <- modOut[,"GO.ID"]
  modOut <- modOut[,c("Significant", "Expected", "weight", "Term")]
  rownames(modOut) <- goIDnames
  f.obj2txt(xtable(modOut, caption = prefix), file.path(resultdir, paste(prefix,onto,"GOresult.tex",sep = '_')))
  #suf_str <- paste(prefix,onto,"GOresultClassic.txt",sep = '_')
  #write.table(result.table.cla,file=file.path(resultdir,suf_str),quote=TRUE,sep="\t",row.names=TRUE,col.names=TRUE,qmethod="escape")
  # return the GOdata
  return(GOdata)
} 

f.topGO.terms.simple <- function(GOdata, GO_of_interest) {
  require("topGO")
  out <- list()
  for (intGO in GO_of_interest) {
    sigGOdata <- sigGenes(GOdata)
    go.genes <- genesInTerm(GOdata, intGO)[[1]]
    cat(paste(intGO, '\n', sep = ''))
    isec <- intersect(sigGOdata, go.genes)
    cat(paste(isec, collapse = '\n'))
    cat('\n')
    out[[intGO]] <- isec
  }
  return(out)
}

f.topGO.terms <- function(original, GOdata, GO_of_interest) {
  require("topGO")
  out <- list()
  for (intGO in GO_of_interest) {
    sigGOdata <- sigGenes(GOdata)
    go.genes <- genesInTerm(GOdata, intGO)[[1]]
    print(intGO)
    isec <- intersect(sigGOdata, go.genes)
    for (id in isec) {
      print(id)
      print(original[id,])
    }
    out[[intGO]] <- isec
  }
  return(out)
}

###############################################################################
## enrichment wrapper
###############################################################################

f.getTermToDescription <- function(x) {
  terms <- unlist(sapply(x[3], function(y) strsplit(y, ",", fixed = TRUE)))
  descriptions <- unlist(sapply(x[4], function(y) strsplit(y, "|", fixed = TRUE)))
  if (length(terms) != length(descriptions)) { 
    cat(paste("wrong size of line: ", paste(terms, collapse = "_"), "\n", sep = ''))
    cat("the error is likely caused by an entry (GB|AAG39646.1) with the pipe. Remove it:\nsed -i 's/(GB|AAG39646\\.1)/(GB_AAG39646\\.1)/g' geneToTerms.txt\n")
  } ## (GB|AAG39646.1) sed -i 's/(GB|AAG39646\.1)/(GB_AAG39646\.1)/g' geneToTerms.txt
  return(cbind(terms,descriptions))
}

f.enrichment.wrapper <- function(interest, universe, rDir, preprefix, mappingFile, onlyBP = TRUE)
{
  ## load gene to annotationterm maps (gene, nodetypeLeft, nodetypeRight, term)
  geneToAnno <- read.table(mappingFile, header = FALSE, quote = "", sep = "\t", stringsAsFactors = FALSE)
  colnames(geneToAnno) <- c("gene", "nodeTypeLeft", "nodeTypeRight", "term", "description")
  geneToAnno <- geneToAnno[grep("AT[[:alnum:]]{1}[[:alpha:]]{1}[[:digit:]]{5}", geneToAnno$gene),c("gene","nodeTypeRight","term","description")]

  termDescrTemp <- apply(geneToAnno, 1, function(x) f.getTermToDescription(x))
  termDescrTemp <- do.call("rbind",termDescrTemp)
  termDescrTemp <- unique(termDescrTemp)
  termDescr <- termDescrTemp[,"descriptions"]
  names(termDescr) <- termDescrTemp[,"terms"]

  if (universe == "all") { universe <- unique(geneToAnno$gene) }
  ## GeneOntology - generic
  cat("generic GO\n")
  agi2goTab <- subset(geneToAnno, nodeTypeRight == "goBP")
  agi2go <- sapply(agi2goTab$term, function(x) strsplit(x, ",", fixed = TRUE))
  names(agi2go) <- agi2goTab$gene
  genGOres <- f.generic.enrichment.test(interest, universe, agi2go, termDescr, rDir, paste(preprefix, "goBP", sep = "_")) 
  
  ## InterPro - generic
  cat("generic InterProFam\n")
  agi2ipTab <- subset(geneToAnno, nodeTypeRight == "iprFamily")
  agi2ip <- sapply(agi2ipTab$term, function(x) strsplit(x, ",", fixed = TRUE))
  names(agi2ip) <- agi2ipTab$gene
  genIPres <- f.generic.enrichment.test(interest, universe, agi2ip, termDescr, rDir, paste(preprefix, "ipFam", sep = "_")) 
  
  cat("generic InterProDom\n")
  agi2ipTab <- subset(geneToAnno, nodeTypeRight == "iprDomain")
  agi2ip <- sapply(agi2ipTab$term, function(x) strsplit(x, ",", fixed = TRUE))
  names(agi2ip) <- agi2ipTab$gene
  genIPres <- f.generic.enrichment.test(interest, universe, agi2ip, termDescr, rDir, paste(preprefix, "ipDom", sep = "_")) 
  
  ## PFAM - generic
  cat("generic PFAM\n")
  agi2pfTab <- subset(geneToAnno, nodeTypeRight == "pfam")
  agi2pf <- sapply(agi2pfTab$term, function(x) strsplit(x, ",", fixed = TRUE))
  names(agi2pf) <- agi2pfTab$gene
  genPFres <- f.generic.enrichment.test(interest, universe, agi2pf, termDescr, rDir, paste(preprefix, "pf", sep = "_")) 
  
  ## GeneOntology - topGO
  cat("topGO\n")
  if (onlyBP) {
    agi2goTopTab <- subset(geneToAnno, nodeTypeRight == "goBP")
    agi2goTop <- sapply(unique(agi2goTopTab$gene), function(x) strsplit(paste(agi2goTopTab$term[agi2goTopTab$gene == x], collapse = ','), ",", fixed = TRUE))
    GOdata <- f.topGO.wrapper(universe, interest, agi2goTop, rDir, paste(preprefix, "go", sep = "_") , 'BP')
  } else {
    agi2goTopTab <- subset(geneToAnno, nodeTypeRight == "goBP" | nodeTypeRight == "goMF" | nodeTypeRight == "goCC")
    agi2goTop <- sapply(unique(agi2goTopTab$gene), function(x) strsplit(paste(agi2goTopTab$term[agi2goTopTab$gene == x], collapse = ','), ",", fixed = TRUE))
    GOdata <- f.topGO.wrapper(universe, interest, agi2goTop, rDir, paste(preprefix, "go", sep = "_") , 'BP')
    GOdata <- f.topGO.wrapper(universe, interest, agi2goTop, rDir, paste(preprefix, "go", sep = "_") , 'MF')
    GOdata <- f.topGO.wrapper(universe, interest, agi2goTop, rDir, paste(preprefix, "go", sep = "_") , 'CC')
  }
  
  return(NULL)
}
