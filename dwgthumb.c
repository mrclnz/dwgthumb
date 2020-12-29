#define _GNU_SOURCE
#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define SENTINEL_SIZE 16
static const char preview_sentinel[SENTINEL_SIZE] = {
    0x1F, 0x25, 0x6D, 0x07, 0xD4, 0x36, 0x28, 0x28,
    0x9D, 0x57, 0xCA, 0x3F, 0x9D, 0x44, 0x10, 0x2B };

static uint32_t rulong(char *s)
{
    uint8_t *m = (uint8_t*)s;
    uint32_t r = *m++;
    r |= (*m++) << 8;
    r |= (*m++) << 16;
    r |= (*m++) << 24;
    return r;
}

int write_preview_png(const char *pngname, const char *png_data,
        unsigned png_size)
{
    int fout = open(pngname, O_WRONLY|O_CREAT|O_TRUNC,
            S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (fout < 0) {
        perror("Opening output");
        return 1;
    }

    if (write(fout, png_data, png_size) != png_size) {
        perror("Writing image data");
        close(fout);
        return 1;
    }
    close(fout);
    return 0;
}


int write_preview_bmp(const char *bmpname, const char *dib_data,
        unsigned dib_size)
{
    int fout = open(bmpname, O_WRONLY|O_CREAT|O_TRUNC,
            S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (fout < 0) {
        perror("Opening output");
        return 1;
    }

    uint8_t bmp_header[14];
    const unsigned header_size = 14;
    const unsigned filesize = dib_size + header_size;
    const unsigned offset = header_size + 40 + 4 * 256;
    bmp_header[0] = 'B';
    bmp_header[1] = 'M';
    bmp_header[2] = filesize & 0xFF;
    bmp_header[3] = (filesize >> 8) & 0xFF;
    bmp_header[4] = (filesize >> 16) & 0xFF;
    bmp_header[5] = (filesize >> 24) & 0xFF;
    bmp_header[6] = 0;
    bmp_header[7] = 0;
    bmp_header[8] = 0;
    bmp_header[9] = 0;
    bmp_header[10] = offset & 0xFF;
    bmp_header[11] = (offset >> 8) & 0xFF;
    bmp_header[12] = (offset >> 16) & 0xFF;
    bmp_header[13] = (offset >> 24) & 0xFF;
    if (write(fout, bmp_header, header_size) != header_size) {
        perror("Writing header");
        close(fout);
        return 1;
    }
    if (write(fout, dib_data, dib_size) != dib_size) {
        perror("Writing image data");
        close(fout);
        return 1;
    }
    close(fout);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s dwgfile bmpfile\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Opening file");
        return 1;
    }

    struct stat stat_buf;
    if (fstat(fd, &stat_buf)) {
        perror("Statting file");
        return 1;
    }
    off_t filesize = stat_buf.st_size;
    char *dwgfile = mmap(NULL, filesize, PROT_READ, MAP_SHARED,
            fd, 0);
    if (dwgfile == MAP_FAILED) {
        perror("Mapping file");
        return 1;
    }

    char *sentinel = memmem(dwgfile, filesize,
            preview_sentinel, SENTINEL_SIZE);
    if (sentinel == NULL) {
        fputs("Preview sentinel not found\n", stderr);
    } else {
        unsigned image_count = sentinel[4+SENTINEL_SIZE];
        char *p = sentinel+5+SENTINEL_SIZE;
        for (unsigned i = 0; i < image_count; ++i) {
            char image_code = *p;
            unsigned image_start = rulong(p+1);
            unsigned image_size = rulong(p+5);
            switch (image_code) {
            case 1:             /* Header, all zero */
                break;
            case 2:             /* BMP preview */
                write_preview_bmp(argv[2],
                        dwgfile+image_start, image_size);
                break;
            case 6:             /* PNG preview */
                write_preview_png(argv[2],
                        dwgfile+image_start, image_size);
                break;
            default:
                fprintf(stderr, "Unknown image type %u at %#x size %#x\n",
                        image_code, image_start, image_size);
                break;
            }
            p += 9;             /* Next chunk */
        }
    }
    munmap(dwgfile, filesize);
    close(fd);
    return 0;
}
