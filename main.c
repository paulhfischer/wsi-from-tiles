#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <regex.h>
#include <vips/vips.h>

#include "image_map.h"
#include "image.h"
#include "tile.h"

#define FILE_REGEX "^([a-f0-9-]+) @\\(([0-9]+)\\|([0-9]+)\\)\\.jpg$"

int DEBUG = 0;

ImageMap *loadImages(const char *input_dir)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(input_dir);
    if (dir == NULL)
    {
        fprintf(stderr, "Error: Can't open directory!\n");
        exit(EXIT_FAILURE);
    }

    regex_t regex;
    if (regcomp(&regex, FILE_REGEX, REG_EXTENDED) != 0)
    {
        fprintf(stderr, "Error: Can't compile regular expression!\n");
        exit(EXIT_FAILURE);
    }

    ImageMap *image_map = init_ImageMap();

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_REG)
        {
            continue;
        }

        char file_name[256];
        strcpy(file_name, entry->d_name);

        if (strcmp(file_name, ".DS_Store") == 0)
        {
            continue;
        }

        if (DEBUG)
        {
            printf("⇒ loading %s\n", file_name);
        }

        regmatch_t matches[4];
        if (regexec(&regex, file_name, 4, matches, 0) == 0)
        {
            char image_id[64];
            strncpy(image_id, file_name + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
            image_id[matches[1].rm_eo - matches[1].rm_so] = '\0';

            char path[256];
            snprintf(path, sizeof(path), "%s/%s", input_dir, file_name);

            int x_pos;
            sscanf(file_name + matches[2].rm_so, "%d", &x_pos);

            int y_pos;
            sscanf(file_name + matches[3].rm_so, "%d", &y_pos);

            VipsImage *vips_image = vips_image_new_from_file(path, NULL);
            if (vips_image == NULL)
            {
                fprintf(stderr, "Error: Failed to load image!\n");
                exit(EXIT_FAILURE);
            }

            Image *image = get_or_create_Image(image_map, image_id);

            const Tile *tile = init_Tile(path, x_pos, y_pos, vips_image);
            add_Tile_to_Image(image, tile);
        }
        else
        {
            fprintf(stderr, "Error: Filename does not match the pattern: '%s'!\n", entry->d_name);
            exit(EXIT_FAILURE);
        }
    }

    closedir(dir);

    regfree(&regex);

    for (size_t i = 0; i < image_map->num_images; i++)
    {
        Image *image = &(image_map->images[i]);

        if (DEBUG)
        {
            printf("⇒ postprocessing %s\n", image->id);
        }

        set_dimensions(image);
        set_grid(image);

        if (image->grid_x * image->grid_y != image->num_tiles)
        {
            fprintf(stderr, "Error: Missing tiles!\n");
            exit(EXIT_FAILURE);
        }

        sort_tiles(image);
        for (size_t j = 1; j < image->num_tiles; j++)
        {
            const Tile *previus_tile = &(image->tiles[j - 1]);
            const Tile *current_tile = &(image->tiles[j]);
            if (previus_tile->x_pos == current_tile->x_pos && previus_tile->y_pos + vips_image_get_height(previus_tile->vips_image) != current_tile->y_pos)
            {
                fprintf(stderr, "Error: Inconsistent grid!\n");
                exit(EXIT_FAILURE);
            }
            if (previus_tile->y_pos == current_tile->y_pos && previus_tile->x_pos + vips_image_get_width(previus_tile->vips_image) != current_tile->x_pos)
            {
                fprintf(stderr, "Error: Inconsistent grid!\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    for (size_t i = 0; i < image_map->num_images; i++)
    {
        const Image *image = &(image_map->images[i]);

        printf("⇒ loaded %s\n", image->id);
        if (DEBUG)
        {
            display_Image(image);
        }
    }

    return image_map;
}

void createImages(const ImageMap *image_map, const char *output_dir, int save_full, int pyramid, int save_compressed, int compressed_size)
{
    for (size_t i = 0; i < image_map->num_images; i++)
    {
        printf("\n");

        Image *image = &(image_map->images[i]);

        VipsImage **tiles = g_malloc_n(image->num_tiles, sizeof(VipsImage *));
        VipsImage *joined;
        VipsImage *full_out;

        for (size_t j = 0; j < image->num_tiles; j++)
        {
            tiles[j] = image->tiles[j].vips_image;
        }

        if (vips_arrayjoin(tiles, &joined, image->num_tiles, "across", image->grid_x, NULL) != 0)
        {
            fprintf(stderr, "Error (arrayjoin): %s", vips_error_buffer());
            exit(EXIT_FAILURE);
        }
        printf("⇒ joined %s\n", image->id);

        if (vips_crop(joined, &full_out, 0, 0, image->width, image->height, NULL) != 0)
        {
            fprintf(stderr, "Error (crop): %s", vips_error_buffer());
            exit(EXIT_FAILURE);
        }
        printf("⇒ cropped %s\n", image->id);

        if (save_full)
        {
            char full_output_path[256];
            snprintf(full_output_path, sizeof(full_output_path), "%s/%s.tiff", output_dir, image->id);
            if (pyramid)
            {
                if (vips_tiffsave(full_out, full_output_path, "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE, "tile", TRUE, "pyramid", TRUE, "region_shrink", VIPS_REGION_SHRINK_MODE, "bigtiff", TRUE, "resunit", VIPS_FOREIGN_TIFF_RESUNIT_INCH, "xres", 300 / 25.4, "yres", 300 / 25.4, "Q", 100, NULL) != 0)
                {
                    fprintf(stderr, "Error (tiffsave): %s", vips_error_buffer());
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                if (vips_tiffsave(full_out, full_output_path, "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE, "bigtiff", TRUE, "resunit", VIPS_FOREIGN_TIFF_RESUNIT_INCH, "xres", 300 / 25.4, "yres", 300 / 25.4, "Q", 100, NULL) != 0)
                {
                    fprintf(stderr, "Error (tiffsave): %s", vips_error_buffer());
                    exit(EXIT_FAILURE);
                }
            }
            printf("⇒ saved (full) %s\n", image->id);
        }

        if (save_compressed)
        {
            VipsImage *compressed_out;

            if (vips_thumbnail_image(full_out, &compressed_out, compressed_size, "height", compressed_size, NULL))
            {
                fprintf(stderr, "Error (thumbnail_image): %s", vips_error_buffer());
                exit(EXIT_FAILURE);
            }
            printf("⇒ compressed %s\n", image->id);

            char compressed_output_path[256];
            snprintf(compressed_output_path, sizeof(compressed_output_path), "%s/%s (x%d).jpg", output_dir, image->id, compressed_size);
            if (vips_jpegsave(compressed_out, compressed_output_path, "Q", 100, NULL) != 0)
            {
                fprintf(stderr, "Error (jpegsave): %s", vips_error_buffer());
                exit(EXIT_FAILURE);
            }
            printf("⇒ saved (compressed) %s\n", image->id);

            g_object_unref(compressed_out);
        }

        g_free(tiles);
        g_object_unref(joined);
        g_object_unref(full_out);
    }
}

void parse_args(int argc, char *argv[], char **input_dir, char **output_dir, int *save_full, int *pyramid, int *save_compressed, int *compressed_size)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <input_dir> <output_dir> [--debug] [--save-full] [--pyramid] [--save-compressed] [--compressed_size=...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    *input_dir = argv[1];
    *output_dir = argv[2];
    *save_full = 0;
    *pyramid = 0;
    *save_compressed = 0;
    *compressed_size = 10000;

    for (int i = 3; i < argc; ++i)
    {
        if (strcmp(argv[i], "--debug") == 0)
        {
            DEBUG = 1;
            continue;
        }

        if (strcmp(argv[i], "--save-full") == 0)
        {
            *save_full = 1;
            continue;
        }

        if (strcmp(argv[i], "--pyramid") == 0)
        {
            *pyramid = 1;
            continue;
        }

        if (strcmp(argv[i], "--save-compressed") == 0)
        {
            *save_compressed = 1;
            continue;
        }

        if (strncmp(argv[i], "--compressed-size=", 18) == 0)
        {
            sscanf(argv[i] + 18, "%d", compressed_size);
            continue;
        }

        fprintf(stderr, "Unknown option: %s\n", argv[i]);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    char *input_dir;
    char *output_dir;
    int save_full;
    int pyramid;
    int save_compressed;
    int compressed_size;

    parse_args(argc, argv, &input_dir, &output_dir, &save_full, &pyramid, &save_compressed, &compressed_size);

    if (VIPS_INIT("image-processing") != 0)
    {
        vips_error_exit("Error: Can't start VIPS!");
    }

    ImageMap *image_map = loadImages(input_dir);

    if (save_full || save_compressed)
    {
        createImages(image_map, output_dir, save_full, pyramid, save_compressed, compressed_size);
    }

    cleanup_ImageMap(image_map);
    free(image_map);

    vips_shutdown();

    return EXIT_SUCCESS;
}
