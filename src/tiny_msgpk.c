/*
 * Copyright (c) [2021] [zmmfly]
 * [Tiny-Msgpk] is licensed under the license file "LICENSE"
 * See the file "LICENSE" for more details.
 */
#include "tiny_msgpk.h"
#include <stdio.h>

static msgpk_port_t hooks = {
    .malloc  = malloc,
    .free    = free,
    .realloc = realloc
};

#define MSGPK_CHK(a,b) if((a)==NULL) return (b);
#define MSGPK_REQCHK(msgpk, sz, ret) if (msgpk_buf_mem_require((msgpk), (sz)) == -1)return (ret)

int msgpk_add_uint(msgpk_t *msgpk, uint64_t dat)
{
    if (dat <= 127) {
        return msgpk_add_positive_fixint(msgpk, dat);

    } else if (dat <= 0xff) {
        return msgpk_add_uint8(msgpk, dat);

    } else if (dat <= 0xffff) {
        return msgpk_add_uint16(msgpk, dat);

    } else if (dat <= 0xffffffff) {
        return msgpk_add_uint32(msgpk, dat);

    } else if (dat <= 0xffffffffffffffff) {
        return msgpk_add_uint64(msgpk, dat);

    }
    return -1;
}

int msgpk_add_int(msgpk_t *msgpk, int64_t dat)
{
    if ( dat <= 127 && dat >= 0 ) {
        return msgpk_add_positive_fixint(msgpk, dat);

    } else if ( dat > -31 && dat <= 0) {
        return msgpk_add_negative_fixint(msgpk, dat);

    } else if ( dat >= SCHAR_MIN && dat <= SCHAR_MAX ) {
        return msgpk_add_int8(msgpk, dat);

    } else if ( dat >= SHRT_MIN && dat <= SHRT_MAX ) {
        return msgpk_add_int16(msgpk, dat);

    } else if ( dat >= INT_MIN && dat <= INT_MAX ) {
        return msgpk_add_int32(msgpk, dat);

    } else if ( dat >= LLONG_MIN && dat <= LLONG_MAX ) {
        return msgpk_add_int64(msgpk, dat);
    }
    return -1;
}

#if 0
int msgpk_add_float(msgpk_t *msgpk, double f)
{
    union{
        uint64_t u64;
        double f64;
        struct{
            uint64_t m:52;
            uint64_t e:11;
        };
    }fdat = {
        .f64 = f
    };

    // IEEE754, single precision, e max = 2^8-1, m max = 2^23-1
    if (fdat.e - 1023 <= 0xff && fdat.m <= 8388607 && fdat.m > -8388607) {
        printf("choose float32, e:%lx, m:%lx\n", fdat.e, fdat.m);
        return msgpk_add_float32(msgpk, f);
    } else {
        printf("choose float64, e:%lx, m:%lx\n", fdat.e, fdat.m);
        return msgpk_add_float64(msgpk, f);
    }
    return -1;
}
#endif

int msgpk_add_str(msgpk_t *msgpk, char *str, uint32_t len)
{
    if (len <= 0x1f) {
        return msgpk_add_fixstr(msgpk, str, len);

    } else if (len <= 0xff) {
        return msgpk_add_str8(msgpk, str, len);

    } else if (len <= 0xffff) {
        return msgpk_add_str16(msgpk, str, len);

    } else if (len <= 0xffffffff) {
        return msgpk_add_str32(msgpk, str, len);
    }
    return -1;
}

int msgpk_add_bin(msgpk_t *msgpk, uint8_t *dat, uint32_t len)
{
    if (len <= 0xff) {
        return msgpk_add_bin8(msgpk, dat, len);

    } else if (len <= 0xffff) {
        return msgpk_add_bin16(msgpk, dat, len);

    } else if (len <= 0xffffffff) {
        return msgpk_add_bin32(msgpk, dat, len);
    }
    return -1;
}

