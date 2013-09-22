default: clean all

ifeq ($(JAVA_INCLUDE),)
  $(error JAVA_INCLUDE not defined)
endif

ifeq ($(OUTPUT),)
  $(error OUTPUT not defined)
endif

ifneq (,$(findstring mingw32,$(HOST)))
  CFLAGS += -I"$(JAVA_INCLUDE)/win32" -static-libgcc -DBUILDING_DLL -Wl,--kill-at
else
ifneq (,$(findstring darwin,$(HOST)))
  CFLAGS += -I"$(JAVA_INCLUDE)/darwin"
else
ifneq (,$(findstring linux,$(HOST)))
  CFLAGS += -I"$(JAVA_INCLUDE)/linux"
else
  $(error Unknown or undefined HOST)
endif
endif
endif

CC = $(HOST)-gcc
CFLAGS += -shared -Ilibxmp/include -I"$(JAVA_INCLUDE)"

all:
	-cd libxmp; while read p; do patch -N -r - --no-backup-if-mismatch -p1 < ../patches/$$p; done < ../patches/series
	-cd libxmp; autoconf; ./configure --host=$$HOST; make
	mkdir -p $(shell dirname $(OUTPUT))
	$(CC) $(CFLAGS) xmp4j.c -Llibxmp/lib -lxmp -o $(OUTPUT)

clean:
	-make -C libxmp clean

.PHONY: all clean