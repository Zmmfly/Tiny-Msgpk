/*
 * Copyright (c) [2021] [zmmfly]
 * [Tiny-Msgpk] is licensed under the license file "LICENSE"
 * See the file "LICENSE" for more details.
 */
#ifndef __TINY_MSGPK_H__
#define __TINY_MSGPK_H__

// #include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

// #define MSGPK_LMT_PFIXINT_MAX   127
// #define MSGPK_LMT_NFIXINT_MAX   -32
// #define MSGPK_LMT_FIXMAP_MAX    16
// #define MSGPK_LMT_FIXSTR_MAX    32
// #define MSGPK_LMT_BIN8_MAX      255
// #define MSGPK_LMT_BIN16_MAX     65536
// #define MSGPK_LMT_BIN32_MAX     4294967296
// #define MSGPK_LMT_EXT8_MAX      MSGPK_LMT_BIN8_MAX
// #define MSGPK_LMT_EXT16_MAX     MSGPK_LMT_BIN16_MAX
// #define MSGPK_LMT_EXT32_MAX     MSGPK_LMT_BIN32_MAX
// #define MSGPK_LMT_UINT8_MAX     MSGPK_LMT_BIN8_MAX
// #define MSGPK_LMT_UINT16_MAX    MSGPK_LMT_BIN16_MAX
// #define MSGPK_LMT_UINT32_MAX    MSGPK_LMT_BIN32_MAX


typedef enum tinymsgpk_format{
    FMTF_POSFIXINT = 0x00,
    FMTM_POSFIXINT = 0b10000000,
    FMTF_FIXMAP    = 0x80,
    FMTM_FIXMAP    = 0b11110000,
    FMTF_FIXARR    = 0x90,
    FMTM_FIXARR    = 0b11110000,
    FMTF_FIXSTR    = 0xa0,
    FMTM_FIXSTR    = 0b11100000,
    FMTF_NIL       = 0xc0,
    FMTF_NUSED,
    FMTF_FALSE,
    FMTF_TRUE,
    FMTF_BIN8,
    FMTF_BIN16,
    FMTF_BIN32,
    FMTF_EXT8,
    FMTF_EXT16,
    FMTF_EXT32,
    FMTF_FLOAT32,
    FMTF_FLOAT64,
    FMTF_UINT8,
    FMTF_UINT16,
    FMTF_UINT32,
    FMTF_UINT64,
    FMTF_INT8,
    FMTF_INT16,
    FMTF_INT32,
    FMTF_INT64,
    FMTF_FIXEXT1,
    FMTF_FIXEXT2,
    FMTF_FIXEXT4,
    FMTF_FIXEXT8,
    FMTF_FIXEXT16,
    FMTF_STR8,
    FMTF_STR16,
    FMTF_STR32,
    FMTF_ARR16,
    FMTF_ARR32,
    FMTF_MAP16,
    FMTF_MAP32,
    FMTF_NFIXINT = 0xe0,
    FMTM_NFIXINT = 0b11100000
}tinymsgpk_format_t;

typedef struct msgpk_port
{
    void *(*malloc)(size_t);
    void (*free)(void *);
    void *(*realloc)(void*, size_t);
}msgpk_port_t;

typedef struct msgpk
{
    uint8_t *msgpk_buf;
    size_t buf_sz;
    size_t buf_stepsz;
    size_t msgpk_sz;
}msgpk_t;

int msgpk_buf_mem_require(msgpk_t *msgpk, size_t require_sz);
msgpk_t *msgpk_create(size_t init_sz, size_t step_sz);
int msgpk_delete(msgpk_t *msgpk);

int msgpk_add_positive_fixint(msgpk_t *msgpk, int8_t num);
int msgpk_add_negative_fixint(msgpk_t *msgpk, int8_t num);

int msgpk_add_nil(msgpk_t *msgpk);
int msgpk_add_false(msgpk_t *msgpk);
int msgpk_add_true(msgpk_t *msgpk);

int msgpk_add_uint(msgpk_t *msgpk, uint64_t dat);   //TODO
int msgpk_add_uint8(msgpk_t *msgpk, uint8_t dat);
int msgpk_add_uint16(msgpk_t *msgpk, uint16_t dat);
int msgpk_add_uint32(msgpk_t *msgpk, uint32_t dat);
int msgpk_add_uint64(msgpk_t *msgpk, uint64_t dat);

int msgpk_add_int(msgpk_t *msgpk, int64_t dat); //TODO
int msgpk_add_int8(msgpk_t *msgpk, int8_t dat);
int msgpk_add_int16(msgpk_t *msgpk, int16_t dat);
int msgpk_add_int32(msgpk_t *msgpk, int32_t dat);
int msgpk_add_int64(msgpk_t *msgpk, int64_t dat);

// int msgpk_add_float(msgpk_t *msgpk, double f);  //TODO
int msgpk_add_float32(msgpk_t *msgpk, float f);
int msgpk_add_float64(msgpk_t *msgpk, double f);

int msgpk_add_str(msgpk_t *msgpk, char *str, uint32_t len); //TODO
int msgpk_add_fixstr(msgpk_t *msgpk, char *str, uint8_t len);
int msgpk_add_str8(msgpk_t *msgpk, char *str, uint8_t len);
int msgpk_add_str16(msgpk_t *msgpk, char *str, uint16_t len);
int msgpk_add_str32(msgpk_t *msgpk, char *str, uint32_t len);

int msgpk_add_bin(msgpk_t *msgpk, uint8_t *dat, uint32_t len);      //TODO
int msgpk_add_bin8(msgpk_t *msgpk, uint8_t *dat, uint8_t len);
int msgpk_add_bin16(msgpk_t *msgpk, uint8_t *dat, uint16_t len);
int msgpk_add_bin32(msgpk_t *msgpk, uint8_t *dat, uint32_t len);

int msgpk_add_ext(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint32_t len); //TODO
int msgpk_add_fixext1(msgpk_t *msgpk, int8_t type, uint8_t *dat);
int msgpk_add_fixext2(msgpk_t *msgpk, int8_t type, uint8_t *dat);
int msgpk_add_fixext4(msgpk_t *msgpk, int8_t type, uint8_t *dat);
int msgpk_add_fixext8(msgpk_t *msgpk, int8_t type, uint8_t *dat);
int msgpk_add_fixext16(msgpk_t *msgpk, int8_t type, uint8_t *dat);
int msgpk_add_ext8(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint8_t len);
int msgpk_add_ext16(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint16_t len);
int msgpk_add_ext32(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint32_t len);

int msgpk_add_arr(msgpk_t *msgpk, uint32_t num);    //TODO
int msgpk_add_fixarr(msgpk_t *msgpk, uint8_t num);
int msgpk_add_arr16(msgpk_t *msgpk, uint16_t num);
int msgpk_add_arr32(msgpk_t *msgpk, uint16_t num);

int msgpk_add_map(msgpk_t *msgpk, uint32_t num);  //TODO
int msgpk_add_fixmap(msgpk_t *msgpk, uint8_t num);
int msgpk_add_map16(msgpk_t *msgpk, uint16_t num);
int msgpk_add_map32(msgpk_t *msgpk, uint32_t num);

#endif