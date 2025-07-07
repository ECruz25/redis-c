# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS =

# Directories
SRCDIR = .
OBJDIR = obj
BINDIR = bin

# Source files
SERVER_SOURCES = server.c conn.c vector.c pollv.c
CLIENT_SOURCES = client.c
MAIN_SOURCES = main.c

# Object files
SERVER_OBJECTS = $(SERVER_SOURCES:%.c=$(OBJDIR)/%.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:%.c=$(OBJDIR)/%.o)
MAIN_OBJECTS = $(MAIN_SOURCES:%.c=$(OBJDIR)/%.o)

# Executables
SERVER_TARGET = $(BINDIR)/server
CLIENT_TARGET = $(BINDIR)/client
MAIN_TARGET = $(BINDIR)/main

# Default target
all: $(SERVER_TARGET) $(CLIENT_TARGET) $(MAIN_TARGET)

# Create directories if they don't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Server target
$(SERVER_TARGET): $(SERVER_OBJECTS) | $(BINDIR)
	$(CC) $(SERVER_OBJECTS) -o $@ $(LDFLAGS)

# Client target
$(CLIENT_TARGET): $(CLIENT_OBJECTS) | $(BINDIR)
	$(CC) $(CLIENT_OBJECTS) -o $@ $(LDFLAGS)

# Main target
$(MAIN_TARGET): $(MAIN_OBJECTS) | $(BINDIR)
	$(CC) $(MAIN_OBJECTS) -o $@ $(LDFLAGS)

# Object file compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Individual targets
server: $(SERVER_TARGET)
client: $(CLIENT_TARGET)
main: $(MAIN_TARGET)

# Clean target
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Clean only object files
clean-obj:
	rm -rf $(OBJDIR)

# Clean only binaries
clean-bin:
	rm -rf $(BINDIR)

# Run server
run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

# Run client
run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

# Run main
run-main: $(MAIN_TARGET)
	./$(MAIN_TARGET)

# Debug builds (with additional debug flags)
debug: CFLAGS += -DDEBUG -O0
debug: all

# Release builds (optimized)
release: CFLAGS += -O2 -DNDEBUG
release: all

# Install dependencies (if needed)
install-deps:
	@echo "No external dependencies to install"

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build all targets (server, client, main)"
	@echo "  server       - Build server only"
	@echo "  client       - Build client only"
	@echo "  main         - Build main only"
	@echo "  clean        - Remove all build artifacts"
	@echo "  clean-obj    - Remove only object files"
	@echo "  clean-bin    - Remove only binaries"
	@echo "  run-server   - Build and run server"
	@echo "  run-client   - Build and run client"
	@echo "  run-main     - Build and run main"
	@echo "  debug        - Build with debug flags"
	@echo "  release      - Build with optimization flags"
	@echo "  help         - Show this help message"

# Phony targets
.PHONY: all server client main clean clean-obj clean-bin run-server run-client run-main debug release install-deps help

# Dependencies (header files)
$(OBJDIR)/server.o: server.c conn.h vector.h pollv.h
$(OBJDIR)/conn.o: conn.c conn.h vector.h pollv.h
$(OBJDIR)/vector.o: vector.c vector.h
$(OBJDIR)/pollv.o: pollv.c pollv.h
$(OBJDIR)/client.o: client.c
$(OBJDIR)/main.o: main.c
