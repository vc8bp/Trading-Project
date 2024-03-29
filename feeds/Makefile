COMPILER = gcc
LDLIBS += -lboost_system -lboost_thread -lboost_filesystem
LDFLAGS += -lstdc++ -O3 -std=c++17

code = ./src/code/
includes = ./includes/

CFLAGS = -I$(includes)
BUILDDIR = ./bin/

$(shell mkdir -p $(BUILDDIR)) # -p will not throw error if it alredy exits

${BUILDDIR}final: ${BUILDDIR}main.o ${BUILDDIR}client.o ${BUILDDIR}queue.o ${BUILDDIR}server.o ${BUILDDIR}asyncServer.o
	@echo "Linking Project"
	@${COMPILER} $^ -o $@ ${LDLIBS} ${LDFLAGS}

run: ${BUILDDIR}final
	clear
	@echo "----------------------OUTPUT-------------------------"
	@./${BUILDDIR}final

${BUILDDIR}main.o: ${code}main.cpp 
	@echo "Compiling main.cpp"
	@${COMPILER} ${LDLIBS} -c $< -o $@ ${CFLAGS}

${BUILDDIR}client.o: ${code}client.cpp
	@echo "Compiling client.c"
	@${COMPILER} ${LDLIBS} -c $< -o $@ ${CFLAGS}

${BUILDDIR}queue.o: ${code}queue.cpp
	@echo "Compiling queue.c"
	@${COMPILER} ${LDLIBS} -c $< -o $@ ${CFLAGS}

${BUILDDIR}server.o: ${code}server.cpp
	@echo "Compiling server.c"
	@${COMPILER} ${LDLIBS} -c $< -o $@ ${CFLAGS}

${BUILDDIR}asyncServer.o: ${code}asyncServer.cpp 
	@echo "Compiling asyncServer.c"
	@${COMPILER} ${LDLIBS} -c $< -o $@ ${CFLAGS}

clean:
	@echo "Cleaning Build"
	rm -f $(BUILDDIR)*.o $(BUILDDIR)final


test: ${code}asyncServer.cpp
	@echo "Compiling asyncServer.c"
	@${COMPILER} ${LDLIBS} $< -o ${BUILDDIR}test  ${CFLAGS} ${LDFLAGS} -lzmq
	chmod +x ${BUILDDIR}test
	clear
	${BUILDDIR}test
