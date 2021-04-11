/*
 * Copyright (c) [2021] [zmmfly]
 * [Tiny-Msgpk] is licensed under the license file "LICENSE"
 * See the file "LICENSE" for more details.
 */
#include "tiny_msgpk.h"

static msgpk_port_t hooks = {
    .malloc  = malloc,
    .calloc  = calloc,
    .free    = free,
    .realloc = realloc
};

int msgpk_parse_next(msgpk_parse_t *parse)
{
    if (parse->idx_nxt < parse->pk->msgpk_sz) {
        parse->idx_cur = parse->idx_nxt;
        return 0;
    }
    return -1;
}

/**
 * @brief Get current index type and data
 * 
 * @param parse Parse struct pointer
 * @param dec   Decode struct pointer
 * @return int 
 */
int msgpk_parse_get(msgpk_parse_t *parse, msgpk_decode_t *dec)
{
#define MEMSZ_CHK(p, req) if ( ((p)->pk->msgpk_sz - (p)->idx_cur) < req ) return -1
    MSGPK_CHK(parse,-1);
    MSGPK_CHK(dec,-1);

    uint8_t *buf = parse->pk->msgpk_buf;
    dec->u64 = 0;

    uint8_t flag = buf[parse->idx_cur];
    switch (msgpk_parse_get_currnet_flag(parse))
    {
        case FMTF_PFIXINT:
            MEMSZ_CHK(parse, 1);
            dec->type_dec  = MSGPK_UINT8;
            dec->u64       = buf[parse->idx_cur] & 0x7f;
            parse->idx_nxt = parse->idx_cur + 1;
            break;

        case FMTF_FIXMAP:
            MEMSZ_CHK(parse, 1);
            dec->type_dec  = MSGPK_MAP;
            dec->length    = buf[parse->idx_cur] & 0x0f;
            parse->idx_nxt = parse->idx_cur + 1;
            break;

        case FMTF_FIXARR:
            MEMSZ_CHK(parse, 1);
            dec->type_dec  = MSGPK_ARR;
            dec->length    = buf[parse->idx_cur] & 0x0f;
            parse->idx_nxt = parse->idx_cur + 1;
            break;

        case FMTF_FIXSTR:
            MEMSZ_CHK(parse, 1);
            dec->type_dec  = MSGPK_STRING;
            dec->length    = buf[parse->idx_cur] & 0x1f;
            dec->str       = buf+parse->idx_cur+1;
            parse->idx_nxt = parse->idx_cur + 1 + dec->length;
            MEMSZ_CHK(parse, 1+dec->length);
            break;

        case FMTF_NIL:
            MEMSZ_CHK(parse, 1);
            dec->type_dec  = MSGPK_NIL;
            parse->idx_nxt = parse->idx_cur + 1;
            break;

        case FMTF_FALSE:
            MEMSZ_CHK(parse, 1);
            dec->type_dec  = MSGPK_BOOL;
            dec->boolean   = 0;
            parse->idx_nxt = parse->idx_cur + 1;
            break;

        case FMTF_TRUE:
            MEMSZ_CHK(parse, 1);
            dec->type_dec  = MSGPK_BOOL;
            dec->boolean   = 1;
            parse->idx_nxt = parse->idx_cur + 1;
            break;

        case FMTF_BIN8:
            MEMSZ_CHK(parse, 2);
            dec->type_dec  = MSGPK_BIN;
            dec->length    = buf[parse->idx_cur+1];
            dec->bin       = buf+parse->idx_cur+2;
            parse->idx_nxt = parse->idx_cur + 2 + dec->length;
            MEMSZ_CHK(parse, 2+dec->length);
            break;

        case FMTF_BIN16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_BIN;
            dec->length   = buf[parse->idx_cur+1];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+2];
            dec->bin      = buf+parse->idx_cur+3;
            parse->idx_nxt = parse->idx_cur + 3 + dec->length;
            MEMSZ_CHK(parse, 3+dec->length);
            break;

        case FMTF_BIN32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec  = MSGPK_BIN;
            dec->length    = buf[parse->idx_cur+1];
            dec->length  <<= 8;
            dec->length   |= buf[parse->idx_cur+2];
            dec->length  <<= 8;
            dec->length   |= buf[parse->idx_cur+3];
            dec->length  <<= 8;
            dec->length   |= buf[parse->idx_cur+4];
            dec->bin       = buf+parse->idx_cur+5;
            parse->idx_nxt = parse->idx_cur + 5 + dec->length;
            MEMSZ_CHK(parse, 5+dec->length);
            break;

        case FMTF_EXT8:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_EXT;
            dec->length   = buf[parse->idx_cur+1];
            dec->type_ext = buf[parse->idx_cur+2];
            dec->bin      = buf + parse->idx_cur+3;
            parse->idx_nxt = parse->idx_cur + 3 + dec->length;
            break;

        case FMTF_EXT16:
            MEMSZ_CHK(parse, 4);
            dec->type_dec = MSGPK_EXT;
            dec->length   = buf[parse->idx_cur+1];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+2];
            dec->type_ext = buf[parse->idx_cur+3];
            dec->bin      = buf + parse->idx_cur+4;
            parse->idx_nxt = parse->idx_cur + 4 + dec->length;
            break;

        case FMTF_EXT32:
            MEMSZ_CHK(parse, 6);
            dec->type_dec = MSGPK_EXT;
            dec->length   = buf[parse->idx_cur+1];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+2];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+3];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+4];
            dec->type_ext = buf[parse->idx_cur+5];
            dec->bin      = buf + parse->idx_cur+6;
            parse->idx_nxt = parse->idx_cur + 6 + dec->length;
            break;

        case FMTF_FLOAT32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_FLOAT32;
            dec->u32      = buf[parse->idx_cur+1];
            dec->u32    <<= 8;
            dec->u32     |= buf[parse->idx_cur+2];
            dec->u32    <<= 8;
            dec->u32     |= buf[parse->idx_cur+3];
            dec->u32    <<= 8;
            dec->u32     |= buf[parse->idx_cur+4];
            parse->idx_nxt = parse->idx_cur + 5;
            break;

        case FMTF_FLOAT64:
            MEMSZ_CHK(parse, 9);
            dec->type_dec = MSGPK_FLOAT64;
            dec->u64      = buf[parse->idx_cur+1];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+2];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+3];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+4];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+5];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+6];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+7];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+8];
            parse->idx_nxt = parse->idx_cur + 9;
            break;

        case FMTF_UINT8:
            MEMSZ_CHK(parse, 2);
            dec->type_dec = MSGPK_UINT8;
            dec->u8       = buf[parse->idx_cur+1];
            parse->idx_nxt = parse->idx_cur + 2;
            break;

        case FMTF_UINT16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_UINT16;
            dec->u16      = buf[parse->idx_cur+1];
            dec->u16    <<= 8;
            dec->u16     |= buf[parse->idx_cur+2];
            parse->idx_nxt = parse->idx_cur + 3;
            break;

        case FMTF_UINT32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_UINT32;
            dec->u32      = buf[parse->idx_cur+1];
            dec->u32    <<= 8;
            dec->u32     |= buf[parse->idx_cur+2];
            dec->u32    <<= 8;
            dec->u32     |= buf[parse->idx_cur+3];
            dec->u32    <<= 8;
            dec->u32     |= buf[parse->idx_cur+4];
            parse->idx_nxt = parse->idx_cur + 5;
            break;

        case FMTF_UINT64:
            MEMSZ_CHK(parse, 9);
            dec->type_dec = MSGPK_UINT64;
            dec->u64      = buf[parse->idx_cur+1];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+2];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+3];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+4];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+5];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+6];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+7];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+8];
            parse->idx_nxt = parse->idx_cur + 9;
            break;

        case FMTF_INT8:
            MEMSZ_CHK(parse, 2);
            dec->type_dec = MSGPK_INT8;
            dec->u8       = buf[parse->idx_cur+1];
            parse->idx_nxt = parse->idx_cur + 2;
            break;

        case FMTF_INT16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_INT16;
            dec->u16      = buf[parse->idx_cur+1];
            dec->u16    <<= 8;
            dec->u16     |= buf[parse->idx_cur+2];
            parse->idx_nxt = parse->idx_cur + 3;
            break;

        case FMTF_INT32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_INT32;
            dec->u32      = buf[parse->idx_cur+1];
            dec->u32    <<= 8;
            dec->u32     |= buf[parse->idx_cur+2];
            dec->u32    <<= 8;
            dec->u32     |= buf[parse->idx_cur+3];
            dec->u32    <<= 8;
            dec->u32     |= buf[parse->idx_cur+4];
            parse->idx_nxt = parse->idx_cur + 5;
            break;

        case FMTF_INT64:
            MEMSZ_CHK(parse, 9);
            dec->type_dec = MSGPK_INT64;
            dec->u64      = buf[parse->idx_cur+1];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+2];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+3];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+4];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+5];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+6];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+7];
            dec->u64    <<= 8;
            dec->u64     |= buf[parse->idx_cur+8];
            parse->idx_nxt = parse->idx_cur + 9;
            break;

        case FMTF_FIXEXT1:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_EXT;
            dec->length   = 1;
            dec->type_ext = buf[parse->idx_cur+1];
            dec->bin      = buf+parse->idx_cur+2;
            parse->idx_nxt = parse->idx_cur + 3;
            break;

        case FMTF_FIXEXT2:
            MEMSZ_CHK(parse, 4);
            dec->type_dec = MSGPK_EXT;
            dec->length   = 2;
            dec->type_ext = buf[parse->idx_cur+1];
            dec->bin      = buf+parse->idx_cur+2;
            parse->idx_nxt = parse->idx_cur + 4;
            break;

        case FMTF_FIXEXT4:
            MEMSZ_CHK(parse, 6);
            dec->type_dec = MSGPK_EXT;
            dec->length   = 4;
            dec->type_ext = buf[parse->idx_cur+1];
            dec->bin      = buf+parse->idx_cur+2;
            parse->idx_nxt = parse->idx_cur + 6;
            break;

        case FMTF_FIXEXT8:
            MEMSZ_CHK(parse, 10);
            dec->type_dec = MSGPK_EXT;
            dec->length   = 8;
            dec->type_ext = buf[parse->idx_cur+1];
            dec->bin      = buf+parse->idx_cur+2;
            parse->idx_nxt = parse->idx_cur + 10;
            break;

        case FMTF_FIXEXT16:
            MEMSZ_CHK(parse, 18);
            dec->type_dec = MSGPK_EXT;
            dec->length   = 16;
            dec->type_ext = buf[parse->idx_cur+1];
            dec->bin      = buf+parse->idx_cur+2;
            parse->idx_nxt = parse->idx_cur + 18;
            break;

        case FMTF_STR8:
            MEMSZ_CHK(parse, 2);
            dec->type_dec = MSGPK_STRING;
            dec->length   = buf[parse->idx_cur+1];
            dec->str      = buf+parse->idx_cur+2;
            parse->idx_nxt = parse->idx_cur + 2 + dec->length;
            MEMSZ_CHK(parse, 2+dec->length);
            break;

        case FMTF_STR16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_STRING;
            dec->length   = buf[parse->idx_cur+1];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+2];
            dec->str      = buf+parse->idx_cur+3;
            parse->idx_nxt = parse->idx_cur + 3 + dec->length;
            MEMSZ_CHK(parse, 3+dec->length);
            break;

        case FMTF_STR32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_STRING;
            dec->length   = buf[parse->idx_cur+1];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+2];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+3];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+4];
            dec->str      = buf+parse->idx_cur+5;
            parse->idx_nxt = parse->idx_cur + 5 + dec->length;
            MEMSZ_CHK(parse, 5+dec->length);
            break;

        case FMTF_ARR16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_ARR;
            dec->length   = buf[parse->idx_cur+1];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+2];
            parse->idx_nxt = parse->idx_cur + 3;
            break;

        case FMTF_ARR32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_ARR;
            dec->length   = buf[parse->idx_cur+1];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+2];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+3];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+4];
            parse->idx_nxt = parse->idx_cur + 5;
            break;

        case FMTF_MAP16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_MAP;
            dec->length   = buf[parse->idx_cur+1];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+2];
            parse->idx_nxt = parse->idx_cur + 3;
            break;

        case FMTF_MAP32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_MAP;
            dec->length   = buf[parse->idx_cur+1];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+2];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+3];
            dec->length <<= 8;
            dec->length  |= buf[parse->idx_cur+4];
            parse->idx_nxt = parse->idx_cur + 5;
            break;

        case FMTF_NFIXINT:
            MEMSZ_CHK(parse, 1);
            dec->type_dec = MSGPK_INT8;
            dec->i8 = buf[parse->idx_cur];
            parse->idx_nxt = parse->idx_cur + 1;
            break;

        default:
            break;
    }
    return 0;
}

