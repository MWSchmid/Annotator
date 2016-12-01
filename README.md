# Annotator
Combine annotations from several sources.

# Dependencies:

You need to download, compile and install:
* QtFTP
* QtHTTP
* QtIOcompressor (in the repo as well)

Clone with git

```SH
git clone https://code.qt.io/qt/qtftp.git
git clone https://codereview.qt-project.org/qt/qthttp
```

Open all three in Qt creator and build them. For QtFTP and QtHTTP:

```SH
cd /path/to/build/folder
sudo make install
```

# Modify

The annotator is kind of experimental. The input files are hard-coded. Open the project in QtCreator and:

* annotatorview.cpp - at the bottom there are some commands like:
    * downloadFile()
        * the first three are fine (GO, InterPro and PFAM)
        * the others are organism specific. These "mapping files" should have a gene ID is in the first column (tab-separated). The rest is then searched for GO/InterPro/PFAM terms. There may be as many files as you like.
    * loadFile()
        * fine like this
        * note that loadFile() is ONLY for GO, InterPro and PFAM files specified in the download section
    * loadMappings()
        * replace the current files with the files you specified before

Build and start it on the command line:

```SH
annotator /path/to/working/dir
```

In the GUI, click:

* download
* loadAnno
* linkAnno
* saveCyto (if you want a network of the annotation for Cytoscape, it's in XGMML format)
* exportTable - gives a table with five columns (useful for annotation in R - e.g. with topGO):
    * ID of the left node
    * type of the left node
    * type of the right node
    * ID of the right node
    * description (of the right node)

Note: the input field in the bottom-left corner takes any Qt-RegExp and searches the entire annotation for matches (you can for example search for "gametophyte.\*cytokinin|cytokinin.\*gametophyte" to get all genes and terms associated with the words cytokinin AND gametophyte). All searches will be stored in the list above and can be saved with "saveExpr" and later on loaded with "loadExpr". "LoadSubset" can be used to load a pre-defined set of genes, and save results dumps whatever was found to a file.

# Use the table in R

```R
source("/path/to/Annotator/enrichmentWrappers.R")

interest <- "a set of gene IDs to test for enrichment"
universe <- "a larger set of gene IDs - the background"
rDir <- "/path/to/a/directory/for/the/results"
prePrefix <- "just something added to the file name"
mappingFile <- "/path/to/the/table/exported/above.txt"
f.enrichment.wrapper(interest, universe, rDir, preprefix, mappingFile, onlyBP = FALSE)

# note that the output of topGO contains simply the 50 top nodes (whether significant or not)
# and - on top of the txt output, there is also tex output (still needs to be integrated in a document though)
```




