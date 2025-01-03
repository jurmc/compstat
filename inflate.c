#include <zlib.h>
#include <stdio.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

//#define CHUNK 16384
#define CHUNK 128

int compute_inflated_size_from_file(char *path) {
    size_t all_compressed = 0;
    size_t all_uncompressed = 0;

    int in_fd = open(path, 0);
    if (in_fd < 0) {
        printf("Error opening file: %s\n", path);
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
            inflateEnd(&strm);
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
                    inflateEnd(&strm);
                    close(in_fd);
                    printf("Error inflating data from file %s\n", path);
                    return ret;
            }

            have = CHUNK - strm.avail_out;
            all_uncompressed += have;
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    close(in_fd);

    printf("compute_inflated_size_from_file: Compressed: %zu\n", all_compressed);
    printf("compute_inflated_size_from_file: Uncompress: %zu\n", all_uncompressed);

    return Z_OK;
}

// Retrun value:
// > 0: success, value is computed size of inflated data
// otherwise: error
size_t compute_inflated_size_from_memory(unsigned char *buf, size_t buf_size) {
    size_t uncompressed_size = 0;
    unsigned char *in_ptr = buf;

    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    ret = inflateInit(&strm);
    if (ret != Z_OK) {
        return 0;
    }

    int bytes_remaining = buf_size;
    do {
        if (bytes_remaining < CHUNK) {
            strm.avail_in = bytes_remaining;
        } else {
            strm.avail_in = CHUNK;
        }
        bytes_remaining -= strm.avail_in;

        if (strm.avail_in == 0)
            break;
        strm.next_in = in_ptr;

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
                    inflateEnd(&strm);
                    printf("Error inflating data\n");
                    return 0;
            }

            have = CHUNK - strm.avail_out;
            uncompressed_size += have;

        } while (strm.avail_out == 0);
        in_ptr += CHUNK;
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);

    return uncompressed_size;
}

size_t read_compressed_file(char *path, unsigned char **buf) {
    *buf = NULL;
    int in_fd = open(path, 0);
    if (in_fd < 0) {
        printf("Error opening file: %s\n", path);
        return 0;
    }
    struct stat statbuf;
    if (0 != fstat(in_fd, &statbuf)) {
        close(in_fd);
        return 0;
    }
    size_t file_size = statbuf.st_size;
    printf("File size: %zu\n", file_size);
    *buf = malloc(file_size);
    if (*buf == NULL) {
        close(in_fd);
        return 0;
    }
    size_t bytes_read = read(in_fd, *buf, file_size);
    if (bytes_read != file_size) {
        close(in_fd);
        free(buf);
        *buf = NULL;
        return 0;
    }
    close(in_fd);

    return file_size;
}

int main() {
    compute_inflated_size_from_file("/media/sf_amj018/Downloads/book-refdoc-API.pdf.zlib");

    unsigned char *buf = NULL;
    size_t compressed = read_compressed_file("/media/sf_amj018/Downloads/book-refdoc-API.pdf.zlib", &buf);
    if(NULL == buf) {
    }
    size_t uncompressed = compute_inflated_size_from_memory(buf, compressed);
    free(buf);

    printf("New Compressed: %zu\n", compressed);
    printf("New Uncompressed: %zu\n", uncompressed);

    return 0;
}
