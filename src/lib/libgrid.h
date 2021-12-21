#ifndef LIBGRID_H
#define LIBGRID_H

#define GRIDS_FILE "grids"
#define MAX_GRIDS 3
#define GRID_SIZE 81

typedef enum { EASY, MEDIUM, HARD } DIFFICULTY;
struct grid_t {
	DIFFICULTY difficulty;
	char problem[GRID_SIZE];
	char solution[GRID_SIZE];
};


#endif
