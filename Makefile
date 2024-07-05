INCDIR = include
SRCDIR = src
OBJDIR = obj

CC = clang

WARNFLAGS = -Wall -Wextra -Wpedantic -Wno-strict-prototypes -Wno-declaration-after-statement -Wno-missing-prototypes -Wno-unsafe-buffer-usage -Weverything
DEBUGFLAGS = -g -fno-omit-frame-pointer
RELEASEFLAGS = -O3 -flto -march=native -mtune=native -mllvm -unroll-count=32
ASANFLAGS = -O2 -fsanitize=address

# build: $(WARNFLAGS) $(RELEASEFLAGS)
# asan: $(WARNFLAGS) $(DEBUGFLAGS) $(ASANFLAGS)
# debug: $(WARNFLAGS) $(DEBUGFLAGS)
CFLAGS = -I$(INCDIR) $(WARNFLAGS) $(DEBUGFLAGS) $(ASANFLAGS)
LFLAGS = -rdynamic

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
TARGET = collatz

all: $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $^ -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(TARGET)