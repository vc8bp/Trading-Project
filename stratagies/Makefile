CC = g++
CFLAGS = -Wall

LIBRARY = libmessageGetter.so
TARGET = sampleStrat

all: $(LIBRARY) $(TARGET)

$(LIBRARY): messageGetter.cpp
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

$(TARGET): sampleStrat.cpp $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@ -ldl -Wl,-rpath,'$$ORIGIN'

clean:
	rm -f $(LIBRARY) $(TARGET)
