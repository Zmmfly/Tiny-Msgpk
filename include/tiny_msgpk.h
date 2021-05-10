/*
 * Copyright (c) [2021] [zmmfly]
 * [Tiny-Msgpk] is licensed under the license file "LICENSE"
 * See the file "LICENSE" for more details.
 */
#ifndef __TINY_MSGPK_H__
#define __TINY_MSGPK_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#ifndef RDWR_INLINE
#define RDWR_INLINE
#endif

#define MSGPK_OK    0
#define MSGPK_ERR   -1

#ifdef __cplusplus
extern "C" {
#endif

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
    FMTF_PFIXINT = 0x00,
    FMTM_PFIXINT = 0b10000000,
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
    void *(*calloc)(size_t, size_t);
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

typedef struct msgpk_parse
{
    msgpk_t *pk;
    size_t idx_cur;
    size_t idx_nxt;
}msgpk_parse_t;

typedef enum msgpk_type{
    MSGPK_INT8 = 0x00,
    MSGPK_INT16,
    MSGPK_INT32,
    MSGPK_INT64,
    MSGPK_UINT8,
    MSGPK_UINT16,
    MSGPK_UINT32,
    MSGPK_UINT64,
    MSGPK_FLOAT32,
    MSGPK_FLOAT64,
    MSGPK_STRING,
    MSGPK_NIL,
    MSGPK_FALSE,
    MSGPK_TRUE,
    MSGPK_BOOL,
    MSGPK_MAP,
    MSGPK_ARR,
    MSGPK_BIN,
    MSGPK_EXT,
    MSGPK_TIMESTAMP
}msgpk_type_t;

typedef struct msgpk_decode
{
    msgpk_type_t type_dec;
    uint8_t type_ext;
    union
    {
        uint64_t u64;
        uint32_t u32;
        uint16_t u16;
        uint8_t u8;
        int64_t i64;
        int32_t i32;
        int16_t i16;
        int8_t i8;
        float f32;
        double f64;
        char *str;
        uint8_t *bin;
        uint8_t *ext;
        uint8_t boolean;
    };
    size_t length;

}msgpk_decode_t;

#define MSGPK_CHK(a,b) if((a)==NULL) return (b)
#define MSGPK_REQCHK(msgpk, sz, ret) if (msgpk_buf_mem_require((msgpk), (sz)) == MSGPK_ERR)return (ret)

int msgpk_buf_mem_require(msgpk_t *msgpk, size_t require_sz);
msgpk_t *msgpk_create(size_t init_sz, size_t step_sz);
int msgpk_delete(msgpk_t *msgpk, uint8_t del_buf, uint8_t destory);

int msgpk_add_positive_fixint(msgpk_t *msgpk, int8_t num);
int msgpk_add_negative_fixint(msgpk_t *msgpk, int8_t num);

int msgpk_add_nil(msgpk_t *msgpk);
int msgpk_add_false(msgpk_t *msgpk);
int msgpk_add_true(msgpk_t *msgpk);

int msgpk_add_uint(msgpk_t *msgpk, uint64_t dat);
int msgpk_add_uint8(msgpk_t *msgpk, uint8_t dat);
int msgpk_add_uint16(msgpk_t *msgpk, uint16_t dat);
int msgpk_add_uint32(msgpk_t *msgpk, uint32_t dat);
int msgpk_add_uint64(msgpk_t *msgpk, uint64_t dat);

int msgpk_add_int(msgpk_t *msgpk, int64_t dat);
int msgpk_add_int8(msgpk_t *msgpk, int8_t dat);
int msgpk_add_int16(msgpk_t *msgpk, int16_t dat);
int msgpk_add_int32(msgpk_t *msgpk, int32_t dat);
int msgpk_add_int64(msgpk_t *msgpk, int64_t dat);

// int msgpk_add_float(msgpk_t *msgpk, double f);  //cancel
int msgpk_add_float32(msgpk_t *msgpk, float f);
int msgpk_add_float64(msgpk_t *msgpk, double f);

int msgpk_add_str(msgpk_t *msgpk, char *str, uint32_t len);
int msgpk_add_fixstr(msgpk_t *msgpk, char *str, uint8_t len);
int msgpk_add_str8(msgpk_t *msgpk, char *str, uint8_t len);
int msgpk_add_str16(msgpk_t *msgpk, char *str, uint16_t len);
int msgpk_add_str32(msgpk_t *msgpk, char *str, uint32_t len);

int msgpk_add_bin(msgpk_t *msgpk, uint8_t *dat, uint32_t len);
int msgpk_add_bin8(msgpk_t *msgpk, uint8_t *dat, uint8_t len);
int msgpk_add_bin16(msgpk_t *msgpk, uint8_t *dat, uint16_t len);
int msgpk_add_bin32(msgpk_t *msgpk, uint8_t *dat, uint32_t len);

int msgpk_add_ext(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint32_t len);
int msgpk_add_fixext1(msgpk_t *msgpk, int8_t type, uint8_t *dat);
int msgpk_add_fixext2(msgpk_t *msgpk, int8_t type, uint8_t *dat);
int msgpk_add_fixext4(msgpk_t *msgpk, int8_t type, uint8_t *dat);
int msgpk_add_fixext8(msgpk_t *msgpk, int8_t type, uint8_t *dat);
int msgpk_add_fixext16(msgpk_t *msgpk, int8_t type, uint8_t *dat);
int msgpk_add_ext8(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint8_t len);
int msgpk_add_ext16(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint16_t len);
int msgpk_add_ext32(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint32_t len);

int msgpk_add_arr(msgpk_t *msgpk, uint32_t num);
int msgpk_add_fixarr(msgpk_t *msgpk, uint8_t num);
int msgpk_add_arr16(msgpk_t *msgpk, uint16_t num);
int msgpk_add_arr32(msgpk_t *msgpk, uint16_t num);

int msgpk_add_map(msgpk_t *msgpk, uint32_t num);
int msgpk_add_fixmap(msgpk_t *msgpk, uint8_t num);
int msgpk_add_map16(msgpk_t *msgpk, uint16_t num);
int msgpk_add_map32(msgpk_t *msgpk, uint32_t num);

int msgpk_parse_next(msgpk_parse_t *parse);
int msgpk_parse_get(msgpk_parse_t *parse, msgpk_decode_t *dec);
uint8_t msgpk_parse_get_currnet_flag(msgpk_parse_t *parse);
int msgpk_parse_deinit(msgpk_parse_t *parse);
int msgpk_parse_init(msgpk_parse_t *parse, uint8_t *dat, size_t length);
void msgpk_set_port(msgpk_port_t *port);

#ifndef RDWR_INLINE
uint16_t msgpk_rd_u16_bigend(uint8_t *dat);
uint32_t msgpk_rd_u32_bigend(uint8_t *dat);
uint64_t msgpk_rd_u64_bigend(uint8_t *dat);
void msgpk_wr_u16_bigend(uint8_t *dat, uint16_t u16);
void msgpk_wr_u32_bigend(uint8_t *dat, uint32_t u32);
void msgpk_wr_u64_bigend(uint8_t *dat, uint64_t u64);
#else
static __inline uint16_t msgpk_rd_u16_bigend(uint8_t *dat)
{
    uint16_t u16 = 0;
    MSGPK_CHK(dat, 0);
    u16   = *dat++;
    u16 <<= 8;
    u16  |= *dat;
    return u16;
}

static __inline uint32_t msgpk_rd_u32_bigend(uint8_t *dat)
{
    uint32_t u32 = 0;
    MSGPK_CHK(dat, 0);
    u32   = *dat++;
    u32 <<= 8;
    u32  |= *dat++;
    u32 <<= 8;
    u32  |= *dat++;
    u32 <<= 8;
    u32  |= *dat;
    return u32;
}

static __inline uint64_t msgpk_rd_u64_bigend(uint8_t *dat)
{
    uint64_t u64 = 0;
    MSGPK_CHK(dat, 0);
    u64   = *dat++;
    u64 <<= 8;
    u64  |= *dat++;
    u64 <<= 8;
    u64  |= *dat++;
    u64 <<= 8;
    u64  |= *dat++;
    u64 <<= 8;
    u64  |= *dat++;
    u64 <<= 8;
    u64  |= *dat++;
    u64 <<= 8;
    u64  |= *dat++;
    u64 <<= 8;
    u64  |= *dat;
    return u64;
}

static __inline void msgpk_wr_u16_bigend(uint8_t *dat, uint16_t u16)
{
    *dat++ = u16>>8;
    *dat   = u16;
}

static __inline void msgpk_wr_u32_bigend(uint8_t *dat, uint32_t u32)
{
    *dat++ = u32>>24;
    *dat++ = u32>>16;
    *dat++ = u32>>8;
    *dat   = u32;
}

static __inline void msgpk_wr_u64_bigend(uint8_t *dat, uint64_t u64)
{
    *dat++ = u64>>56;
    *dat++ = u64>>48;
    *dat++ = u64>>40;
    *dat++ = u64>>32;
    *dat++ = u64>>24;
    *dat++ = u64>>16;
    *dat++ = u64>>8;
    *dat   = u64;
}
#endif

#ifdef __cplusplus
}
#endif

#endif