uint8_t msgpk_parse_get_currnet_flag(msgpk_parse_t *parse)
{
    MSGPK_CHK(parse,-1);
    uint8_t flag = parse->pk->msgpk_buf[parse->idx_cur];

    if ( (flag & 0x80) == 0x00) {
        return FMTF_PFIXINT;

    } else if ( (flag & 0xf0) == FMTF_FIXMAP) {
        return FMTF_FIXMAP;

    } else if ( (flag & 0xf0) == FMTF_FIXARR) {
        return FMTF_FIXARR;

    } else if ( (flag & 0xe0) == FMTF_FIXSTR) {
        return FMTF_FIXSTR;

    } else if ( (flag & 0xe0) == FMTF_NFIXINT) {
        return FMTF_NFIXINT;

    }
    return flag;
}

/**
 * @brief Deinitialize parse struct
 * 
 * @param parse 
 * @return int 
 */
int msgpk_parse_deinit(msgpk_parse_t *parse)
{
    MSGPK_CHK(parse, -1);
    if (parse->pk) {
        hooks.free(parse->pk);
    }
    return 0;
}

/**
 * @brief Initialize parse struct
 * 
 * @param parse 
 * @param dat       Need keep until parse end
 * @param length    MessagePack length
 * @return int 
 */
int msgpk_parse_init(msgpk_parse_t *parse, uint8_t *dat, size_t length)
{
    MSGPK_CHK(parse, -1);
    parse->pk      = (msgpk_t *)hooks.calloc(1, sizeof(msgpk_t));
    MSGPK_CHK(parse->pk, -1);

    parse->pk->msgpk_buf = dat;
    parse->pk->msgpk_sz  = length;
    parse->idx_cur       = 0;
    parse->idx_nxt       = 0;
    return 0;
}

