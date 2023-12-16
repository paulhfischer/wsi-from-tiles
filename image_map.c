#include <stdio.h>
#include <stdlib.h>

#include "image.h"
#include "image_map.h"

ImageMap *init_ImageMap()
{
    ImageMap *image_map = (ImageMap *)calloc(1, sizeof(ImageMap));
    if (image_map == NULL)
    {
        fprintf(stderr, "Error: Can't allocate memory for ImageMap!\n");
        exit(EXIT_FAILURE);
    }
    image_map->capacity = 5;
    image_map->images = (Image *)calloc(image_map->capacity, sizeof(Image));
    if (image_map->images == NULL)
    {
        fprintf(stderr, "Error: Can't allocate memory for images in ImageMap!\n");
        exit(EXIT_FAILURE);
    }
    return image_map;
}

void add_Image_to_ImageMap(ImageMap *image_map, const Image *image)
{
    if (image_map == NULL)
    {
        fprintf(stderr, "Error: Received NULL as ImageMap!\n");
        exit(EXIT_FAILURE);
    }

    if (image == NULL)
    {
        fprintf(stderr, "Error: Received NULL as Image!\n");
        exit(EXIT_FAILURE);
    }

    if (image_map->num_images == image_map->capacity)
    {
        image_map->capacity *= 2;
        Image *temp = (Image *)realloc(image_map->images, image_map->capacity * sizeof(Image));
        if (temp == NULL)
        {
            fprintf(stderr, "Error: Can't reallocate memory for images in ImageMap!\n");
            exit(EXIT_FAILURE);
        }
        image_map->images = temp;
    }
    image_map->images[image_map->num_images] = *image;
    image_map->num_images++;
}

void cleanup_ImageMap(ImageMap *image_map)
{
    if (image_map == NULL)
    {
        fprintf(stderr, "Error: Received NULL as ImageMap!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < image_map->num_images; i++)
    {
        cleanup_Image(&image_map->images[i]);
    }
    free(image_map->images);
}

void display_ImageMap(const ImageMap *image_map)
{
    if (image_map == NULL)
    {
        fprintf(stderr, "Error: Received NULL as ImageMap!\n");
        exit(EXIT_FAILURE);
    }

    printf("images:\n");
    for (size_t i = 0; i < image_map->num_images; i++)
    {
        display_Image(&(image_map->images[i]));
    }
}

Image *get_or_create_Image(ImageMap *image_map, const char *id)
{
    if (image_map == NULL)
    {
        fprintf(stderr, "Error: Received NULL as ImageMap!\n");
        exit(EXIT_FAILURE);
    }

    if (id == NULL)
    {
        fprintf(stderr, "Error: Received NULL as ID!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < image_map->num_images; i++)
    {
        if (strncmp(image_map->images[i].id, id, strlen(image_map->images[i].id)) == 0)
        {
            return &(image_map->images[i]);
        }
    }

    const Image *image = init_Image(id);
    add_Image_to_ImageMap(image_map, image);
    return get_or_create_Image(image_map, id);
}
