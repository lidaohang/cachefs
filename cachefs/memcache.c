#include "memcache.h"
#include <assert.h>
#include <string.h>

struct memcache_t {
    int fd;
};

void DumpHex(const void* data, size_t size)
{
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        printf("%02X ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i + 1) % 8 == 0 || i + 1 == size) {
            printf(" ");
            if ((i + 1) % 16 == 0) {
                printf("|  %s \n", ascii);
            } else if (i + 1 == size) {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8) {
                    printf(" ");
                }
                for (j = (i + 1) % 16; j < 16; ++j) {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}

int line_start_index(const char* st, int line)
{

    for (int i = 0; st[i] != '\0'; i++) {
        if (i == 0 || st[i - 1] == '\n') {
            line--;
            if (line == 0)
                return i;
        }
    }
    return -1;
}

struct memcache_t* memcache_init()
{
    struct memcache_t* memcache = malloc(sizeof(struct memcache_t));
    if (memcache == NULL)
        return NULL;

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    struct in_addr s_addr;
    if (!inet_pton(AF_INET, MEMCACHED_ADDRESS, &s_addr.s_addr)) {
        free(memcache);
        return NULL;
    }

    struct sockaddr_in addr;
    addr.sin_addr = s_addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MEMCACHED_PORT);
    if (connect(clientfd, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
        free(memcache);
        return NULL;
    }

    memcache->fd = clientfd;

    return memcache;
}

void memcache_create(struct memcache_t* memcache)
{
    assert(memcache == NULL);
    int value = CONSISTENCY_VALUE;
    memcache_add(memcache, CONSISTENCY_KEY, (void*)&value, sizeof(int));
}

bool memcache_is_consistent(struct memcache_t* memcache)
{
    assert(memcache != NULL);
    int value;
    return (memcache_get(memcache, CONSISTENCY_KEY, (void*)&value) && value == CONSISTENCY_VALUE);
}

bool memcache_get(struct memcache_t* memcache, const char* key, void* buff)
{
    char data[2048];
    int filled = sprintf(data, "get %s\r\n", key);

    if (write(memcache->fd, data, filled) < filled) {
        return false;
    }
    if (read(memcache->fd, data, 2048) >= 0) {
        char ret_key[64];
        int ret_exp, ret_size;
        sscanf(data, "VALUE %s %d %d\r\n", ret_key, &ret_exp, &ret_size);
        memcpy(buff, data + line_start_index(data, 2), ret_size);
        return true;
    } else {
        return false;
    }
}

bool memcache_add(struct memcache_t* memcache, const char* key,
    const void* value, size_t size)
{
    char data[2048];
    int filled = sprintf(data, "set %s 0 0 %zu\r\n", key, size);
    memcpy(data + filled, value, size);
    filled += size;

    filled += sprintf(data + filled, "\r\n");

    if (write(memcache->fd, data, filled) < filled) {
        return false;
    }
    char result[64];
    return (read(memcache->fd, result, 64) >= 0 && strncmp(result, "STORED", 6) == 0);
}

void memcache_close(struct memcache_t* memcache)
{
    if (memcache == NULL)
        return;
    close(memcache->fd);
    free(memcache);
}

bool memcache_clear(struct memcache_t* memcache)
{
    char data[2048];
    int filled = sprintf(data, "flush_all\r\n");

    if (write(memcache->fd, data, filled) < filled) {
        assert(1 == 2);
        return false;
    }

    char result[64];
    return (read(memcache->fd, result, 64) >= 0 && strncmp(result, "OK", 2) == 0);
}