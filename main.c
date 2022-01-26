#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  WAIT_FOR_LENGTH,
  WAIT_FOR_FILTER
} ParseState;

int main(void) {
  char *line = NULL;
  size_t cap = 0;
  size_t n_lines = 0;
  ssize_t read;
  ParseState parse = WAIT_FOR_LENGTH;

  while ((read = getline(&line, &cap, stdin)) != EOF) {
    n_lines++;

    bool ascii = true;
    for (ssize_t i = 0; i < read; i++) {
      ascii = (ascii && isascii(line[i])) ? ascii : false;
    }

    if (ascii) {
      int type, version;
      if (sscanf(line, "%d %d obj", &type, &version) == 2)
        printf("Found obj decl., t = %d, v = %d\n", type, version);
      else if (!strncmp(line, "/Length", sizeof("/Length") - 1))
        printf("Found /Length, line %zu\n", n_lines);
      else if (!strncmp(line, "/Filter /FlateDecode", sizeof("/Filter /FlateDecode") - 1))
        printf("Found /Filter, line %zu\n", n_lines);
      else if (!strncmp(line, "stream", sizeof("stream") - 1))
        printf("Found \"stream\", line %zu\n", n_lines);
      else if (!strncmp(line, "endstream", sizeof ("endstream") - 1))
        printf("Found \"endstream\", line %zu\n", n_lines);
      else if (!strncmp(line, "endobj", sizeof ("endobj") - 1))
        printf("Found \"endobj\", line %zu\n", n_lines);
    }
  }

  free(line);
}
