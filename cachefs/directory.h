

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define FUSE_USE_VERSION 31

#include "inode.h"
#include <stdbool.h>
#include <stddef.h>

#define NAME_MAX 23
#define ROOT_INODE_ID 0

bool dir_create(int inode_id, __gid_t gid, __uid_t uid, __mode_t mode);

struct dir* dir_open(struct inode* inode);
struct dir* dir_open_root();
struct dir* dir_reopen(struct dir* dir);

struct inode* dir_get_inode(struct dir* dir);
struct inode* inode_from_path(const char* path);

bool dir_lookup(const struct dir*, const char* name, struct inode**);
bool dir_add(struct dir*, const char* name, int inode_id);
bool dir_remove(struct dir*, const char* name);
bool dir_readdir(struct dir*, char name[NAME_MAX + 1]);
bool dir_is_empty(struct dir* dir);

void dir_close(struct dir* dir);

#endif
