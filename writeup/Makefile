
LATEX=pdflatex -halt-on-error
BIBTEX=bibtex

all: writeup.pdf

writeup.pdf: writeup.tex bibliography.bib
	$(LATEX) writeup
	$(BIBTEX) writeup
	$(LATEX) writeup
	$(LATEX) writeup

clean:
	rm -f *.aux *.bbl *.blg *.log *.pdf
