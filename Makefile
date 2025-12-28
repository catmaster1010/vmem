TARGET=vmem
CFLAGS+=-g -Wno-builtin-declaration-mismatch
override SOURCES := $(shell find . -type f -name '*.c')

all:  $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS)  -o $(TARGET) $(SOURCES) 

clean:
	rm -f vmem