int msgpk_add_ext(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint32_t len)
{
    if (len == 1) {
        return msgpk_add_fixext1(msgpk, type, dat);

    }else if (len == 2) {
        return msgpk_add_fixext2(msgpk, type, dat);

    } else if (len == 4) {
        return msgpk_add_fixext4(msgpk, type, dat);

    } else if (len == 8) {
        return msgpk_add_fixext8(msgpk, type, dat);

    } else if (len == 16) {
        return msgpk_add_fixext16(msgpk, type, dat);

    } else if (len <= 0xff) {
        return msgpk_add_ext8(msgpk, type, dat, len);

    } else if (len <= 0xffff) {
        return msgpk_add_ext16(msgpk, type, dat, len);

    } else if (len <= 0xffffffff) {
        return msgpk_add_ext32(msgpk, type, dat, len);

    }
    return -1;
}

int msgpk_add_arr(msgpk_t *msgpk, uint32_t num)
{
    if (num <= 0x0f) {
        return msgpk_add_fixarr(msgpk, num);

    } else if (num <= 0xffff) {
        return msgpk_add_arr16(msgpk, num);

    } else if (num <= 0xffffffff) {
        return msgpk_add_arr32(msgpk, num);

    }
    return -1;
}

int msgpk_add_map(msgpk_t *msgpk, uint32_t num)
{
    if (num <= 0x0f) {
        return msgpk_add_fixmap(msgpk, num);

    } else if (num <= 0xffff) {
        return msgpk_add_map16(msgpk, num);

    } else if (num <= 0xffffffff) {
        return msgpk_add_map32(msgpk, num);

    }
    return -1;
}

int msgpk_add_negative_fixint(msgpk_t *msgpk, int8_t num)
{
    uint8_t dat = 0;
    MSGPK_CHK(msgpk,-1);
    if (num < -31)return -1;
    MSGPK_REQCHK(msgpk, 1,-1);

    dat = num & 0x1f;
    dat |= 0xe0;

    msgpk->msgpk_buf[msgpk->msgpk_sz] = dat;
    msgpk->msgpk_sz += 1;
    return 1;
}

int msgpk_add_map32(msgpk_t *msgpk, uint32_t num)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 5,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_MAP32;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = num>>24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = num>>16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = num>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4] = num;
    msgpk->msgpk_sz += 5;

    return 1;
}

int msgpk_add_map16(msgpk_t *msgpk, uint16_t num)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_MAP16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = num>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = num;
    msgpk->msgpk_sz += 3;

    return 1;
}

int msgpk_add_arr32(msgpk_t *msgpk, uint16_t num)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 5,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_ARR32;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = num>>24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = num>>16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = num>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4] = num;
    msgpk->msgpk_sz += 5;

    return 1;
}

int msgpk_add_arr16(msgpk_t *msgpk, uint16_t num)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_ARR16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = num>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = num;
    msgpk->msgpk_sz += 3;

    return 1;
}

int msgpk_add_str32(msgpk_t *msgpk, char *str, uint32_t len)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, len+5,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_STR32;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = len>>24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = len>>16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = len>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4] = len;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+5, str, len);
    msgpk->msgpk_sz += (len+5);

    return 1;
}

int msgpk_add_str16(msgpk_t *msgpk, char *str, uint16_t len)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, len+3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_STR16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = len>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = len;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+3, str, len);
    msgpk->msgpk_sz += (len+3);

    return 1;
}

int msgpk_add_str8(msgpk_t *msgpk, char *str, uint8_t len)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, len+2,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_STR8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = len;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+2, str, len);
    msgpk->msgpk_sz += (len+2);

    return 1;
}

int msgpk_add_int64(msgpk_t *msgpk, int64_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 9,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT64;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat>>56;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = dat>>48;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = dat>>40;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4] = dat>>32;
    msgpk->msgpk_buf[msgpk->msgpk_sz+5] = dat>>24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+6] = dat>>16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+7] = dat>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+8] = dat;
    msgpk->msgpk_sz+=9;

    return 1;
}

