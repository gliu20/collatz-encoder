INCDIR = include
SRCDIR = src
OBJDIR = obj

CC = clang

WARNFLAGS = -Wall -Wextra -Wpedantic -Wno-strict-prototypes -Wno-declaration-after-statement -Wno-missing-prototypes -Wno-unsafe-buffer-usage -Weverything
DEBUGFLAGS = -g -fno-omit-frame-pointer
ASANFLAGS = -O2 -fsanitize=address
RELEASEFLAGS = -O3 -flto -DNDEBUG -march=native -mtune=native -fprofile-instr-use=default.profdata
#PGOFLAGS = -fprofile-instr-generate
#PGOFLAGS = -fprofile-instr-use


# build: $(WARNFLAGS) $(RELEASEFLAGS)
# asan: $(WARNFLAGS) $(DEBUGFLAGS) $(ASANFLAGS)
# debug: $(WARNFLAGS) $(DEBUGFLAGS)
CFLAGS = -I$(INCDIR) $(WARNFLAGS) $(DEBUGFLAGS) $(RELEASEFLAGS) -fopenmp
OPTFLAGS = -mllvm -unroll-count=8
LDFLAGS = -rdynamic -flto

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
TARGET = collatz

all: $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(TARGET)