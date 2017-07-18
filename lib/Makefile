SUBDIRS = brotli misc

define make_subdir
    @for subdir in $(SUBDIRS) ; do \
    ($(MAKE) -C $$subdir $1) \
    done;
endef

.PHONY: all lib clean

all:
	$(call make_subdir , all)

lib:
	$(call make_subdir , lib)

clean:
	$(call make_subdir , clean)
