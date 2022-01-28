#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "libio.h"

// Creates an empty file for writing.
// If a file with the same name already exists, its content is erased and the file is considered as a new empty file.
FILE *io_file_create(const char *file_name)
{
    FILE *fp;
    if (file_name == NULL || *file_name == '\0')
    {
        return NULL;
    }

    fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        return NULL;
    }

    return fp;
}

// Reads the contents of a file, and stores them in a buffer.
int io_file_read(const char *file_name, char **buf)
{
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        // FAILED TO OPEN FILE
        return -1;
    }

    unsigned long bufsize = io_file_len(fp);

    // Allocate memory for each character.
    *buf = calloc(bufsize, sizeof(char));
    if (buf == NULL)
    {
        return -1;
    }

    // `fread` can now be used to store the content of the file in `buf`.
    fread(*buf, sizeof(char), bufsize, fp);
    if (ferror(fp) != 0)
    {
        // FAILED TO READ FILE
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

// Returns the lenght of a file
unsigned long io_file_len(FILE *fp)
{
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        // FAILED TO SEEK_END
        return -1;
    }

    long size = ftell(fp);
    if (size == -1)
    {
        // FTELL RETURNED ERROR
        return -1;
    }


    if (fseek(fp, 0, SEEK_SET) != 0)
    {
        // FAILED TO SEEK_SET
        return -1;
    }

    return size;
}