/**
 * @brief Add unsigned integer
 * 
 * @param msgpk 
 * @param dat 
 * @return int 
 */
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

/**
 * @brief Add signed integer
 * 
 * @param msgpk 
 * @param dat 
 * @return int 
 */
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

/**
 * @brief Add string
 * 
 * @param msgpk 
 * @param str string pointer
 * @param len string length
 * @return int 
 */
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

/**
 * @brief Add binary
 * 
 * @param msgpk 
 * @param dat bin pointer
 * @param len bin length
 * @return int 
 */
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

/**
 * @brief Add Ext
 * 
 * @param msgpk 
 * @param type Ext type
 * @param dat Ext data pointer
 * @param len Ext data length
 * @return int 
 */
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

/**
 * @brief Add array
 * 
 * @param msgpk 
 * @param num Array element number
 * @return int 
 */
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

/**
 * @brief Add map
 * 
 * @param msgpk 
 * @param num map pair number
 * @return int 
 */
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
    return 0;
}

int msgpk_add_map32(msgpk_t *msgpk, uint32_t num)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 5,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_MAP32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, num);
    msgpk->msgpk_sz += 5;

    return 0;
}

int msgpk_add_map16(msgpk_t *msgpk, uint16_t num)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_MAP16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, num);
    msgpk->msgpk_sz += 3;

    return 0;
}

