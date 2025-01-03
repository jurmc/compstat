#include <zlib.h>
#include <stdio.h>
#include <assert.h>

#define IN_PATH "/media/sf_amj018/Downloads/book-refdoc-API.pdf"
#define OUT_PATH "/media/sf_amj018/Downloads/book-refdoc-API.pdf.zlib"

#define CHUNK 16384

int main() {
    size_t all_read = 0;
    size_t all_write = 0;

    printf("Hello, Deflate!\n");

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

    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        printf("Error initalizing z_steam for deflation\n");
        fclose(in_fd);
        fclose(out_fd);
        return ret;
    }

    do {
        strm.avail_in = fread(in, 1, CHUNK, in_fd);
        if (ferror(in_fd)) {
            (void)deflateEnd(&strm);
            fclose(in_fd);
            fclose(out_fd);
            printf("Error reading file\n");
            return 1;
        }
        all_read += strm.avail_in;
        flush = feof(in_fd) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);
            have = CHUNK - strm.avail_out;
            all_write += have;
            if (fwrite(out, 1, have, out_fd) != have || ferror(stdout)) {
                (void)deflateEnd(&strm);
                fclose(in_fd);
                fclose(out_fd);
                printf("Error writing file\n");
                return 1;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);

    deflateEnd(&strm);
    fclose(in_fd);
    fclose(out_fd);

    printf("Read: %ld\n", all_read);
    printf("Written: %ld\n", all_write);

    return 0;

}
