CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRCDIR = src
SOURCES = $(SRCDIR)/main.cpp \
          $(SRCDIR)/common/config.cpp \
          $(SRCDIR)/common/units.cpp \
          $(SRCDIR)/metrics/timer.cpp \
          $(SRCDIR)/metrics/throughput.cpp \
          $(SRCDIR)/network/socket.cpp \
          $(SRCDIR)/server/tcp_server.cpp \
          $(SRCDIR)/client/tcp_client.cpp

TARGET = pingpong

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: all clean
