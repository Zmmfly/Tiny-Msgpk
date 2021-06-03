#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <iostream>
#include <fstream>
#include "tiny_msgpk.h"

#define TYPE_MALLOC     1
#define TYPE_CALLOC     2
#define TYPE_REALLOC    3
#define MSGPK_PATH  "msgpk.bin"


typedef struct allocate_info
{
    void *ptr;
    uint8_t type;
    size_t sz;
}allocate_info_t;

typedef std::list<allocate_info_t> allocate_info_list_t;

allocate_info_list_t list;

void *t_malloc(size_t sz)
{
    allocate_info_t info;
    info.ptr = malloc(sz);
    info.sz = sz;
    if (info.ptr == NULL) return NULL;
    info.type = TYPE_MALLOC;

    list.push_back(info);
    return info.ptr;
}

void *t_calloc(size_t elm_num, size_t elm_sz)
{
    allocate_info_t info;
    info.ptr = calloc(elm_num, elm_sz);
    info.sz = elm_num * elm_sz;
    if (info.ptr == NULL) return NULL;
    info.type = TYPE_CALLOC;

    list.push_back(info);
    return info.ptr;
}

void t_free(void *ptr)
{
    for (auto it = list.begin(); it != list.end(); it++) {
        if (it->ptr == ptr) {
            free(ptr);
            list.erase(it);
            return;
        }
    }
}

void *t_realloc(void *ptr, size_t sz)
{
    allocate_info_t info;
    info.ptr = realloc(ptr, sz);
    info.sz = sz;
    if (info.ptr == NULL) return NULL;

    for (auto it = list.begin(); it != list.end(); )
    {
        if (it->ptr == ptr) {
            list.erase(it++);
            break;
        } else {
            it++;
        }
    }

    info.type = TYPE_REALLOC;

    list.push_back(info);
    return info.ptr;
}

msgpk_port_t port = {
    .malloc  = t_malloc,
    .calloc  = t_calloc,
    .free    = t_free,
    .realloc = t_realloc
};

void prHex(uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        printf("%02x", data[i]);
        printf( ((i+1) % 16) ? " " : "\n" );
    }
    if(len % 16)printf("\n");
}

void parse(uint8_t *dat, size_t length)
{
    msgpk_parse_t parse;
    msgpk_decode_t decode;

    printf("Parse  init\n");
    msgpk_parse_init(&parse, dat, length);

    printf("Parse  start\n");
    do
    {
        if ( msgpk_parse_get(&parse, &decode) == -1) {
            printf("parse get error\n");
            break;
        }
        switch (decode.type_dec)
        {
            case MSGPK_INT8:
                printf("INT8:%d\n", decode.i8);
                break;

            case MSGPK_INT16:
                printf("INT16:%d\n", decode.i16);
                break;

            case MSGPK_INT32:
                printf("INT32:%d\n", decode.i32);
                break;

            case MSGPK_INT64:
                printf("INT64:%lld\n", decode.i64);
                break;

            case MSGPK_UINT8:
                printf("UINT8:%u\n", decode.u8);
                break;

            case MSGPK_UINT16:
                printf("UINT16:%u\n", decode.u16);
                break;

            case MSGPK_UINT32:
                printf("UINT32:%u\n", decode.u32);
                break;

            case MSGPK_UINT64:
                printf("UINT64:%llu\n", decode.u64);
                break;

            case MSGPK_FLOAT32:
                printf("FLOAT32:%f\n", decode.f32);
                break;

            case MSGPK_FLOAT64:
                printf("FLOAT64:%f\n", decode.f64);
                break;

            case MSGPK_STRING:
                printf("STRING:%.*s\n", decode.length, decode.str);
                break;

            case MSGPK_NIL:
                printf("NIL\n");
                break;

            case MSGPK_FALSE:
                printf("FALSE\n");
                break;

            case MSGPK_TRUE:
                printf("TRUE\n");
                break;

            case MSGPK_MAP:
                printf("MAP:%u\n", decode.length);
                break;

            case MSGPK_ARR:
                printf("ARR:%u\n", decode.length);
                break;

            case MSGPK_BIN:
                printf("BIN:%u\n", decode.length);
                prHex(decode.bin, decode.length);
                break;

            case MSGPK_EXT:
                printf("EXT: type(%u), length(%u)\n", decode.type_ext, decode.length);
                prHex(decode.bin, decode.length);
                break;

            case MSGPK_TIMESTAMP:
                printf("EXT: Timestamp\n");
                break;
        
            default:
                break;
        }
    }
    while( !msgpk_parse_next(&parse) );
    msgpk_parse_deinit(&parse);
    printf("end\n");
    return;
}

