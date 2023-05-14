# Compiler
CC = gcc

# Targets
all: stnc

# Build stnc
stnc: stnc.o partB.o common.o server.o client.o
	@echo "Building stnc..."
	@$(CC) -o stnc stnc.o partB.o common.o server.o client.o
	@echo "stnc built successfully."
	@echo "╔══════════════════════════════════════════════════════════════════════════╗"
	@echo "║                                  Usage                                   ║"
	@echo "╠══════════════════════════════════════════════════════════════════════════╣"
	@echo "║ Part A:                                                                  ║"
	@echo "║   Client: ./stnc -[c] <ip> <port>                                        ║"
	@echo "║   Server: ./stnc -[s] <port>                                             ║"
	@echo "║ Part B:                                                                  ║"
	@echo "║   Client: ./stnc -[c] <ip> <port> -[p] <type> <param>                    ║"
	@echo "╚══════════════════════════════════════════════════════════════════════════╝"

# Build isTest
isTest: isTest.o partB.o
	@echo "Building isTest..."
	@$(CC) -o isTest isTest.o partB.o
	@echo "isTest built successfully."

# Compile object files
%.o: %.c
	@echo "Compiling $<..."
	@$(CC) -c $< -o $@
	@echo "Compiled $<."

# Clean up
clean:
	@echo "Cleaning up..."
	@rm -f stnc isTest *.o *.txt
	@echo "Cleaned up."
