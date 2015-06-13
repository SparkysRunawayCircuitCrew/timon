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

$(NAME)::	all

CXX=g++-4.7

srcDir = ./
objDir = $(buildDir)/obj

CPPFLAGS += -fPIC -std=c++11
LDFLAGS += -lBlackLib -lrt

cppFiles = $(name).cpp Command.cpp CommandParallel.cpp CommandSequence.cpp \
	   GyroBNO055.cpp HBridge.cpp Servo.cpp Timer.cpp UserLeds.cpp

# C++ files unique to timon
ifeq ($(name),timon)
cppFiles += TimonDriveStraight.cpp
endif

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
	systemctl stop avc || true;
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

$(buildDir)/%-00A0.dtbo:	%.dts
	dtc -O dtb -o $(@) -b 0 -@ $(?)

/lib/firmware/timon-gpio-00A0.dtbo:	$(buildDir)/timon-gpio-00A0.dtbo
	cp -p $(?) $(@)

dts::	$(buildDir)/timon-gpio-00A0.dtbo

dts-install::	 /lib/firmware/timon-gpio-00A0.dtbo

# Helper method to semi-install necessary runtime files to run on
# other partition (like the local MMC)
#
#   sudo make disk-install DISK=/media/BEAGLEBONE_ 
#
# NOTE: You must have the chkconfig, sysvinit-utils, libc6, libstdc++6
# and libgcc1 packages installed

disk-install::	install

disk-install::
	if [ ! -d "$(DISK)" ]; then \
	  echo "***ERROR*** You must specify the mount point of the disk like:"; \
	  echo "  sudo make disk-install DISK=/media/BEAGLEBONE_ "; \
	  exit 1; \
	fi
	rsync -avh /usr/local/lib/libBlack* $(DISK)/usr/local/lib;
	rsync -avh /etc/init.d/avc $(DISK)/etc/init.d;
	rsync -avh /usr/sbin/avc $(DISK)/usr/sbin;
	cp -p $(buildDir)/timon-gpio-00A0.dtbo $(DISK)/lib/firmware/timon-gpio-00A0.dtbo;
	chroot $(DISK) chkconfig --add avc;
	chroot $(DISK) ldconfig;
	@echo -e "Installed so you should be able to boot from $(DISK) now";
