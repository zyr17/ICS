#ifndef __FS_H__
#define __FS_H__

void init_fs();
uint32_t fs_open(const char *pathname, int flags);
uint32_t fs_read(int fd, void *buf, int len);
uint32_t fs_write(int fd, void *buf, int len);
uint32_t fs_lseek(int fd, int offset, int whence);
uint32_t fs_close(int fd);

#endif
