BFNNCONV_SRC = bfnnconv.pl m-ascii.pl m-html.pl m-info.pl m-lout.pl m-post.pl

FAQ = fftw-faq.ascii fftw-faq.html
EXTRA_DIST = fftw-faq.bfnn $(FAQ) $(BFNNCONV_SRC) html.refs

$(FAQ): $(BFNNCONV_SRC) fftw-faq.bfnn
	@echo converting...
	perl bfnnconv.pl fftw-faq.bfnn
	@echo converting again...
	perl bfnnconv.pl fftw-faq.bfnn

faq: $(FAQ)

clean-local:
	rm -f *~ core a.out *.lout *.ps *.info *.ascii *.xrefdb *.post
	rm -rf *.html
