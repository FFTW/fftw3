info_TEXINFOS = fftw3.texi
fftw3_TEXINFOS = fftw3.texi version.texi

# fig2dev gif output sucks.
# rfftwnd.gif: rfftwnd.fig
# 	fig2dev -L gif rfftwnd.fig rfftwnd.gif

fftw3.ps: rfftwnd.eps
fftw3.pdf: rfftwnd.eps

rfftwnd.eps: rfftwnd.fig
	fig2dev -L ps -m .7 rfftwnd.fig rfftwnd.eps
