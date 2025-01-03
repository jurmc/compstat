#include <zlib.h>
#include <stdio.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#define IN_PATH "/media/sf_amj018/Downloads/book-refdoc-API.pdf.zlib"
#define OUT_PATH "/media/sf_amj018/Downloads/book-refdoc-API.pdf.inflated"

#define CHUNK 16384

int main() {
    size_t all_read = 0;
    size_t all_write = 0;

    printf("Hello, Inflate: w!\n");

    FILE *in_fd = fopen(IN_PATH, "r");
    if (in_fd == NULL) {
        printf("Error opening file: %s\n", IN_PATH);
        return 1;
    }

    FILE *out_fd = fopen(OUT_PATH, "w");
    if (out_fd == NULL) {
        printf("Error opening file: %s\n", OUT_PATH);
        fclose(in_fd);
        return 1;
    }

    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    ret = inflateInit(&strm);
    if (ret != Z_OK) {
        fclose(in_fd);
        fclose(out_fd);
        return ret;
    }

    do {
        strm.avail_in = fread(in, 1, CHUNK, in_fd);
        all_read += strm.avail_in;

        if (ferror(in_fd)) {
            (void)inflateEnd(&strm);
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;

            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    fclose(in_fd);
                    fclose(out_fd);
                    printf("Error inflating data from file %s\n", IN_PATH);
                    return ret;
            }

            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, out_fd) != have || ferror(out_fd)) {
                (void)inflateEnd(&strm);
                printf("Error writing to file: %s\n", OUT_PATH);
                break;
            }
            all_write += have;
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    (void)inflateEnd(&strm);
    fclose(in_fd);
    fclose(out_fd);

    printf("Read: %i\n", all_read);
    printf("Written: %i\n", all_write);
}
