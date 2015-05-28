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

all::	bin dts


CXX=g++-4.7

srcDir = ./
objDir = $(buildDir)/obj

CPPFLAGS += -fPIC -std=c++11
LDFLAGS += -lBlackLib -lrt

cppFiles = $(name).cpp Command.cpp CommandParallel.cpp CommandSequence.cpp \
	   GyroBNO055.cpp HBridge.cpp Servo.cpp Timer.cpp UserLeds.cpp

oFiles = $(cppFiles:%.cpp=$(objDir)/%.o)

# Include dependency files
-include $(oFiles:%.o=%.d)

$(objDir)/%.o::	$(srcDir)/%.cpp
	[ -d "$(objDir)" ] || install -d "$(objDir)";
	$(COMPILE.cc) -o $(@) $(@:$(objDir)/%.o=%.cpp)
	$(COMPILE.cc) -MM -MT $(@) -MF $(@:%.o=%.d) $(@:$(objDir)/%.o=%.cpp)

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

install::	/usr/sbin/avc /etc/init.d/avc /lib/firmware/timon-gpio-00A0.dtbo

/lib/firmware/timon-gpio-00A0.dtbo:	$(buildDir)/timon-gpio-00A0.dtbo
	cp -p $(?) $(@)

uninstall::
	@chkconfig avc off || true;
	rm -f /usr/sbin/avc /etc/init.d/avc;
	systemctl daemon-reload

clean::
	rm -fr $(buildDir)



$(buildDir)/%-00A0.dtbo:	%.dts
	dtc -O dtb -o $(@) -b 0 -@ $(?)

dts::	$(buildDir)/timon-gpio-00A0.dtbo

