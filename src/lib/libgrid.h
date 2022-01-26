#ifndef LIBGRID_H
#define LIBGRID_H

#define GRIDS_FILE "grids"
#define MAX_GRIDS 3
#define GRID_SIZE 81

typedef enum { EASY, MEDIUM, HARD } DIFFICULTY;
struct grid_t {
	DIFFICULTY difficulty;
	char problem[GRID_SIZE + 1];
	char solution[GRID_SIZE + 1];
};

void get_available_cells(char* problem, char* buf);

#endif
