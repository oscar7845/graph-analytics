# Compiler and flags (GCC)
CC      = gcc
CFLAGS  = -DGCC -DPARALLEL -fopenmp -funroll-loops -funroll-all-loops -O2 \
          -I../../original-triangle-counting -DTEST_TC_FAST

# Source files
SRCS    = \
    ../../original-triangle-counting/main.c \
    ../../original-triangle-counting/graph.c \
    ../../original-triangle-counting/bfs.c \
    ../../original-triangle-counting/queue.c \
    ../../original-triangle-counting/tc.c \
    ../../original-triangle-counting/tc_parallel.c \
    tc.c

# Output executable
TARGET  = tc_p2_deepseek_r1

# Build rule
all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) -lm

# Clean up
clean:
	rm -f $(TARGET)
