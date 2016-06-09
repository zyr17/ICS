#include "common.h"
#include "fs.h"

#include <string.h>

typedef struct {
	char *name;
	uint32_t size;
	uint32_t disk_offset;
} file_info;

enum {SEEK_SET, SEEK_CUR, SEEK_END};

/* This is the information about all files in disk. */
static const file_info file_table[] = {
	{"1.rpg", 188864, 1048576}, {"2.rpg", 188864, 1237440},
	{"3.rpg", 188864, 1426304}, {"4.rpg", 188864, 1615168},
	{"5.rpg", 188864, 1804032}, {"abc.mkf", 1022564, 1992896},
	{"ball.mkf", 134704, 3015460}, {"data.mkf", 66418, 3150164},
	{"desc.dat", 16027, 3216582}, {"fbp.mkf", 1128064, 3232609},
	{"fire.mkf", 834728, 4360673}, {"f.mkf", 186966, 5195401},
	{"gop.mkf", 11530322, 5382367}, {"map.mkf", 1496578, 16912689},
	{"mgo.mkf", 1577442, 18409267}, {"m.msg", 188232, 19986709},
	{"mus.mkf", 331284, 20174941}, {"pat.mkf", 8488, 20506225},
	{"rgm.mkf", 453202, 20514713}, {"rng.mkf", 4546074, 20967915},
	{"sss.mkf", 557004, 25513989}, {"voc.mkf", 1997044, 26070993},
	{"wor16.asc", 5374, 28068037}, {"wor16.fon", 82306, 28073411},
	{"word.dat", 5650, 28155717},
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void ide_read(uint8_t *, uint32_t, uint32_t);
void ide_write(uint8_t *, uint32_t, uint32_t);

/* TODO: implement a simplified file system here. */

typedef struct {
	bool opened;
	uint32_t offset;
} Fstate;

Fstate fstate[NR_FILES + 3];

void init_fs(){
    fstate[0].opened = fstate[1].opened = fstate[2].opened = 1;
}

uint32_t fs_open(const char *pathname, int flags){
#ifdef DEBUG
    Log("open filename: %s", pathname);
#endif
    int i = 0;
    for (; i < NR_FILES; i ++ )
        if (strcmp(pathname, file_table[i].name) == 0){
            if (fstate[i + 3].opened){
                Log("filename \'%s\' is opening!", pathname);
                nemu_assert(0);
                return -1;
            }
#ifdef DEBUG
            Log("open file fd: %d", i + 3);
#endif
            fstate[i + 3].opened = 1;
            return i + 3;
        }
    Log("filename \'%s\' not found!", pathname);
    nemu_assert(0);
    return -1;
}

uint32_t fs_read(int fd, void *buf, int len){
    nemu_assert(fd >= 0 && fd < NR_FILES + 3);
    nemu_assert(fstate[fd].opened);
    nemu_assert(!(fd >= 0 && fd < 3)); // read stdxxx
    if (file_table[fd - 3].size - fstate[fd].offset < len) len = file_table[fd - 3].size - fstate[fd].offset;
    ide_read((uint8_t*)buf, file_table[fd - 3].disk_offset + fstate[fd].offset, len);
    fstate[fd].offset += len;
    return len;
}

uint32_t fs_write(int fd, void *buf, int len){
    nemu_assert(fd >= 0 && fd < NR_FILES + 3);
    nemu_assert(fstate[fd].opened);
    nemu_assert(!(fd >= 0 && fd < 3)); // write stdxxx
    nemu_assert(file_table[fd - 3].size - fstate[fd].offset - len >= 0);
    ide_write((uint8_t*)buf, file_table[fd - 3].disk_offset + fstate[fd].offset, len);
    fstate[fd].offset += len;
    return len;
}

uint32_t fs_lseek(int fd, int offset, int whence){
#ifdef DEBUG
    Log("lseek fd: %d, opened: %d, offset: %d", fd, fstate[fd].opened, fstate[fd].offset);
#endif
    nemu_assert(fd >= 0 && fd < NR_FILES + 3);
    if (!fstate[fd].opened){
        nemu_assert(0);
    }
    nemu_assert(!(fd >= 0 && fd < 3)); // lseek stdxxx
    if (whence == SEEK_SET) fstate[fd].offset = offset;
    else if (whence == SEEK_CUR) fstate[fd].offset += offset;
    else if (whence == SEEK_END) fstate[fd].offset = offset + file_table[fd - 3].size;
    else nemu_assert(0);
    return fstate[fd].offset;
}

uint32_t fs_close(int fd){
#ifdef DEBUG
    Log("close fd: %d, opened: %d, offset: %d", fd, fstate[fd].opened, fstate[fd].offset);
#endif
    if (fd >= 3 && fd < NR_FILES + 3){
        fstate[fd].offset = 0;
        fstate[fd].opened = 0;
        return 0;
    }
    return -1;
}


