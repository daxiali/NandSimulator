SOURCES = $(wildcard *.c)
OBJDIR = ./obj
OBJECTS = $(patsubst %.c, %.o, ${SOURCES})
INCLUDE = -I../include \
		  -I../lib/misc \
		  -I../lib/brotli/include
LDFLAGS = -L../lib/misc/ -lmisc \
		  -L../lib/brotli/ -lbrotli
LIB_A = libnand.a
CFLAGS += -O0 -g

.PHONY: all lib clean

all: $(OBJECTS) $(OBJDIR)
	
$(OBJDIR):
	mkdir -p $@

$(OBJECTS): %.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@ $(LDFLAGS) -static

lib: all
	rm -f $(LIB_A)
	ar -crs $(LIB_A) $(OBJECTS)
	mv $(OBJECTS) $(OBJDIR)

clean:
	rm -f $(LIB_A)
	rm -rf $(OBJDIR)
	rm -f *.o
