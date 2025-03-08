# 主 Makefile
TOPSRCDIR = $(shell pwd)
DESTDIR = /usr/local/bin/
export TOPSRCDIR DESTDIR
SUBDIRS = droomc droomd kmod
.PHONY: all clean install
all:
	@for x in $(SUBDIRS); do (cd $$x && make all) || exit 1; done
clean:
	@for x in $(SUBDIRS); do (cd $$x && make clean) || exit 1; done
install:
	@for x in $(SUBDIRS); do (cd $$x && make install) || exit 1; done
	@echo
	@echo
	@if [ -a /etc/droom.conf ]; then \
	    echo "========================" && \
	    echo "" && \
	    echo "/etc/droom.conf exists."; \
	    echo "" && \
	    echo "You May Need to Edit /etc/droom.conf" && \
	    echo "" && \
	    echo "========================" \
	; \
	else \
	    install -m 600 droom.conf /etc/droom.conf && \
	    echo "========================" && \
	    echo "" && \
	    echo "Do Not Forget To Edit /etc/droom.conf" && \
	    echo "" && \
	    echo "========================" \
	; \
	fi
	@echo
