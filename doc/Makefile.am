info_TEXINFOS = fftw3.texi
fftw3_TEXINFOS = fftw3.texi version.texi

# fig2dev gif output sucks.
# rfftwnd.gif: rfftwnd.fig
# 	fig2dev -L gif rfftwnd.fig rfftwnd.gif

fftw3.ps: rfftwnd.eps
fftw3.pdf: rfftwnd.eps

rfftwnd.eps: rfftwnd.fig
	fig2dev -L ps -m .7 rfftwnd.fig rfftwnd.eps

EQN_IMAGES = equation-dft.gif equation-dht.gif equation-idft.gif	\
equation-redft00.gif equation-redft01.gif equation-redft10.gif		\
equation-redft11.gif equation-rodft00.gif equation-rodft01.gif		\
equation-rodft10.gif equation-rodft11.gif

EXTRA_DIST = fftw.pdf html rfftwnd.fig

html: $(fftw3_TEXINFOS) $(EQN_IMAGES) rfftwnd.gif
	$(MAKEINFO) $(AM_MAKEINFOFLAGS) $(MAKEINFOFLAGS) -I $(srcdir) \
		--html --number-sections -o html fftw3.texi
	cp -f $(EQN_IMAGES) html
	cp -f rfftwnd.gif html

clean-local:
	rm -f *.html