#include <stdio.h>
#include "spng.h"
#include "png_reader.hpp"

static const char *color_type_str(uint8_t color_type)
{
    switch(color_type)
    {
        case SPNG_COLOR_TYPE_GRAYSCALE: return "grayscale";
        case SPNG_COLOR_TYPE_TRUECOLOR: return "truecolor";
        case SPNG_COLOR_TYPE_INDEXED: return "indexed color";
        case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA: return "grayscale with alpha";
        case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA: return "truecolor with alpha";
        default: return "(invalid)";
    }
}

static int has_alpha(uint8_t color_type)
{
    return color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA || color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
}

static void bgr_to_rgb(unsigned char *image, int width, int height)
{
    unsigned char *offset = image;
    for (int i = 0; i < width * height; i++) {
        unsigned char b = offset[0];
        unsigned char r = offset[2];
        offset[0] = r;
        offset[2] = b;
        offset += 4;
    }
}

static void premultiply_alpha(unsigned char *image, int width, int height)
{
    unsigned char *offset = image;
    for (int i = 0; i < width * height; i++) {
        double a = (double)offset[3] / 255.0;
        double r = (double)offset[0] * a / 255.0;
        double g = (double)offset[1] * a / 255.0;
        double b = (double)offset[2] * a / 255.0;
        offset[0] = (unsigned char)(r * 255.0);
        offset[1] = (unsigned char)(g * 255.0);
        offset[2] = (unsigned char)(b * 255.0);
        offset += 4;
    }
}

int PngReader::ReadFromFile(const char* file_name)
{
    FILE *png = fopen(file_name, "rb");
    if (!png) return 1;

    spng_ctx *ctx = spng_ctx_new(0);
    if (!ctx) return 1;

    int ret = 0;
    unsigned char *image = NULL;

    // ignore chunk crc's
    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

    // set memory usage limits
    size_t limit = 1024 * 1024 * 64;
    spng_set_chunk_limits(ctx, limit, limit);

    spng_set_png_file(ctx, png);

    struct spng_ihdr ihdr;
    ret = spng_get_ihdr(ctx, &ihdr);

    if(ret)
    {
        printf("spng_get_ihdr() error: %s\n", spng_strerror(ret));
        spng_ctx_free(ctx);
        return 1;
    }

    const char *color_name = color_type_str(ihdr.color_type);

    /*
    printf("width: %u\n"
        "height: %u\n"
        "bit depth: %u\n"
        "color type: %u - %s\n",
        ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.color_type, color_name);
    */

    size_t image_size, image_width;
    int fmt = SPNG_FMT_PNG;
    if(ihdr.color_type == SPNG_COLOR_TYPE_INDEXED) fmt = SPNG_FMT_RGB8;

    ret = spng_decoded_image_size(ctx, fmt, &image_size);
    if (ret)
    {
        spng_ctx_free(ctx);
        return 1;
    }

    image = (unsigned char *)malloc(image_size);

    ret = spng_decode_image(ctx, NULL, 0, fmt, SPNG_DECODE_PROGRESSIVE);

    if(ret)
    {
        printf("progressive spng_decode_image() error: %s\n", spng_strerror(ret));
        spng_ctx_free(ctx);
        free(image);
        return 1;
    }

    image_width = image_size / ihdr.height;

    struct spng_row_info row_info = {0};

    do
    {
        ret = spng_get_row_info(ctx, &row_info);
        if(ret) break;

        ret = spng_decode_row(ctx, image + row_info.row_num * image_width, image_width);
    }
    while(!ret);


    if(ret != SPNG_EOI)
    {
        printf("progressive decode error: %s\n", spng_strerror(ret));

        if(ihdr.interlace_method)
            printf("last pass: %d, scanline: %u\n", row_info.pass, row_info.scanline_idx);
        else
            printf("last row: %u\n", row_info.row_num);
    }

    // We should convert the raw data to the libui format
    bgr_to_rgb(image, ihdr.width, ihdr.height);
    premultiply_alpha(image, ihdr.width, ihdr.height);

    m_data = image;
    m_width = ihdr.width;
    m_height = ihdr.height;
    m_has_alpha = has_alpha(ihdr.color_type);

    spng_ctx_free(ctx);
    return 0;
}
