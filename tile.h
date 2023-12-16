#include <vips/vips.h>

#ifndef TILE_H
#define TILE_H

typedef struct
{
    char *path;
    int x_pos;
    int y_pos;
    VipsImage *vips_image;
} Tile;

Tile *init_Tile(const char *path, int x_pos, int y_pos, VipsImage *vips_image);
void cleanup_Tile(Tile *tile);
void display_Tile(const Tile *tile);

#endif
