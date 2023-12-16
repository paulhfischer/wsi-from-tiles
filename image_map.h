#include "image.h"

#ifndef IMAGE_MAP_H
#define IMAGE_MAP_H

typedef struct
{
    Image *images;
    size_t num_images;
    size_t capacity;
} ImageMap;

ImageMap *init_ImageMap();
void add_Image_to_ImageMap(ImageMap *image_map, const Image *image);
void cleanup_ImageMap(ImageMap *image_map);
void display_ImageMap(const ImageMap *image_map);

Image *get_or_create_Image(ImageMap *image_map, const char *id);

#endif
