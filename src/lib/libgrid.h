#ifndef LIBGRID_H
#define LIBGRID_H

#define GRIDS_FILE "grids"
#define MAX_GRIDS 9
#define GRID_SIZE 81

typedef enum { EASY, MEDIUM, HARD } DIFFICULTY;
typedef struct {
	DIFFICULTY difficulty;
	char problem[GRID_SIZE];
	char solution[GRID_SIZE];
} grid_t ;


#endif
