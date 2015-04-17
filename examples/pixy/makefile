all::	bin/pixy-usb

bin/pixy-usb::
	install -d build
	(cd build && cmake ../pixy-usb && make)
	install -d bin
	install --mode=755 build/pixy-usb bin/pixy-usb

clean::
	rm -fr build

clear::	clean
	rm -fr bin
