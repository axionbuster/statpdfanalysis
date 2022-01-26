#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libdeflate.h>

typedef enum { PDFENTER, PDFEXIT } ParseState;

#define STRPRE(s1, lits) (!strncmp(s1, lits, sizeof(lits) - 1))

static void *xrealloc(void *p, size_t bytes) {
  void *q = realloc(p, bytes);
  if (!q && p) {
    fprintf(stderr, "Tried to ask the OS for more memory; failed.\n");
    abort();
  }
  return q;
}

int main(void) {
  char *line = NULL;
  size_t cap = 0;
  size_t n_lines = 0;
  ssize_t read;
  ParseState parse = PDFEXIT;
  uint8_t *raw_buf = NULL, *decomp_buf = NULL;
  struct libdeflate_decompressor *decompressor =
      libdeflate_alloc_decompressor();
  if (!decompressor) {
    fprintf(stderr, "The decompressor API is unavailable at this moment.\n"
                    "Please try again!\n");
    abort();
  }

  while ((read = getline(&line, &cap, stdin)) != EOF) {
    n_lines++;

    bool ascii = true;
    for (ssize_t i = 0; i < read; i++) {
      ascii = (ascii && line[i] && isascii(line[i])) ? ascii : false;
    }

    if (ascii) {
      int type, version;
      size_t length;
      if (sscanf(line, "%d %d obj", &type, &version) == 2)
        printf("Found obj decl., t = %d, v = %d\n", type, version);
      else if (STRPRE(line, "<<")) {
        parse = PDFENTER;
        printf("<< found, line %zu\n", n_lines);
      } else if (STRPRE(line, ">>")) {
        parse = PDFEXIT;
        printf(">> found, line %zu\n", n_lines);
      } else if (sscanf(line, "/Length %zu", &length) == 1) {
        printf("Found /Length (%zu), line %zu\n", length, n_lines);
        if (parse == PDFENTER) {
          printf("[/Length] Allocate raw_buf\n");
          raw_buf = xrealloc(raw_buf, length);
        }
      } else if (STRPRE(line, "/Filter /FlateDecode")) {
        printf("Found /Filter, line %zu\n", n_lines);
        if (parse == PDFENTER) {
          printf("[/Filter /FlateDecode] Read raw_buf\n");
          size_t bread = fread(raw_buf, sizeof(uint8_t), length, stdin);
          if (bread == length) {
            printf("[/Filter /FlateDecode] Read Success\n");
            size_t avail = (size_t)length * 4, real_decomp_len;
            decomp_buf = xrealloc(decomp_buf, avail);
            enum libdeflate_result res = libdeflate_deflate_decompress(
                decompressor, raw_buf, length, decomp_buf, avail,
                &real_decomp_len);
            switch (res) {
            case LIBDEFLATE_SUCCESS:
              printf("[/Filter /FlateDecode] Decompression success\n");
              break;
            case LIBDEFLATE_BAD_DATA:
              printf("[/Filter /FlateDecode] Your data sucks :(\n");
              break;
            default:
              printf("[/F /F] FAIL DECOMP %d\n", (int)res);
              abort();
            }
          } else {
            printf("[/Filter /FlateDecode] Read failure\n");
            abort();
          }
        }
      } else if (STRPRE(line, "stream"))
        printf("Found \"stream\", line %zu\n", n_lines);
      else if (STRPRE(line, "endstream"))
        printf("Found \"endstream\", line %zu\n", n_lines);
      else if (STRPRE(line, "endobj"))
        printf("Found \"endobj\", line %zu\n", n_lines);
    }
  }

  free(raw_buf);
  free(decomp_buf);
  libdeflate_free_decompressor(decompressor);
  free(line);
}
