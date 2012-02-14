CPP=g++
CPPFLAGS=-O3 -Wall # -g
LDFLAGS=-lcrypto
TARGET=driver

all: $(TARGET)

OBJS = cbf.o sblookuptable.o util.o regexp.o conditions.o flexsample.o

$(TARGET): driver.cc $(OBJS)
	$(CPP) $(CPPFLAGS) $(LDFLAGS) -o $@ $< $(OBJS)

util.o: util.cc
	$(CPP) $(CPPFLAGS) -o $@ -c $<

regexp.o: regexp.cc
	$(CPP) $(CPPFLAGS) -o $@ -c $<

flexsample.o: flexsample.cc 
	$(CPP) $(CPPFLAGS) -o $@ -c $<

conditions.o: conditions.cc conditions.h
	$(CPP) $(CPPFLAGS) -o $@ -c $<
 
cbf.o: cbf.cc cbf.h
	$(CPP) $(CPPFLAGS) -o $@ -c $<

sblookuptable.o: sblookuptable.cc 
	$(CPP) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f *.o $(TARGET)