void parse_file(const char *path)
{
    msgpk_parse_t parse;
    msgpk_decode_t decode;

    printf("Parse  init\n");
    msgpk_parse_init_file(&parse, path);

    printf("Parse  start\n");
    do
    {
        if ( msgpk_parse_get(&parse, &decode) == -1) {
            printf("parse get error\n");
            break;
        }
        switch (decode.type_dec)
        {
            case MSGPK_INT8:
                printf("INT8:%d\n", decode.i8);
                break;

            case MSGPK_INT16:
                printf("INT16:%d\n", decode.i16);
                break;

            case MSGPK_INT32:
                printf("INT32:%d\n", decode.i32);
                break;

            case MSGPK_INT64:
                printf("INT64:%lld\n", decode.i64);
                break;

            case MSGPK_UINT8:
                printf("UINT8:%u\n", decode.u8);
                break;

            case MSGPK_UINT16:
                printf("UINT16:%u\n", decode.u16);
                break;

            case MSGPK_UINT32:
                printf("UINT32:%u\n", decode.u32);
                break;

            case MSGPK_UINT64:
                printf("UINT64:%llu\n", decode.u64);
                break;

            case MSGPK_FLOAT32:
                printf("FLOAT32:%f\n", decode.f32);
                break;

            case MSGPK_FLOAT64:
                printf("FLOAT64:%f\n", decode.f64);
                break;

            case MSGPK_STRING:
                printf("STRING:%.*s\n", decode.length, decode.str);
                break;

            case MSGPK_NIL:
                printf("NIL\n");
                break;

            case MSGPK_FALSE:
                printf("FALSE\n");
                break;

            case MSGPK_TRUE:
                printf("TRUE\n");
                break;

            case MSGPK_MAP:
                printf("MAP:%u\n", decode.length);
                break;

            case MSGPK_ARR:
                printf("ARR:%u\n", decode.length);
                break;

            case MSGPK_BIN:
                printf("BIN:%u\n", decode.length);
                prHex(decode.bin, decode.length);
                break;

            case MSGPK_EXT:
                printf("EXT: type(%u), length(%u)\n", decode.type_ext, decode.length);
                prHex(decode.bin, decode.length);
                break;

            case MSGPK_TIMESTAMP:
                printf("EXT: Timestamp\n");
                break;
        
            default:
                break;
        }
    }
    while( !msgpk_parse_next(&parse) );
    msgpk_parse_deinit(&parse);
    printf("end\n");
    return;
}

