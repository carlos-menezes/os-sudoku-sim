#include <stdlib.h>
#include <string.h>

#include "libgrid.h"
#include "libio.h"
#include "libconfigserver.h"

int load_grids(server_t** s) {
    char* buf;
    if(io_file_read(GRIDS_FILE, &buf) == -1) {
        return -1;
    };

    char* line = strtok(strdup(buf), "\n");
    int i = 0;
    while (line) {
        sscanf(line, "%u %s %s", &((*s)->grids[i].difficulty), (*s)->grids[i].problem, (*s)->grids[i].solution);
        i++;
        line  = strtok(NULL, "\n");
    }

    free(line);
    return 0;
}