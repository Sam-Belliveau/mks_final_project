# Directories for compiling results
BIN=./bin
OBJ=./obj
SRC=./src

# Output for binary
SERVER=$(BIN)/shell_server
CLIENT=$(BIN)/shell_client

# Main files for server and client
SERVER_MAIN=./server.c
CLIENT_MAIN=./client.c

# Get headers and c files
DEPS=$(wildcard $(SRC)/*.h)
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

# Compiler / Compiler Settings
LINKS=-lm
FLAGS=-O2
COMPILER=gcc $(FLAGS)

# Command to create directory
MKDIR=mkdir

# Compile the Binary
server: $(SERVER)
client: $(CLIENT)

$(SERVER): $(SERVER_MAIN) $(OBJS) 
	$(MKDIR) -p $(BIN)
	$(COMPILER) $^ -o $@ $(LINKS)

$(CLIENT): $(CLIENT_MAIN) $(OBJS) 
	$(MKDIR) -p $(BIN)
	$(COMPILER) $^ -o $@ $(LINKS)

# Compile Every Object
$(OBJ)/%.o: $(SRC)/%.c $(DEPS)
	$(MKDIR) -p $(@D)
	$(COMPILER) -c $< -o $@

# Run the binary
run_server: $(SERVER)
	$(SERVER)

run_client: $(CLIENT)
	$(CLIENT)

# Clean make output
clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)