CC=g++
LD=$(CC)
CFLAGS=-O2 -g -fvisibility=hidden
CPPFLAGS=-I../Thirdparty/iPlug2 -I../Thirdparty/hiir-1.33 -I../Thirdparty/x42 $(CAIRO_CPPFLAGS)
INSTALL_DIR = /usr/local/lib/lv2
CAIRO_CPPFLAGS=$(shell pkg-config cairo --cflags)
CAIRO_LDFLAGS=$(shell pkg-config cairo --libs)
ifndef ARCHNAME
OEXT=.o
SOEXT=.so
ARCCFLAG=
ARCLFLAG=
DLLNAME=Molot_Mono_Lite
else
DLLNAME=so
OEXT=_$(ARCHNAME).o
SOEXT=.$(ARCHNAME)
ifeq ($(ARCHNAME),x86_64)
ARCCFLAG=-m64
else
ARCCFLAG=-m32
endif
ARCLFLAG=-melf_$(ARCHNAME)
endif
ifdef LV2CREATE
UNAME := $(shell uname -m)
COPYNAME=so
else
UNAME=so
COPYNAME=Molot_Mono_Lite
endif
BUNDLE=Molot_Mono_Lite.lv2
OBJECTS=Molot_Mono_Lite$(OEXT) moloteng$(OEXT)
all: $(DLLNAME)$(SOEXT) ladspa$(SOEXT)

Molot_Mono_Lite$(OEXT): Molot_Mono_Lite.cpp Molot_Mono_Lite.h plugin.h
	$(CC) $(ARCCFLAG) -Wall -fPIC $(CFLAGS) $(CPPFLAGS) -o Molot_Mono_Lite$(OEXT) -c Molot_Mono_Lite.cpp

plugin$(OEXT): plugin.cpp Molot_Mono_Lite.h plugin.h
	$(CC) $(ARCCFLAG) -Wall -fPIC $(CFLAGS) $(CPPFLAGS) -o plugin$(OEXT) -c plugin.cpp

moloteng$(OEXT): moloteng.cpp plugin.h Molot_Mono_Lite.h moloteng.h
	$(CC) $(ARCCFLAG) -Wall -fPIC $(CFLAGS) $(CPPFLAGS) -o moloteng$(OEXT) -c moloteng.cpp

$(DLLNAME)$(SOEXT): $(OBJECTS) plugin$(OEXT)
	$(LD) $(ARCLFLAG) -o $(DLLNAME)$(SOEXT) plugin$(OEXT) $(OBJECTS) $(CAIRO_LDFLAGS) -shared

ladspa$(OEXT): ladspa.cpp Molot_Mono_Lite.h plugin.h
	$(CC) $(ARCCFLAG) -Wall -fPIC $(CFLAGS) $(CPPFLAGS) -o ladspa$(OEXT) -c ladspa.cpp

ladspa$(SOEXT): $(OBJECTS) ladspa$(OEXT)
	$(LD) $(ARCLFLAG) -o ladspa$(SOEXT) ladspa$(OEXT) $(OBJECTS) -shared

install:
	mkdir -p $(INSTALL_DIR)/$(BUNDLE)
	cp -f manifest.ttl $(INSTALL_DIR)/$(BUNDLE)/manifest.ttl
	cp -f Molot_Mono_Lite.ttl $(INSTALL_DIR)/$(BUNDLE)/Molot_Mono_Lite.ttl
	cp -f $(COPYNAME).$(UNAME) $(INSTALL_DIR)/$(BUNDLE)/Molot_Mono_Lite.so
	chmod 775 $(INSTALL_DIR)/$(BUNDLE)/*.ttl

uninstall:
	rm -rf $(INSTALL_DIR)/$(BUNDLE)

clean:
	rm -f *.o *.so