int msgpk_add_int32(msgpk_t *msgpk, int32_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 5,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT32;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat>>24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = dat>>16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = dat>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4] = dat;
    msgpk->msgpk_sz+=5;

    return 1;
}

int msgpk_add_int16(msgpk_t *msgpk, int16_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = dat;
    msgpk->msgpk_sz+=3;

    return 1;
}

int msgpk_add_int8(msgpk_t *msgpk, int8_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 2,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat;
    msgpk->msgpk_sz += 2;
    return 1;
}

int msgpk_add_uint64(msgpk_t *msgpk, uint64_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 9,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT64;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat>>56;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = dat>>48;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = dat>>40;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4] = dat>>32;
    msgpk->msgpk_buf[msgpk->msgpk_sz+5] = dat>>24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+6] = dat>>16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+7] = dat>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+8] = dat;
    msgpk->msgpk_sz+=9;

    return 1;
}

int msgpk_add_uint32(msgpk_t *msgpk, uint32_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 5,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT32;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat>>24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = dat>>16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = dat>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4] = dat;
    msgpk->msgpk_sz+=5;

    return 1;
}

int msgpk_add_uint16(msgpk_t *msgpk, uint16_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = dat;
    msgpk->msgpk_sz+=3;

    return 1;
}

int msgpk_add_uint8(msgpk_t *msgpk, uint8_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 2,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat;
    msgpk->msgpk_sz += 2;
    return 1;
}

int msgpk_add_float64(msgpk_t *msgpk, double f)
{
    union{
        uint64_t f_u64;
        double f_f;
    }dat = {
        .f_f = f
    };

    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 9,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_FLOAT64;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat.f_u64>>56;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = dat.f_u64>>48;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = dat.f_u64>>40;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4] = dat.f_u64>>32;
    msgpk->msgpk_buf[msgpk->msgpk_sz+5] = dat.f_u64>>24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+6] = dat.f_u64>>16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+7] = dat.f_u64>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+8] = dat.f_u64;
    msgpk->msgpk_sz += 9;

    return 1;
}

int msgpk_add_float32(msgpk_t *msgpk, float f)
{
    union{
        uint32_t f_u32;
        float f_f;
    }dat = {
        .f_f = f
    };

    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 5,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_FLOAT32;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat.f_u32>>24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = dat.f_u32>>16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = dat.f_u32>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4] = dat.f_u32;
    msgpk->msgpk_sz += 5;

    return 1;
}

int msgpk_add_ext32(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint32_t len)
{
    MSGPK_CHK(msgpk,-1);
    if (type < 0)return -1;
    MSGPK_REQCHK(msgpk, len+6,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_EXT32;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = len>>24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = len>>16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = len>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4] = len;
    msgpk->msgpk_buf[msgpk->msgpk_sz+5] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+6, dat, len);
    msgpk->msgpk_sz+= (len+6);

    return 1;
}

int msgpk_add_ext16(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint16_t len)
{
    MSGPK_CHK(msgpk,-1);
    if (type < 0)return -1;
    MSGPK_REQCHK(msgpk, len+4,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_EXT16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = len>>8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = len;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+4, dat, len);
    msgpk->msgpk_sz+= (len+4);

    return 1;
}

int msgpk_add_ext8(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint8_t len)
{
    MSGPK_CHK(msgpk,-1);
    if (type < 0)return -1;
    MSGPK_REQCHK(msgpk, len+3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_EXT8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = len;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+3, dat, len);
    msgpk->msgpk_sz+= (len+3);

    return 1;
}

int msgpk_add_fixext16(msgpk_t *msgpk, int8_t type, uint8_t *dat)
{
    MSGPK_CHK(msgpk,-1);
    if (type < 0)return -1;
    MSGPK_REQCHK(msgpk, 18,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_FIXEXT16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+2, dat, 16);
    msgpk->msgpk_sz+=18;

    return 1;
}

