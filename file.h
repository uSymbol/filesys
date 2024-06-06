//
// Created by 84424 on 2024/6/6.
//

#ifndef FILE_SYSTEM_FILE_H
#define FILE_SYSTEM_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef float f32;
typedef double f64;

// 文件头
#pragma pack(push, 1)
typedef struct {
    i8 version[16];       // 版本信息
    time_t timestamp;       // 最后写入时间
    i32 item_num_max;       // 条目的最大数
    i32 item_num_cur;       // 当前条目数
    i32 item_len;           // 条目大小
    i32 sizeof_key;         // 索引长度
} file_header_s;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    u64 offset;
    i16 full;
    i16 len;
} item_header_s;
#pragma pack(pop)

typedef struct {
    item_header_s header;
    void *key;
} item_s;


typedef struct {
    FILE *fp;
    file_header_s header;
    item_s *items;

} storage_file_s;

extern storage_file_s *storage_file_init(char *filename, int max_num, int key_len, int value_len);
extern int storage_insert(storage_file_s *obj, void *key, void *value, int value_len);
extern int storage_delete(storage_file_s *obj, void *key);
extern int storage_update(storage_file_s *obj, void *key, void *value, int value_len);
extern int storage_find(storage_file_s *obj, void *key, void *value, int value_len);

#endif //FILE_SYSTEM_FILE_H
