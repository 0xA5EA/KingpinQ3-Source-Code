VERSION=1.35_SVN
RELEASE=0

all: kingpinq3-$(VERSION)-$(RELEASE).x86.exe

kingpinq3.nsi: kingpinq3.nsi.in
	sed 's/XXXVERSIONXXX/$(VERSION)/;s/XXXRELEASEXXX/$(RELEASE)/' < $< > $@

kingpinq3-$(VERSION)-$(RELEASE).x86.exe: kingpinq3.nsi
	makensis kingpinq3.nsi

clean:
	rm -rf *.exe kingpinq3.nsi

.PHONY: all clean
