#ifndef LIBIO_H
#define LIBIO_H

#include <stdio.h>

FILE *io_file_create(const char *file_name);
int io_file_read(const char *file_name, char **buf);
unsigned long io_file_len(FILE *fp);

#endif
