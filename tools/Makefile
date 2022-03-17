SUBDIRS = ../lib ../nand
SOURCES = $(wildcard *.c)
EXECUTABLE = $(patsubst %.c, %, ${SOURCES})
INCLUDE = -I../include \
		  -I../nand \
		  -I../lib/brotli/include \
		  -I../lib/misc
LDFLAGS = -L../nand/ -lnand \
		  -L../lib/misc/ -lmisc \
		  -L../lib/brotli/ -lbrotli
CFLAGS += -O0 -g

define make_subdir
    @for subdir in $(SUBDIRS) ; do \
    ($(MAKE) -C $$subdir $1) \
    done;
endef

.PHONY: all lib clean

all: ${EXECUTABLE}

${EXECUTABLE}: %: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS) -static

lib:
	$(call make_subdir , lib)

clean:
	$(call make_subdir , clean)
	find . -not -name "*.c" -not -name "Makefile" -delete
