#include <stdio.h>
#include <stdlib.h>

#include "tile.h"
#include "image.h"

Image *init_Image(const char *id)
{
    if (id == NULL)
    {
        fprintf(stderr, "Error: Received NULL as ID!\n");
        exit(EXIT_FAILURE);
    }

    Image *image = (Image *)calloc(1, sizeof(Image));
    if (image == NULL)
    {
        fprintf(stderr, "Error: Can't allocate memory for Image!\n");
        exit(EXIT_FAILURE);
    }
    image->id = strdup(id);
    if (image->id == NULL)
    {
        fprintf(stderr, "Error: Can't allocate memory for id in Image!\n");
        exit(EXIT_FAILURE);
    }
    image->num_tiles = 0;
    image->capacity = 10;
    image->tiles = (Tile *)calloc(image->capacity, sizeof(Tile));
    if (image->tiles == NULL)
    {
        fprintf(stderr, "Error: Can't allocate memory for tiles in Image!\n");
        exit(EXIT_FAILURE);
    }
    return image;
}

void add_Tile_to_Image(Image *image, const Tile *tile)
{
    if (image == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Image!\n");
        exit(EXIT_FAILURE);
    }

    if (tile == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Tile!\n");
        exit(EXIT_FAILURE);
    }

    if (image->num_tiles == image->capacity)
    {
        image->capacity *= 2;
        Tile *temp = (Tile *)realloc(image->tiles, image->capacity * sizeof(Tile));
        if (temp == NULL)
        {
            fprintf(stderr, "Error: Can't reallocate memory for tiles in Image!\n");
            exit(EXIT_FAILURE);
        }
        image->tiles = temp;
    }
    image->tiles[image->num_tiles] = *tile;
    image->num_tiles += 1;
}

void cleanup_Image(Image *image)
{
    if (image == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Image!\n");
        exit(EXIT_FAILURE);
    }

    free(image->id);
    for (size_t i = 0; i < image->num_tiles; i++)
    {
        cleanup_Tile(&image->tiles[i]);
    }
    free(image->tiles);
}

void display_Image(const Image *image)
{
    if (image == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Image!\n");
        exit(EXIT_FAILURE);
    }

    printf(" ↪id: %s\n", image->id);
    printf(" ↪dimensions: %d x %d\n", image->width, image->height);
    printf(" ↪grid: %d x %d\n", image->grid_x, image->grid_y);
    printf(" ↪tiles:\n");
    for (size_t i = 0; i < image->num_tiles; i++)
    {
        display_Tile(&(image->tiles[i]));
    }
}

int compare_tiles(const void *a, const void *b)
{
    if (a == NULL || b == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Tile!\n");
        exit(EXIT_FAILURE);
    }

    const Tile *tile_a = (const Tile *)a;
    const Tile *tile_b = (const Tile *)b;

    if (tile_a->y_pos != tile_b->y_pos)
    {
        return tile_a->y_pos - tile_b->y_pos;
    }
    else
    {
        return tile_a->x_pos - tile_b->x_pos;
    }
}

void sort_tiles(Image *image)
{
    if (image == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Image!\n");
        exit(EXIT_FAILURE);
    }

    qsort(image->tiles, image->num_tiles, sizeof(Tile), compare_tiles);
}

void set_dimensions(Image *image)
{
    if (image == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Image!\n");
        exit(EXIT_FAILURE);
    }

    sort_tiles(image);
    Tile *last_tile = &(image->tiles[image->num_tiles - 1]);
    image->width = last_tile->x_pos + vips_image_get_width(last_tile->vips_image);
    image->height = last_tile->y_pos + vips_image_get_height(last_tile->vips_image);
}

int get_common_number(const int *array, size_t array_size)
{
    if (array == NULL || array_size == 0)
    {
        fprintf(stderr, "Error: Invalid input parameters!\n");
        exit(EXIT_FAILURE);
    }

    int common_number = 0;

    for (size_t i = 0; i < array_size; i++)
    {
        if (array[i] == 0)
        {
            continue;
        }

        if (common_number != 0 && array[i] != common_number)
        {
            fprintf(stderr, "Error: Could not determine common number (multiple non-zero values for %zu)!\n", i);
            exit(EXIT_FAILURE);
        }

        common_number = array[i];
    }

    if (common_number == 0)
    {
        fprintf(stderr, "Error: Could not determine common number (all-zero array)!\n");
        exit(EXIT_FAILURE);
    }

    return common_number;
}

void set_grid(Image *image)
{
    if (image == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Image!\n");
        exit(EXIT_FAILURE);
    }

    int *grid_x = (int *)calloc(image->width, sizeof(int));
    if (grid_x == NULL)
    {
        fprintf(stderr, "Error: Can't allocate memory for grid_x!\n");
        exit(EXIT_FAILURE);
    }
    int *grid_y = (int *)calloc(image->height, sizeof(int));
    if (grid_y == NULL)
    {
        fprintf(stderr, "Error: Can't allocate memory for grid_y!\n");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < image->num_tiles; i++)
    {
        grid_x[image->tiles[i].x_pos]++;
        grid_y[image->tiles[i].y_pos]++;
    }
    image->grid_x = get_common_number(grid_y, image->height);
    image->grid_y = get_common_number(grid_x, image->width);
    free(grid_x);
    free(grid_y);
}
