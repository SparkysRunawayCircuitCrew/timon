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
name = timon
buildDir = build

ifndef prefix
  prefix = /usr/local
endif

ifndef DESTDIR
  DESTDIR = $(buildDir)/dest
endif

all::	bin


CXX=g++-4.7

srcDir = ./
objDir = $(buildDir)/obj

CPPFLAGS += -fPIC -std=c++11

cppFiles = main.cpp
oFiles = $(cppFiles:%.cpp=$(objDir)/%.o)

$(objDir)/%.o::	$(srcDir)/%.cpp
	[ -d "$(objDir)" ] || install -d "$(objDir)";
	$(COMPILE.cc) -o $(@) $(?)

$(buildDir)/$(name)::	$(oFiles)
	$(LINK.cpp) $(oFiles) -lBlackLib -o $(@)

bin::	$(buildDir)/$(name)

clean::
	rm -fr $(buildDir)
