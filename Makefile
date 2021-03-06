all:

USR_FILES := $(shell git ls-files usr/bin usr/sbin)
TOOLS_FILES := $(shell git ls-files tools)

.PHONY: all



.PHONY: tpkgs

tpkgs:
	mkdir -p build
	cd build && cmake ../tpkgs
	$(MAKE) -C build



.PHONY: install install-exec
install: install-exec
install-dirs:
	mkdir -p $(sort $(foreach dir,${USR_FILES},${DESTDIR}/$(dir ${dir})))
ifneq "$(wildcard ${DESTDIR}/tools)" ""
	mkdir -p $(sort $(foreach dir,${TOOLS_FILES},${DESTDIR}/$(dir ${dir})))
endif

install-exec: install-dirs tpkgs
	$(foreach p,$(USR_FILES), ln -fs $$(pwd)/$p ${DESTDIR}/$p;)
ifneq "$(wildcard ${DESTDIR}/tools)" ""
	$(foreach p,$(TOOLS_FILES), ln -fvs $$(pwd)/$p ${DESTDIR}/$p;)
endif
	cp build/src/tpkgs /usr/bin



.PHONY: gather apply

gather:
	./gather-package-data.sh
	./gather-etc-data.sh

apply:
	./apply-package-data.sh
	./apply-etc-data.sh