int msgpk_add_arr32(msgpk_t *msgpk, uint16_t num)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 5,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_ARR32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, num);
    msgpk->msgpk_sz += 5;

    return 0;
}

int msgpk_add_arr16(msgpk_t *msgpk, uint16_t num)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_ARR16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, num);
    msgpk->msgpk_sz += 3;

    return 0;
}

int msgpk_add_str32(msgpk_t *msgpk, char *str, uint32_t len)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, len+5,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_STR32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+5, str, len);
    msgpk->msgpk_sz += (len+5);

    return 0;
}

int msgpk_add_str16(msgpk_t *msgpk, char *str, uint16_t len)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, len+3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_STR16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+3, str, len);
    msgpk->msgpk_sz += (len+3);

    return 0;
}

int msgpk_add_str8(msgpk_t *msgpk, char *str, uint8_t len)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, len+2,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_STR8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = len;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+2, str, len);
    msgpk->msgpk_sz += (len+2);

    return 0;
}

int msgpk_add_int64(msgpk_t *msgpk, int64_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 9,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT64;
    msgpk_wr_u64_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=9;

    return 0;
}

int msgpk_add_int32(msgpk_t *msgpk, int32_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 5,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=5;

    return 0;
}

int msgpk_add_int16(msgpk_t *msgpk, int16_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=3;

    return 0;
}

int msgpk_add_int8(msgpk_t *msgpk, int8_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 2,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat;
    msgpk->msgpk_sz += 2;
    return 0;
}

int msgpk_add_uint64(msgpk_t *msgpk, uint64_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 9,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT64;
    msgpk_wr_u64_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=9;

    return 0;
}

int msgpk_add_uint32(msgpk_t *msgpk, uint32_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 5,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=5;

    return 0;
}

int msgpk_add_uint16(msgpk_t *msgpk, uint16_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=3;

    return 0;
}

int msgpk_add_uint8(msgpk_t *msgpk, uint8_t dat)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 2,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat;
    msgpk->msgpk_sz += 2;
    return 0;
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
    msgpk_wr_u64_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat.f_u64);
    msgpk->msgpk_sz += 9;

    return 0;
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
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat.f_u32);
    msgpk->msgpk_sz += 5;

    return 0;
}