int msgpk_add_fixext8(msgpk_t *msgpk, int8_t type, uint8_t *dat)
{
    MSGPK_CHK(msgpk,-1);
    if (type < 0)return -1;
    MSGPK_REQCHK(msgpk, 10,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_FIXEXT8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+2, dat, 8);
    msgpk->msgpk_sz+=10;

    return 1;
}

int msgpk_add_fixext4(msgpk_t *msgpk, int8_t type, uint8_t *dat)
{
    MSGPK_CHK(msgpk,-1);
    if (type < 0)return -1;
    MSGPK_REQCHK(msgpk, 6,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_FIXEXT4;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+2, dat, 4);
    msgpk->msgpk_sz+=6;

    return 1;
}

int msgpk_add_fixext2(msgpk_t *msgpk, int8_t type, uint8_t *dat)
{
    MSGPK_CHK(msgpk,-1);
    if (type < 0)return -1;
    MSGPK_REQCHK(msgpk, 4,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_FIXEXT2;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = type;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = *dat++;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = *dat;
    msgpk->msgpk_sz+=4;

    return 1;
}

int msgpk_add_fixext1(msgpk_t *msgpk, int8_t type, uint8_t *dat)
{
    MSGPK_CHK(msgpk,-1);
    if (type < 0)return -1;
    MSGPK_REQCHK(msgpk, 3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_FIXEXT1;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = type;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = *dat;
    msgpk->msgpk_sz+=3;

    return 1;
}

int msgpk_add_bin32(msgpk_t *msgpk, uint8_t *dat, uint32_t len)
{
    MSGPK_CHK(msgpk,-1);
    if ( (uint8_t)len > 32) return -1;

    MSGPK_REQCHK(msgpk, len+4,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]    = FMTF_BIN32;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1]  = len >> 24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2]  = len >> 16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3]  = len >> 8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4]  = len;
    msgpk->msgpk_sz                     += 4;

    memcpy(msgpk->msgpk_buf + msgpk->msgpk_sz, dat, len);
    msgpk->msgpk_sz+=len;

    return 1;
}

int msgpk_add_bin16(msgpk_t *msgpk, uint8_t *dat, uint16_t len)
{
    MSGPK_CHK(msgpk,-1);
    if ( (uint8_t)len > 32) return -1;

    MSGPK_REQCHK(msgpk, len+3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]    = FMTF_BIN16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1]  = len >> 8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2]  = len;
    msgpk->msgpk_sz                     += 3;

    memcpy(msgpk->msgpk_buf + msgpk->msgpk_sz, dat, len);
    msgpk->msgpk_sz+=len;

    return 1;
}

int msgpk_add_bin8(msgpk_t *msgpk, uint8_t *dat, uint8_t len)
{
    MSGPK_CHK(msgpk,-1);
    if ( (uint8_t)len > 32) return -1;

    MSGPK_REQCHK(msgpk, len+2,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]    = FMTF_BIN8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1]  = len;
    msgpk->msgpk_sz                     += 2;

    memcpy(msgpk->msgpk_buf + msgpk->msgpk_sz, dat, len);
    msgpk->msgpk_sz+=len;

    return 1;
}

int msgpk_add_true(msgpk_t *msgpk)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 1,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_TRUE;
    msgpk->msgpk_sz++;
    return 1;
}

int msgpk_add_false(msgpk_t *msgpk)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 1,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_FALSE;
    msgpk->msgpk_sz++;
    return 1;
}

int msgpk_add_nil(msgpk_t *msgpk)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 1,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_NIL;
    msgpk->msgpk_sz++;
    return 1;
}

