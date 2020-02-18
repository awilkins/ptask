SUBDIRS = icons/hicolor/scalable \
	po \
	src \
	tests 

dist_doc_DATA = AUTHORS \
	COPYING \
	INSTALL \
	NEWS \
	NEWS.html \
	README \
	README.html

NEWS.html: NEWS style.css
	asciidoc --theme style -a themedir=`pwd` NEWS

desktopdir = $(datadir)/applications
desktop_DATA = ptask.desktop

EXTRA_DIST = $(desktop_DATA) style.css

README.html: README style.css
	asciidoc --theme style -a themedir=`pwd` -n -a toc README
