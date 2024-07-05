
#include <execinfo.h>
#include <stdlib.h>

#include "debug.h"

void print_trace() {
  printf("Attempting to get backtrace.\n");
  void *array[32];
  char **strings;
  int size, i;

  size = backtrace(array, 32);
  strings = backtrace_symbols(array, size);
  if (strings != NULL) {
    printf("Obtained %d stack frames.\n", size);
    for (i = 0; i < size; i++)
      printf("%s\n", strings[i]);
  }

  free(strings);
}

