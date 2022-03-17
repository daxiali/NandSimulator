
LIBSOURCES = $(wildcard common/*.c) $(wildcard dec/*.c) \
             $(wildcard enc/*.c)
SOURCES = $(LIBSOURCES) tools/brotli.c
BINDIR = bin
OBJDIR = $(BINDIR)/obj
LIBOBJECTS = $(addprefix $(OBJDIR)/, $(LIBSOURCES:.c=.o))
OBJECTS = $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))
LIB_A = libbrotli.a
EXECUTABLE = brotli
DIRS = $(OBJDIR)/common $(OBJDIR)/dec $(OBJDIR)/enc \
       $(OBJDIR)/tools $(BINDIR)/tmp
CFLAGS += -O0 -g

all: $(EXECUTABLE)
	@:

.PHONY: all clean

$(DIRS):
	mkdir -p $@

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -lm -o $(BINDIR)/$(EXECUTABLE)

lib: $(LIBOBJECTS)
	rm -f $(LIB_A)
	ar -crs $(LIB_A) $(LIBOBJECTS)

clean:
	rm -rf $(BINDIR) $(LIB_A)
	
.SECONDEXPANSION:
$(OBJECTS): $$(patsubst %.o,%.c,$$(patsubst $$(OBJDIR)/%,%,$$@)) | $(DIRS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -Iinclude \
        -c $(patsubst %.o,%.c,$(patsubst $(OBJDIR)/%,%,$@)) -o $@
