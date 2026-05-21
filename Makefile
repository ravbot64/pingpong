CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wconversion -Wshadow -Wpedantic \
            -fstack-protector-strong -D_FORTIFY_SOURCE=2
LDFLAGS  ?=

SRCDIR  := src
TARGET  := pingpong

SOURCES := \
    $(SRCDIR)/main.cpp \
    $(SRCDIR)/common/config.cpp \
    $(SRCDIR)/common/signal.cpp \
    $(SRCDIR)/common/units.cpp \
    $(SRCDIR)/metrics/timer.cpp \
    $(SRCDIR)/metrics/throughput.cpp \
    $(SRCDIR)/network/socket.cpp \
    $(SRCDIR)/server/tcp_server.cpp \
    $(SRCDIR)/client/tcp_client.cpp

OBJECTS := $(SOURCES:.cpp=.o)
DEPS    := $(OBJECTS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(TARGET) $(OBJECTS) $(DEPS)

.PHONY: all clean
