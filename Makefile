DIRS := src/monitor src/server

all: $(DIRS)

$(DIRS): clean
	$(MAKE) -C $@

clean:
	rm -rf build
	mkdir build
	rm -rf *.log

.PHONY: all $(DIRS)
