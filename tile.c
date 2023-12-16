#include <stdio.h>
#include <stdlib.h>
#include <vips/vips.h>

#include "tile.h"

Tile *init_Tile(const char *path, int x_pos, int y_pos, VipsImage *vips_image)
{
    if (path == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Path!\n");
        exit(EXIT_FAILURE);
    }

    if (vips_image == NULL)
    {
        fprintf(stderr, "Error: Received NULL as VipsImage!\n");
        exit(EXIT_FAILURE);
    }

    Tile *tile = (Tile *)calloc(1, sizeof(Tile));
    if (tile == NULL)
    {
        fprintf(stderr, "Error: Can't allocate memory for Tile!\n");
        exit(EXIT_FAILURE);
    }
    tile->path = strdup(path);
    if (tile->path == NULL)
    {
        fprintf(stderr, "Error: Can't allocate memory for path in Tile!\n");
        exit(EXIT_FAILURE);
    }
    tile->x_pos = x_pos;
    tile->y_pos = y_pos;
    tile->vips_image = vips_image;
    return tile;
}

void cleanup_Tile(Tile *tile)
{
    if (tile == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Tile!\n");
        exit(EXIT_FAILURE);
    }

    free(tile->path);
    g_object_unref(tile->vips_image);
}

void display_Tile(const Tile *tile)
{
    if (tile == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Tile!\n");
        exit(EXIT_FAILURE);
    }

    printf("   ↪path: %s\n", tile->path);
    printf("   ↪position: (%d, %d)\n", tile->x_pos, tile->y_pos);
    printf("   ↪dimensions: %d x %d\n", vips_image_get_width(tile->vips_image), vips_image_get_height(tile->vips_image));
}