int msgpk_add_fixstr(msgpk_t *msgpk, char *str, uint8_t len)
{
    uint8_t hdr = 0;
    MSGPK_CHK(msgpk,-1);
    if ( len > 32) return -1;

    MSGPK_REQCHK(msgpk, len+1,-1);

    hdr = len & 0x1f;
    hdr |= 0xa0;

    msgpk->msgpk_buf[msgpk->msgpk_sz] = hdr;
    msgpk->msgpk_sz++;
    memcpy(msgpk->msgpk_buf + msgpk->msgpk_sz, str, len);
    msgpk->msgpk_sz+=len;

    return 1;
}

int msgpk_add_fixarr(msgpk_t *msgpk, uint8_t num)
{
    MSGPK_CHK(msgpk,-1);
    if ( (uint8_t)num > 16) return -1;

    num &= 0x0F;
    num |= 0x90;

    MSGPK_REQCHK(msgpk, 1,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = num;
    msgpk->msgpk_sz++;

    return 1;
}

int msgpk_add_fixmap(msgpk_t *msgpk, uint8_t num)
{
    MSGPK_CHK(msgpk,-1);
    if ( (uint8_t)num > 15) return -1;

    num &= 0x0F;
    num |= 0x80;

    MSGPK_REQCHK(msgpk, 1,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = num;
    msgpk->msgpk_sz++;

    return 1;
}

int msgpk_add_positive_fixint(msgpk_t *msgpk, int8_t num)
{
    MSGPK_CHK(msgpk,-1);
    if ( (uint8_t)num > 127) return -1;

    num &= 0x7f;

    MSGPK_REQCHK(msgpk, 1,-1);
    msgpk->msgpk_buf[msgpk->msgpk_sz] = num;
    msgpk->msgpk_sz++;
    return 1;
}

#include <stdio.h>

/**
 * @brief Memory require
 * 
 * @param msgpk MessagePack pointer
 * @param require_sz Require size
 * @return int 
 */
int msgpk_buf_mem_require(msgpk_t *msgpk, size_t require_sz)
{
    // uint8_t *newbuf = NULL;
    size_t newsz = 0;
    MSGPK_CHK(msgpk,-1);
    if (require_sz <=0) return 0;

    // if mem enough
    if (require_sz < (msgpk->buf_sz - msgpk->msgpk_sz) ) {
        return 1;
    }

    newsz = (require_sz / msgpk->buf_stepsz) < 1.0 ? msgpk->buf_sz + msgpk->buf_stepsz :
        msgpk->buf_sz + msgpk->buf_stepsz + require_sz;
    
    msgpk->msgpk_buf = (uint8_t *)hooks.realloc(msgpk->msgpk_buf, newsz);
    if (msgpk->msgpk_buf == NULL) return -1;

    msgpk->buf_sz = newsz;

    return 1;
}

/**
 * @brief Delete MessagePack
 * 
 * @param msgpk MessagePack pointer
 * @return int 
 */
int msgpk_delete(msgpk_t *msgpk)
{
    MSGPK_CHK(msgpk,-1);
    if (msgpk->msgpk_buf != NULL)hooks.free(msgpk->msgpk_buf);
    return 1;
}

/**
 * @brief Create MessagePack
 * 
 * @param init_sz Initialize buffer size
 * @param step_sz Step increase size if memory not enough
 * @return msgpk_t* 
 */
msgpk_t *msgpk_create(size_t init_sz, size_t step_sz)
{
    msgpk_t *msgpk = (msgpk_t *)hooks.malloc(sizeof(msgpk_t));
    MSGPK_CHK(msgpk, NULL);

    msgpk->msgpk_buf = (uint8_t *)hooks.malloc(init_sz);
    if (msgpk->msgpk_buf == NULL) {
        hooks.free(msgpk);
        return NULL;
    }

    msgpk->buf_stepsz = step_sz;
    msgpk->buf_sz     = init_sz;
    msgpk->msgpk_sz   = 0;
    return msgpk;
}


