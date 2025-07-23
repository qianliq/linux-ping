ifeq ($(ARCH), x86_64)
	CROSS_COMPILE=x86_64-linux-musl-
else ifeq ($(ARCH), riscv64)
	CROSS_COMPILE=riscv64-linux-musl-
endif

CC=$(CROSS_COMPILE)g++
CFLAGS=-Wall -Wextra -std=c++11 -Isrc
LDFLAGS=-static

# Source files and object files
SRC_DIR=src
PING_SRC_DIR=$(SRC_DIR)/ping
SOURCES=$(SRC_DIR)/main.cpp \
	$(PING_SRC_DIR)/client.cpp \
	$(PING_SRC_DIR)/packet.cpp \
	$(PING_SRC_DIR)/statistics.cpp
OBJECTS=$(SOURCES:.cpp=.o)
TARGET=ping

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Dependencies
$(SRC_DIR)/main.o: $(SRC_DIR)/main.cpp $(PING_SRC_DIR)/ping.h
$(PING_SRC_DIR)/client.o: $(PING_SRC_DIR)/client.cpp $(PING_SRC_DIR)/ping.h
$(PING_SRC_DIR)/packet.o: $(PING_SRC_DIR)/packet.cpp $(PING_SRC_DIR)/ping.h
$(PING_SRC_DIR)/statistics.o: $(PING_SRC_DIR)/statistics.cpp $(PING_SRC_DIR)/ping.h

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: install clean fmt debug
install: all
	mv $(TARGET) $(DADK_CURRENT_BUILD_DIR)/$(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)

fmt:
