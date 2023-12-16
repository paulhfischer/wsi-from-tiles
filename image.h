#include "tile.h"

#ifndef IMAGE_H
#define IMAGE_H

typedef struct
{
    char *id;
    int width;
    int height;
    int grid_x;
    int grid_y;
    Tile *tiles;
    size_t num_tiles;
    size_t capacity;
} Image;

Image *init_Image(const char *id);
void add_Tile_to_Image(Image *image, const Tile *tile);
void cleanup_Image(Image *image);
void display_Image(const Image *image);

void sort_tiles(Image *image);
void set_dimensions(Image *image);
void set_grid(Image *image);

#endif
