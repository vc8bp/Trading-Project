COMPILER = g++
LDLIBS = -lstdc++ -O3 -std=c++20
LDLIBS += -lssl -lcrypto -lboost_system -lboost_thread -lboost_filesystem 

code = ./src/code/
includes = ./src/includes/

CFLAGS = -I$(includes)
BUILDDIR = ./bin/

$(shell mkdir -p $(BUILDDIR)) # -p will not throw error if it alredy exits

${BUILDDIR}final: ${BUILDDIR}main.o ${BUILDDIR}api.o
	@echo "Linking Project"
	${COMPILER} -o $@ $^ ${LDLIBS} 

run: ${BUILDDIR}final
	clear
	@echo "----------------------OUTPUT-------------------------"
	@./${BUILDDIR}final

${BUILDDIR}api.o: ${code}api.cpp
	echo "Compiling api.cpp"
	${COMPILER} -c $< -o $@ ${CFLAGS}

${BUILDDIR}main.o: ${code}main.cpp ${BUILDDIR}api.o
	echo "Compiling main.cpp"
	${COMPILER} -c $< -o $@ ${CFLAGS}

clean:
	@echo "Cleaning Build"
	rm -f $(BUILDDIR)*.o $(BUILDDIR)final
