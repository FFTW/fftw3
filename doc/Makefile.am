info_TEXINFOS = fftw3.texi
fftw3_TEXINFOS = fftw3.texi version.texi

DVIPS = dvips -Pwww

fftw3.dvi: rfftwnd.eps
fftw3.pdf: rfftwnd.pdf

rfftwnd.eps: rfftwnd.fig
	fig2dev -L eps -m .7 ${srcdir}/rfftwnd.fig rfftwnd.eps

rfftwnd.png: rfftwnd.fig
	fig2dev -L png -m 1 ${srcdir}/rfftwnd.fig rfftwnd.png

rfftwnd.pdf: rfftwnd.fig
	fig2dev -L pdf -m .7 ${srcdir}/rfftwnd.fig rfftwnd.pdf


EQN_IMAGES = equation-dft.gif equation-dht.gif equation-idft.gif	\
equation-redft00.gif equation-redft01.gif equation-redft10.gif		\
equation-redft11.gif equation-rodft00.gif equation-rodft01.gif		\
equation-rodft10.gif equation-rodft11.gif

EXTRA_DIST = fftw3.pdf html rfftwnd.fig $(EQN_IMAGES)

html: $(fftw3_TEXINFOS) $(EQN_IMAGES) rfftwnd.png
	$(MAKEINFO) $(AM_MAKEINFOFLAGS) $(MAKEINFOFLAGS) -I $(srcdir) \
		--html --number-sections -o html fftw3.texi
	for i in $(EQN_IMAGES); do cp -f ${srcdir}/$$i html; done
	cp -f rfftwnd.png html

clean-local:
	rm -f *.html
