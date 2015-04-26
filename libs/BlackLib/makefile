# Makefile which attempts to download and compile (or cross-compile) the
# BlackLib library for a BBB
#
# On BeagleBone:
#
#   make
#   make install
#   make uninstall
#
# Cross compiling
#
#   make [CCPREFIX=arm-linux-gnueabihf-]
#   # Then transfer .tar.gz file under build directory to BBB and extract
# 
name = BlackLib
version = 2
srcName = $(name)-v$(version)_0.zip
libName = lib$(name).so.$(version)
srcUrl = http://downloads.sourceforge.net/project/blacklib/$(srcName)
tarFile = $(buildDir)/$(name)-bin-$(version)_0.tar.gz
buildDir = build

ifndef prefix
  prefix = /usr/local
endif

ifndef DESTDIR
  DESTDIR = $(buildDir)/dest
endif

all::	$(tarFile)
	@echo "\n\nCreated tar file: $(tarFile). Contents:\n";
	@tar tzf $(tarFile)

install::	$(tarFile)
	tar xzf $(tarFile) -C /

uninstall::
	for f in $$(tar tzf $(tarFile)); do \
	  if [ -f "/$${f}" ] || [ -L "/$${f}" ]; then rm -f "/$${f}"; fi; \
	done

$(buildDir)/config.mk::	makefile
	install -d $(buildDir)
	if [ -x /usr/bin/arm-linux-gnueabihf-g++ ]; then \
	  printf "ifndef CCPREFIX\n  CCPREFIX = arm-linux-gnueabihf-\nendif" >| $(@); \
	else \
	  printf "ifndef CCPREFIX\n  CCPREFIX =\nendif" >| $(@); \
	fi

sinclude $(buildDir)/config.mk

CXX=$(CCPREFIX)g++

srcDir = $(buildDir)/BlackLib-v$(version)_0
objDir = $(buildDir)/obj

CPPFLAGS += -fPIC -std=c++11

cppFiles = BlackADC.cpp BlackCore.cpp BlackGPIO.cpp BlackI2C.cpp BlackPWM.cpp\
 BlackSPI.cpp BlackUART.cpp
hFiles = BlackADC.h BlackCore.h BlackDef.h BlackErr.h BlackGPIO.h BlackI2C.h BlackLib.h BlackPWM.h BlackSPI.h BlackUART.h
oFiles = $(cppFiles:%.cpp=$(objDir)/%.o)

$(objDir)/%.o::	$(srcDir)/%.cpp
	[ -d "$(objDir)" ] || install -d "$(objDir)";
	$(COMPILE.cc) -o $(@) $(?)

$(buildDir)/$(srcName)::
	[ -s "$(@)" ] || (install -d "$$(dirname "$(@)")" && wget -O "$(@)" "$(srcUrl)");

download::	$(buildDir)/$(srcName)

$(srcDir)::	$(buildDir)/$(srcName)
	(cd $(buildDir) && unzip $(srcName))
	sed -i -e 's,\(namespace BlackLib\),#include <unistd.h>\n\1,' \
	  $(srcDir)/BlackI2C.cpp $(srcDir)/BlackSPI.cpp $(srcDir)/BlackUART.cpp

src::	$(srcDir) 

$(buildDir)/$(libName)::	$(srcDir)
	$(MAKE) $(oFiles)
	$(LINK.cpp) -shared $(oFiles) -o $(@)

lib::	$(buildDir)/$(libName)

$(tarFile)::	$(buildDir)/$(libName)
	rm -fr $(DESTDIR)
	install -D --mode=755 $(buildDir)/$(libName) $(DESTDIR)$(prefix)/lib/$(libName)
	ln -s lib$(name).so.$(version) $(DESTDIR)$(prefix)/lib/lib$(name).so
	for f in $(hFiles); do install -D --mode=644 $(srcDir)/$${f} $(DESTDIR)$(prefix)/include/$${f}; done
	tar czf $(@) -C $(DESTDIR) $(prefix:/%=%)/lib/$(libName) $(prefix:/%=%)/lib/lib$(name).so $(hFiles:%=$(prefix:/%=%)/include/%)

clean::
	rm -fr $(srcDir) $(objDir)

clear::
	rm -fr $(buildDir)

$(buildDir)/src::
	install -d $(buildDir)/src;

$(buildDir)/src/%.o::	$(buildDir)/src/%.cpp
	$(COMPILE.cpp) $(?) -c -o $(@)

checkout::
	git clone $(gitUrl) $(buildDir)/src

