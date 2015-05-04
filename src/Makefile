# Makefile which attempts to download and compile (or cross-compile) the
# BlackLib library for a BBB
#
# On BeagleBone:
#
#   make
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
LDFLAGS += -lBlackLib -lrt

cppFiles = $(name).cpp Command.cpp CommandParallel.cpp CommandSequence.cpp \
	   HBridge.cpp Servo.cpp Timer.cpp UserLeds.cpp

oFiles = $(cppFiles:%.cpp=$(objDir)/%.o)

# Include dependency files
-include $(oFiles:%.o=%.d)

$(objDir)/%.o::	$(srcDir)/%.cpp
	[ -d "$(objDir)" ] || install -d "$(objDir)";
	$(COMPILE.cc) -o $(@) $(@:$(objDir)/%.o=%.cpp)
	$(COMPILE.cc) -MM -MT "$(@)" -MF $(@:%.o=%.d) $(?)

$(buildDir)/$(name)::	$(oFiles)
	$(LINK.cpp) $(oFiles) -lBlackLib -o $(@)

bin::	$(buildDir)/$(name)

/usr/sbin/avc::	$(buildDir)/$(name)
	install --mode=755 $(buildDir)/$(name) $(@);

/etc/init.d/avc::	scripts/avc
	install --mode=755 scripts/avc $(@);
	chkconfig avc on;
	chkconfig --list avc;
	systemctl daemon-reload

install::	/usr/sbin/avc /etc/init.d/avc

uninstall::
	@chkconfig avc off || true;
	rm -f /usr/sbin/avc /etc/init.d/avc;
	systemctl daemon-reload

clean::
	rm -fr $(buildDir)