int main(void)
{
    uint8_t *buf = NULL;
    msgpk_t *msgpk = NULL;
    std::ifstream file;

    msgpk_set_port(&port);

    // msgpk = msgpk_create(8, 4);
    msgpk = msgpk_file_create(MSGPK_PATH, 0xffffffff);
    msgpk_add_map(msgpk, 5);

    msgpk_add_str(msgpk, "fixstr", 6);
    msgpk_add_str(msgpk, "hello world", 11);

    
    msgpk_add_str(msgpk, "number", 6);
    msgpk_add_map(msgpk, 10);

        
        msgpk_add_str(msgpk, "float32", 7);
        msgpk_add_float32(msgpk, 1.256);
        
        msgpk_add_str(msgpk, "float64", 7);
        msgpk_add_float64(msgpk, 1.256);
        
        msgpk_add_str(msgpk, "uint8", 5);
        msgpk_add_uint(msgpk, 254);
        
        msgpk_add_str(msgpk, "uint16", 6);
        msgpk_add_uint(msgpk, 65530);

        msgpk_add_str(msgpk, "uint32", 6);
        msgpk_add_uint(msgpk, 0x1fffffff);
        
        msgpk_add_str(msgpk, "uint64", 6);
        msgpk_add_uint(msgpk, 0x1122334455667788);
        
        msgpk_add_str(msgpk, "int8", 4);
        msgpk_add_int(msgpk, -125);
        
        msgpk_add_str(msgpk, "int16", 5);
        msgpk_add_int(msgpk, -6000);
        
        msgpk_add_str(msgpk, "int32", 5);
        msgpk_add_int(msgpk, -7556000);
        
        msgpk_add_str(msgpk, "int64", 5);
        msgpk_add_int(msgpk, -755600000000000);
    
    msgpk_add_str(msgpk, "extbin", 6);
    msgpk_add_map(msgpk, 8);
        
        msgpk_add_str(msgpk, "fixext1", 7);
        uint8_t fixext1[] = {0xaf};
        msgpk_add_ext(msgpk, 1, fixext1, 1);
        
        msgpk_add_str(msgpk, "fixext2", 7);
        uint8_t fixext2[] = {0xaa,0xbb};
        msgpk_add_ext(msgpk, 2, fixext2, 2);
        
        msgpk_add_str(msgpk, "fixext4", 7);
        uint8_t fixext4[] = {0xa1,0xf2,0xf3,0xf4};
        msgpk_add_ext(msgpk, 3, fixext4, 4);
        
        msgpk_add_str(msgpk, "fixext8", 7);
        static uint8_t fixext8[] = {0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x11,0x22};
        msgpk_add_ext(msgpk, 4, fixext8, 8);
        
        msgpk_add_str(msgpk, "fixext16", 8);
        static uint8_t fixext16[] = {
            0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
            0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
        };
        msgpk_add_ext(msgpk, 5, fixext16, 16);
        // msgpk_add_fixext16(msgpk, 5, fixext16);
        
        msgpk_add_str(msgpk, "ext8", 4);
        uint8_t ext8[] = {0xaa, 0xbb, 0xcc, 0xdd};
        msgpk_add_ext(msgpk, 6, ext8, 4);
        
        msgpk_add_str(msgpk, "ext16", 5);
        uint8_t ext16[] = {0xee, 0xff, 0xaa, 0xbb};
        msgpk_add_ext(msgpk, 7, ext16, 4);
        
        msgpk_add_str(msgpk, "ext32", 5);
        uint8_t ext32[] = {0xcc, 0xdd, 0xee, 0xff};
        msgpk_add_ext(msgpk, 8, ext32, 4);
    
    msgpk_add_str(msgpk, "arr16", 5);
    msgpk_add_arr(msgpk, 3);
        msgpk_add_int(msgpk, -1);
        msgpk_add_int(msgpk, -2);
        msgpk_add_int(msgpk, -3);
    
    msgpk_add_str(msgpk, "arr32", 5);
    msgpk_add_arr(msgpk, 3);
        msgpk_add_int(msgpk, -4);
        msgpk_add_int(msgpk, -5);
        msgpk_add_int(msgpk, -6);

    exit:
    msgpk_file_done(msgpk, 1);    
    buf = (uint8_t *)malloc(msgpk->msgpk_sz);
    if (buf == NULL) {
        printf("Nomem\n");
        return 0;
    }
    file.open(MSGPK_PATH, std::ios::binary| std::ios::in);
    if (!file.is_open()) {
        printf("Read error\n");
        return 0;
    }
    file.seekg(0, std::ios::end);
    size_t sz = file.tellg();
    printf("File sz: %u\n", sz);
    file.seekg(0);
    file.read((char *)buf, sz);
    prHex(buf, sz);
    file.close();
    // file.clear();
    free(msgpk);

    // parse(buf, sz);
    free(buf);

    parse_file(MSGPK_PATH);

    // mem leak check
    if (list.size() > 0) {
        printf("Had memory leak\n");
        for (auto it = list.begin(); it != list.end(); it++) 
        {
            printf("Leak addr: %p, size: %u, type: %u\n", it->ptr, it->sz, it->type);
        }
    } else {
        printf("No memory leak\n");
    }
    return 0;
}