int msgpk_add_ext32(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint32_t len)
{
    MSGPK_CHK(msgpk,-1);
    if (type < 0)return -1;
    MSGPK_REQCHK(msgpk, len+6,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_EXT32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    msgpk->msgpk_buf[msgpk->msgpk_sz+5] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+6, dat, len);
    msgpk->msgpk_sz+= (len+6);

    return 0;
}

int msgpk_add_ext16(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint16_t len)
{
    MSGPK_CHK(msgpk,-1);
    if (type < 0)return -1;
    MSGPK_REQCHK(msgpk, len+4,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_EXT16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+4, dat, len);
    msgpk->msgpk_sz+= (len+4);

    return 0;
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

    return 0;
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

    return 0;
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

    return 0;
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

    return 0;
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

    return 0;
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

    return 0;
}

int msgpk_add_bin32(msgpk_t *msgpk, uint8_t *dat, uint32_t len)
{
    MSGPK_CHK(msgpk,-1);
    if ( (uint8_t)len > 32) return -1;

    MSGPK_REQCHK(msgpk, len+4,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]    = FMTF_BIN32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    msgpk->msgpk_buf[msgpk->msgpk_sz+1]  = len >> 24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2]  = len >> 16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3]  = len >> 8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4]  = len;
    msgpk->msgpk_sz                     += 4;

    memcpy(msgpk->msgpk_buf + msgpk->msgpk_sz, dat, len);
    msgpk->msgpk_sz+=len;

    return 0;
}

int msgpk_add_bin16(msgpk_t *msgpk, uint8_t *dat, uint16_t len)
{
    MSGPK_CHK(msgpk,-1);
    if ( (uint8_t)len > 32) return -1;

    MSGPK_REQCHK(msgpk, len+3,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz]    = FMTF_BIN16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    msgpk->msgpk_buf[msgpk->msgpk_sz+1]  = len >> 8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2]  = len;
    msgpk->msgpk_sz                     += 3;

    memcpy(msgpk->msgpk_buf + msgpk->msgpk_sz, dat, len);
    msgpk->msgpk_sz+=len;

    return 0;
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

    return 0;
}

int msgpk_add_true(msgpk_t *msgpk)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 1,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_TRUE;
    msgpk->msgpk_sz++;
    return 0;
}

int msgpk_add_false(msgpk_t *msgpk)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 1,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_FALSE;
    msgpk->msgpk_sz++;
    return 0;
}

int msgpk_add_nil(msgpk_t *msgpk)
{
    MSGPK_CHK(msgpk,-1);
    MSGPK_REQCHK(msgpk, 1,-1);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_NIL;
    msgpk->msgpk_sz++;
    return 0;
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

    return 0;
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

    return 0;
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

    return 0;
}

int msgpk_add_positive_fixint(msgpk_t *msgpk, int8_t num)
{
    MSGPK_CHK(msgpk,-1);
    if ( (uint8_t)num > 127) return -1;

    num &= 0x7f;

    MSGPK_REQCHK(msgpk, 1,-1);
    msgpk->msgpk_buf[msgpk->msgpk_sz] = num;
    msgpk->msgpk_sz++;
    return 0;
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

    return 0;
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
    return 0;
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

void msgpk_set_port(msgpk_port_t *port)
{
    if (port == NULL)return;
    if ( port->malloc != NULL ) hooks.malloc  = port->malloc;
    if ( port->calloc != NULL ) hooks.malloc  = port->calloc;
    if ( port->realloc != NULL ) hooks.malloc = port->realloc;
    if ( port->free != NULL ) hooks.malloc    = port->free;
}

#ifndef RDWR_INLINE
uint16_t msgpk_rd_u16_bigend(uint8_t *dat)
{
    uint16_t u16 = 0;
    MSGPK_CHK(dat, 0);
    u16   = *dat++;
    u16 <<= 8;
    u16  |= *dat;
    return u16;
}

uint32_t msgpk_rd_u32_bigend(uint8_t *dat)
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

uint64_t msgpk_rd_u64_bigend(uint8_t *dat)
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

void msgpk_wr_u16_bigend(uint8_t *dat, uint16_t u16)
{
    *dat++ = u16>>8;
    *dat   = u16;
}

void msgpk_wr_u32_bigend(uint8_t *dat, uint32_t u32)
{
    *dat++ = u32>>24;
    *dat++ = u32>>16;
    *dat++ = u32>>8;
    *dat   = u32;
}

void msgpk_wr_u64_bigend(uint8_t *dat, uint64_t u64)
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