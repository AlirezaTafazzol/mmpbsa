install_dir=.
PROGRAMS=mmpbsa.template
MANPAGES=$(PROGRAMS:.template=.1)
XMLS=$(PROGRAMS:.template=.xml)
XHTMLS=$(PROGRAMS:.template=.xhtml)
PRODUCTS=$(XMLS) $(MANPAGES) $(XHTMLS)

.PHONY: clean all install

all: $(PRODUCTS)

install: all
	mkdir -p $(install_dir)/man
	\cp $(MANPAGES) $(install_dir)/man/
	mkdir -p $(install_dir)/xhtml
	\cp $(XHTMLS) $(install_dir)/xhtml/
	mkdir -p $(install_dir)/xml
	\cp $(XMLS) $(install_dir)/xml/

%.xml : %.template
	cat $< |sed '/[^\\]%author/r author.xml'|sed '/[^\\]%author/d' > $@
	sed '/[^\\]%planefiles/r planefiles.xml' $@|sed '/[^\\]%planefiles/d' > $@.tmp
	\mv $@.tmp $@
	sed  "s/[^\\]%command/<command>$(@:.xml=)<\/command>/g" $@ > $@.tmp
	\mv $@.tmp $@
	sed 's/\\%/%/g' $@ > $@.tmp
	\mv $@.tmp $@

%.1 : %.xml
	xmlto man $<

%.xhtml : %.xml
	xmlto xhtml $<
	mv index.html $@

clean:
	rm -f $(XMLS) $(MANPAGES)
