#include <zlib.h>
#include <stdio.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#define IN_PATH "/media/sf_amj018/Downloads/book-refdoc-API.pdf.zlib"

#define CHUNK 16384

int compute_inflated_size_from_file() {
    printf("compute_inflated_size_from_file\n");
    size_t all_compressed = 0;
    size_t all_uncompressed = 0;

    int in_fd = open(IN_PATH, 0);
    if (in_fd < 0) {
        printf("Error opening file: %s\n", IN_PATH);
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
        close(in_fd);
        return ret;
    }

    do {
        size_t bytes_read = read(in_fd, in, CHUNK);
        if (bytes_read < 0) {
            (void)inflateEnd(&strm);
            close(in_fd);
            return Z_ERRNO;
        }
        strm.avail_in = bytes_read;
        all_compressed += strm.avail_in;

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
                    close(in_fd);
                    printf("Error inflating data from file %s\n", IN_PATH);
                    return ret;
            }

            have = CHUNK - strm.avail_out;
            all_uncompressed += have;
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    (void)inflateEnd(&strm);
    close(in_fd);

    printf("Compressed: %zu\n", all_compressed);
    printf("Uncompress: %zu\n", all_uncompressed);

    return Z_OK;
}

int compute_inflated_size_from_memory() {
    printf("compute_inflated_size_from_memory\n");

    return Z_OK;
}

int main() {
    compute_inflated_size_from_file();
    compute_inflated_size_from_memory();
    return 0;
}
