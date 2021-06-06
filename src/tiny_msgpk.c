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
    MSGPK_CHK(parse,MSGPK_ERR);

    if (parse->idx_nxt < parse->pk->msgpk_sz) {
        parse->idx_cur = parse->idx_nxt;
        return MSGPK_OK;
    }
    return MSGPK_ERR;
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
    uint8_t buf[8];
#define MEMSZ_CHK(p, req) if ( ((p)->pk->msgpk_sz - (p)->idx_cur) < req ) return MSGPK_ERR

// only for multiple read
#define rd_with_bit(bits, dec_name)    \
    do{ \
        msgpk_parse_get_multi_bytes(parse, (bits / 8) , buf, 1); \
        dec->dec_name = msgpk_rd_u##bits##_bigend(buf); \
    }while(0)

    MSGPK_CHK(parse,MSGPK_ERR);
    MSGPK_CHK(dec,MSGPK_ERR);

    if (dec->str != NULL && parse->pk->msgpk_fd) {
        hooks.free(dec->str);
        dec->str = NULL;
    }

    // if (dec->bin != NULL && parse->pk->msgpk_fd) {
    //     hooks.free(dec->bin);
    //     dec->bin = NULL;
    // }

    // if (dec->ext != NULL && parse->pk->msgpk_fd) {
    //     hooks.free(dec->ext);
    //     dec->ext = NULL;
    // }

    // uint8_t *buf = parse->pk->msgpk_buf;
    dec->u64 = 0;
    memset(buf, 0, 8);

    // uint8_t flag = buf[parse->idx_cur];
    switch (msgpk_parse_get_currnet_flag(parse))
    {
        case FMTF_PFIXINT:
            MEMSZ_CHK(parse, 1);
            dec->type_dec  = MSGPK_UINT8;
            dec->u64       = msgpk_parse_get_currnet_byte(parse, 0) & 0x7f;
            parse->idx_nxt = parse->idx_cur + 1;
            break;

        case FMTF_FIXMAP:
            MEMSZ_CHK(parse, 1);
            dec->type_dec  = MSGPK_MAP;
            dec->length    = msgpk_parse_get_currnet_byte(parse, 0) & 0x0f;
            parse->idx_nxt = parse->idx_cur + 1;
            break;

        case FMTF_FIXARR:
            MEMSZ_CHK(parse, 1);
            dec->type_dec  = MSGPK_ARR;
            dec->length    = msgpk_parse_get_currnet_byte(parse, 0) & 0x0f;
            parse->idx_nxt = parse->idx_cur + 1;
            break;

        case FMTF_FIXSTR:
            MEMSZ_CHK(parse, 1);
            dec->type_dec  = MSGPK_STRING;
            dec->length    = msgpk_parse_get_currnet_byte(parse, 0) & 0x1f;
            // dec->str       = (char *)buf+parse->idx_cur+1;
            dec->str       = (char *)msgpk_parse_get_buf(parse, 1, dec->length);
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
            dec->length    = msgpk_parse_get_currnet_byte(parse, 1);
            dec->bin       = msgpk_parse_get_buf(parse, 2, dec->length);
            parse->idx_nxt = parse->idx_cur + 2 + dec->length;
            MEMSZ_CHK(parse, 2+dec->length);
            break;

        case FMTF_BIN16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_BIN;

            #if PARSE_RDFN
            rd_with_bit(16, length);
            #else
            dec->length   = msgpk_parse_get_currnet_byte(parse, 1);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 2);
            #endif

            dec->bin      = msgpk_parse_get_buf(parse, 3, dec->length);
            parse->idx_nxt = parse->idx_cur + 3 + dec->length;
            MEMSZ_CHK(parse, 3+dec->length);
            break;

        case FMTF_BIN32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec  = MSGPK_BIN;

            #if PARSE_RDFN
            rd_with_bit(32, length);
            #else
            dec->length    = msgpk_parse_get_currnet_byte(parse, 1);
            dec->length  <<= 8;
            dec->length   |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->length  <<= 8;
            dec->length   |= msgpk_parse_get_currnet_byte(parse, 3);
            dec->length  <<= 8;
            dec->length   |= msgpk_parse_get_currnet_byte(parse, 4);
            #endif

            dec->bin       = msgpk_parse_get_buf(parse, 5, dec->length);
            parse->idx_nxt = parse->idx_cur + 5 + dec->length;
            MEMSZ_CHK(parse, 5+dec->length);
            break;

        case FMTF_EXT8:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_EXT;
            dec->length   = msgpk_parse_get_currnet_byte(parse, 1);
            dec->type_ext = msgpk_parse_get_currnet_byte(parse, 2);
            dec->bin      = msgpk_parse_get_buf(parse, 3, dec->length);
            parse->idx_nxt = parse->idx_cur + 3 + dec->length;
            break;

        case FMTF_EXT16:
            MEMSZ_CHK(parse, 4);
            dec->type_dec = MSGPK_EXT;

            #if PARSE_RDFN
            rd_with_bit(16, length);
            #else
            dec->length   = msgpk_parse_get_currnet_byte(parse, 1);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 2);
            #endif

            dec->type_ext = msgpk_parse_get_currnet_byte(parse, 3);
            dec->bin      = msgpk_parse_get_buf(parse, 4, dec->length);
            parse->idx_nxt = parse->idx_cur + 4 + dec->length;
            break;

        case FMTF_EXT32:
            MEMSZ_CHK(parse, 6);
            dec->type_dec = MSGPK_EXT;

            #if PARSE_RDFN
            rd_with_bit(32, length);
            #else
            dec->length   = msgpk_parse_get_currnet_byte(parse, 1);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 3);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 4);
            #endif

            dec->type_ext = msgpk_parse_get_currnet_byte(parse, 5);
            dec->bin      = msgpk_parse_get_buf(parse, 6, dec->length);
            parse->idx_nxt = parse->idx_cur + 6 + dec->length;
            break;

        case FMTF_FLOAT32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_FLOAT32;

            #if PARSE_RDFN
            rd_with_bit(32, u32);
            #else
            dec->u32      = msgpk_parse_get_currnet_byte(parse, 1);
            dec->u32    <<= 8;
            dec->u32     |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->u32    <<= 8;
            dec->u32     |= msgpk_parse_get_currnet_byte(parse, 3);
            dec->u32    <<= 8;
            dec->u32     |= msgpk_parse_get_currnet_byte(parse, 4);
            #endif

            parse->idx_nxt = parse->idx_cur + 5;
            break;

        case FMTF_FLOAT64:
            MEMSZ_CHK(parse, 9);
            dec->type_dec = MSGPK_FLOAT64;

            #if PARSE_RDFN
            rd_with_bit(64, u64);
            #else
            dec->u64      = msgpk_parse_get_currnet_byte(parse, 1);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 3);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 4);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 5);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 6);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 7);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 8);
            #endif

            parse->idx_nxt = parse->idx_cur + 9;
            break;

        case FMTF_UINT8:
            MEMSZ_CHK(parse, 2);
            dec->type_dec = MSGPK_UINT8;
            dec->u8       = msgpk_parse_get_currnet_byte(parse, 1);
            parse->idx_nxt = parse->idx_cur + 2;
            break;

        case FMTF_UINT16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_UINT16;

            #if PARSE_RDFN
            rd_with_bit(16, u16);
            #else
            dec->u16      = msgpk_parse_get_currnet_byte(parse, 1);
            dec->u16    <<= 8;
            dec->u16     |= msgpk_parse_get_currnet_byte(parse, 2);
            #endif

            parse->idx_nxt = parse->idx_cur + 3;
            break;

        case FMTF_UINT32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_UINT32;

            #if PARSE_RDFN
            rd_with_bit(32, u32);
            #else
            dec->u32      = msgpk_parse_get_currnet_byte(parse, 1);
            dec->u32    <<= 8;
            dec->u32     |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->u32    <<= 8;
            dec->u32     |= msgpk_parse_get_currnet_byte(parse, 3);
            dec->u32    <<= 8;
            dec->u32     |= msgpk_parse_get_currnet_byte(parse, 4);
            #endif

            parse->idx_nxt = parse->idx_cur + 5;
            break;

        case FMTF_UINT64:
            MEMSZ_CHK(parse, 9);
            dec->type_dec = MSGPK_UINT64;

            #if PARSE_RDFN
            rd_with_bit(64, u64);
            #else
            dec->u64      = msgpk_parse_get_currnet_byte(parse, 1);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 3);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 4);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 5);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 6);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 7);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 8);
            #endif

            parse->idx_nxt = parse->idx_cur + 9;
            break;

        case FMTF_INT8:
            MEMSZ_CHK(parse, 2);
            dec->type_dec = MSGPK_INT8;
            dec->u8       = msgpk_parse_get_currnet_byte(parse, 1);
            parse->idx_nxt = parse->idx_cur + 2;
            break;

        case FMTF_INT16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_INT16;

            #if PARSE_RDFN
            rd_with_bit(16, u16);
            #else
            dec->u16      = msgpk_parse_get_currnet_byte(parse, 1);
            dec->u16    <<= 8;
            dec->u16     |= msgpk_parse_get_currnet_byte(parse, 2);
            #endif

            parse->idx_nxt = parse->idx_cur + 3;
            break;

        case FMTF_INT32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_INT32;

            #if PARSE_RDFN
            rd_with_bit(32, u32);
            #else
            dec->u32      = msgpk_parse_get_currnet_byte(parse, 1);
            dec->u32    <<= 8;
            dec->u32     |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->u32    <<= 8;
            dec->u32     |= msgpk_parse_get_currnet_byte(parse, 3);
            dec->u32    <<= 8;
            dec->u32     |= msgpk_parse_get_currnet_byte(parse, 4);
            #endif

            parse->idx_nxt = parse->idx_cur + 5;
            break;

        case FMTF_INT64:
            MEMSZ_CHK(parse, 9);
            dec->type_dec = MSGPK_INT64;

            #if PARSE_RDFN
            rd_with_bit(64, u64);
            #else
            dec->u64      = msgpk_parse_get_currnet_byte(parse, 1);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 3);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 4);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 5);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 6);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 7);
            dec->u64    <<= 8;
            dec->u64     |= msgpk_parse_get_currnet_byte(parse, 8);
            #endif

            parse->idx_nxt = parse->idx_cur + 9;
            break;

        case FMTF_FIXEXT1:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_EXT;
            dec->length   = 1;
            dec->type_ext = msgpk_parse_get_currnet_byte(parse, 1);
            dec->bin      = msgpk_parse_get_buf(parse, 2, dec->length);
            parse->idx_nxt = parse->idx_cur + 3;
            break;


        case FMTF_FIXEXT2:
            MEMSZ_CHK(parse, 4);
            dec->type_dec = MSGPK_EXT;
            dec->length   = 2;
            dec->type_ext = msgpk_parse_get_currnet_byte(parse, 1);
            dec->bin      = msgpk_parse_get_buf(parse, 2, dec->length);
            parse->idx_nxt = parse->idx_cur + 4;
            break;

        case FMTF_FIXEXT4:
            MEMSZ_CHK(parse, 6);
            dec->type_dec = MSGPK_EXT;
            dec->length   = 4;
            dec->type_ext = msgpk_parse_get_currnet_byte(parse, 1);
            dec->bin      = msgpk_parse_get_buf(parse, 2, dec->length);
            parse->idx_nxt = parse->idx_cur + 6;
            break;

        case FMTF_FIXEXT8:
            MEMSZ_CHK(parse, 10);
            dec->type_dec = MSGPK_EXT;
            dec->length   = 8;
            dec->type_ext = msgpk_parse_get_currnet_byte(parse, 1);
            dec->bin      = msgpk_parse_get_buf(parse, 2, dec->length);
            parse->idx_nxt = parse->idx_cur + 10;
            break;

        case FMTF_FIXEXT16:
            MEMSZ_CHK(parse, 18);
            dec->type_dec = MSGPK_EXT;
            dec->length   = 16;
            dec->type_ext = msgpk_parse_get_currnet_byte(parse, 1);
            dec->bin      = msgpk_parse_get_buf(parse, 2, dec->length);
            parse->idx_nxt = parse->idx_cur + 18;
            break;

        case FMTF_STR8:
            MEMSZ_CHK(parse, 2);
            dec->type_dec = MSGPK_STRING;
            dec->length   = msgpk_parse_get_currnet_byte(parse, 1);
            dec->str      = (char *)msgpk_parse_get_buf(parse, 2, dec->length);
            parse->idx_nxt = parse->idx_cur + 2 + dec->length;
            MEMSZ_CHK(parse, 2+dec->length);
            break;

        case FMTF_STR16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_STRING;
            dec->length   = msgpk_parse_get_currnet_byte(parse, 1);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->str      = (char *)msgpk_parse_get_buf(parse, 3, dec->length);
            parse->idx_nxt = parse->idx_cur + 3 + dec->length;
            MEMSZ_CHK(parse, 3+dec->length);
            break;

        case FMTF_STR32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_STRING;

            #if PARSE_RDFN
            rd_with_bit(32, length);
            #else
            dec->length   = msgpk_parse_get_currnet_byte(parse, 1);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 3);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 4);
            #endif

            dec->str      = (char *)msgpk_parse_get_buf(parse, 5, dec->length);
            parse->idx_nxt = parse->idx_cur + 5 + dec->length;
            MEMSZ_CHK(parse, 5+dec->length);
            break;

        case FMTF_ARR16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_ARR;

            #if PARSE_RDFN
            rd_with_bit(16, length);
            #else
            dec->length   = msgpk_parse_get_currnet_byte(parse, 1);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 2);
            #endif

            parse->idx_nxt = parse->idx_cur + 3;
            break;

        case FMTF_ARR32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_ARR;

            #if PARSE_RDFN
            rd_with_bit(32, length);
            #else
            dec->length   = msgpk_parse_get_currnet_byte(parse, 1);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 3);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 4);
            #endif

            parse->idx_nxt = parse->idx_cur + 5;
            break;

        case FMTF_MAP16:
            MEMSZ_CHK(parse, 3);
            dec->type_dec = MSGPK_MAP;

            #if PARSE_RDFN
            rd_with_bit(16, length);
            #else
            dec->length   = msgpk_parse_get_currnet_byte(parse, 1);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 2);
            #endif

            parse->idx_nxt = parse->idx_cur + 3;
            break;

        case FMTF_MAP32:
            MEMSZ_CHK(parse, 5);
            dec->type_dec = MSGPK_MAP;

            #if PARSE_RDFN
            rd_with_bit(32, length);
            #else
            dec->length   = msgpk_parse_get_currnet_byte(parse, 1);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 2);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 3);
            dec->length <<= 8;
            dec->length  |= msgpk_parse_get_currnet_byte(parse, 4);
            #endif

            parse->idx_nxt = parse->idx_cur + 5;
            break;

        case FMTF_NFIXINT:
            MEMSZ_CHK(parse, 1);
            dec->type_dec = MSGPK_INT8;
            dec->i8 = msgpk_parse_get_currnet_byte(parse, 0);
            parse->idx_nxt = parse->idx_cur + 1;
            break;

        default:
            break;
    }
    return MSGPK_OK;
#undef rd_with_bit
#undef MEMSZ_CHK
}

/**
 * @brief Read multiple bytes from buf or file
 * 
 * @param parse 
 * @param sz 
 * @param buf 
 * @param offset
 * @return int8_t 
 */
int8_t msgpk_parse_get_multi_bytes(msgpk_parse_t *parse, size_t sz, uint8_t *buf, int8_t offset)
{
    int8_t ret = MSGPK_ERR;
    size_t rd = 0;
    if (parse == NULL || buf == NULL || sz == 0) return ret;

    #if FILE_ENABLE
    if (parse->pk->msgpk_fd != NULL) {
        // check overflow
        if (parse->idx_cur + sz > parse->pk->msgpk_sz) return MSGPK_ERR_OF;
        fseek(parse->pk->msgpk_fd, offset, SEEK_CUR);
        rd = fread(buf, 1, sz, parse->pk->msgpk_fd);
        ret = (rd != sz) ? MSGPK_ERR_RDSZ : MSGPK_OK;
        fseek(parse->pk->msgpk_fd, -1 *rd + -1 * offset, SEEK_CUR);
        return ret;
    }
    #endif

    // check overflow
    if (parse->idx_cur+sz+offset > parse->pk->msgpk_sz) return MSGPK_ERR_OF;
    memcpy(buf, parse->pk->msgpk_buf + parse->idx_cur + offset, sz);
    return MSGPK_OK;
}

/**
 * @brief Read a byte from parse buf
 * 
 * @param parse 
 * @param offset Offset from current position 
 * @return uint8_t 
 */
uint8_t msgpk_parse_get_currnet_byte(msgpk_parse_t *parse, size_t offset)
{
    uint8_t ch = 0xff;
    MSGPK_CHK(parse, ch);

    #if FILE_ENABLE == 1
    if (parse->pk->msgpk_fd != NULL) {
        fseek(parse->pk->msgpk_fd, parse->idx_cur + offset, SEEK_SET);
        fread(&ch, 1, 1, parse->pk->msgpk_fd);
        fseek(parse->pk->msgpk_fd, parse->idx_cur, SEEK_SET);
        return ch;
    }
    #endif

    return parse->pk->msgpk_buf[parse->idx_cur + offset];
}

/**
 * @brief Get buffer from memory or file
 * 
 * @param parse 
 * @param offset Offset from buffer or file, it's will add cur_idx
 * @param length Only for file parse
 * @return uint8_t* In memory, it's pointer to source buffer with offset, In file, it's
 *      allocate new memory to store, and release in next get or deinit_file
 */
uint8_t *msgpk_parse_get_buf(msgpk_parse_t *parse, size_t offset, size_t length)
{
    uint8_t *buf = NULL;
    MSGPK_CHK(parse, NULL);

    #if FILE_ENABLE == 1
    if (parse->pk->msgpk_fd) {
        buf = (uint8_t *)malloc(length);
        if (buf == NULL)return NULL;

        fseek(parse->pk->msgpk_fd, parse->idx_cur + offset, SEEK_SET);
        fread(buf, 1, length, parse->pk->msgpk_fd);
        fseek(parse->pk->msgpk_fd, parse->idx_cur, SEEK_SET);

        return buf;
    }
    #endif

    // check range
    if ( (parse->idx_cur + offset + length) > parse->pk->msgpk_sz )return NULL;
    buf = parse->pk->msgpk_buf + parse->idx_cur + offset;
    return buf;
}

uint8_t msgpk_parse_get_currnet_flag(msgpk_parse_t *parse)
{
    MSGPK_CHK(parse, 0xff);
    // uint8_t flag = parse->pk->msgpk_buf[parse->idx_cur];
    uint8_t flag = msgpk_parse_get_currnet_byte(parse, 0);

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
    MSGPK_CHK(parse, MSGPK_ERR);
    if (parse->pk) {
        hooks.free(parse->pk);
    }

    #if FILE_ENABLE
    if (parse->pk->msgpk_fd) {
        fclose(parse->pk->msgpk_fd);
    }
    #endif
    return MSGPK_OK;
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
    MSGPK_CHK(parse, MSGPK_ERR);
    parse->pk      = (msgpk_t *)hooks.calloc(1, sizeof(msgpk_t));
    MSGPK_CHK(parse->pk, MSGPK_ERR);

    parse->pk->msgpk_fd  = NULL;
    parse->pk->msgpk_buf = dat;
    parse->pk->msgpk_sz  = length;
    parse->idx_cur       = 0;
    parse->idx_nxt       = 0;
    return MSGPK_OK;
}


#if FILE_ENABLE

/**
 * @brief Initlialize file parse
 * 
 * @param parse 
 * @param decoded Decoded struct to save result
 * @param file_path 
 * @return int 
 */
#if PARSE_INSIDE
int msgpk_parse_init_file(msgpk_parse_t *parse, const char *file_path)
#else
int msgpk_parse_init_file(msgpk_parse_t *parse, FILE *fd)
#endif
{
    MSGPK_CHK(parse, MSGPK_ERR);
    // MSGPK_CHK(decoded, MSGPK_ERR);
    #if PARSE_INSIDE
    MSGPK_CHK(file_path, MSGPK_ERR);
    #else
    MSGPK_CHK(fd, MSGPK_ERR);
    #endif

    // memset(decoded, 0, sizeof(msgpk_decode_t));

    parse->pk      = (msgpk_t *)hooks.calloc(1, sizeof(msgpk_t));
    MSGPK_CHK(parse->pk, MSGPK_ERR);

    #if PARSE_INSIDE
    parse->pk->msgpk_fd = fopen(file_path, "rb");
    if (parse->pk->msgpk_fd == NULL)
    {
        free(parse->pk);
        return MSGPK_ERR;
    }
    #else
    parse->pk->msgpk_fd = fd;
    #endif

    parse->idx_cur = 0;
    parse->idx_nxt = 0;

    fseek(parse->pk->msgpk_fd, 0, SEEK_END);
    parse->pk->msgpk_sz = ftell(parse->pk->msgpk_fd);
    fseek(parse->pk->msgpk_fd, 0, SEEK_SET);
    return MSGPK_OK;
}

/**
 * @brief File parse deinit
 * 
 * @param parse 
 * @return int 
 */
int msgpk_parse_deinit_file(msgpk_parse_t *parse, msgpk_decode_t *decoded)
{
    MSGPK_CHK(parse, MSGPK_ERR);
    MSGPK_CHK(decoded, MSGPK_ERR);

    if (parse->pk) {
        if (parse->pk->msgpk_fd) {
            fclose(parse->pk->msgpk_fd);
        }

        hooks.free(parse->pk);
    }

    if (decoded->str != NULL) {
        hooks.free(decoded->str);
        decoded->str = NULL;
    }

    if (decoded->bin != NULL) {
        hooks.free(decoded->bin);
        decoded->bin = NULL;
    }

    if (decoded->ext != NULL) {
        hooks.free(decoded->ext);
        decoded->ext = NULL;
    }
    return MSGPK_OK;
}

#endif

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
    return MSGPK_ERR;
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
    return MSGPK_ERR;
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
    return MSGPK_ERR;
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
    return MSGPK_ERR;
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
    return MSGPK_ERR;
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
    return MSGPK_ERR;
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
    return MSGPK_ERR;
}

int msgpk_add_negative_fixint(msgpk_t *msgpk, int8_t num)
{
    uint8_t dat = 0;
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (num < -31)return MSGPK_ERR;

    #if FILE_ENABLE

    dat = num & 0x1f;
    dat |= 0xe0;

    return msgpk_write(msgpk, &dat, 1);
    #else
    MSGPK_REQCHK(msgpk, 1, MSGPK_ERR);

    dat = num & 0x1f;
    dat |= 0xe0;

    msgpk->msgpk_buf[msgpk->msgpk_sz] = dat;
    msgpk->msgpk_sz += 1;
    return MSGPK_OK;
    #endif
}

int msgpk_add_map32(msgpk_t *msgpk, uint32_t num)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_MAP32;
    uint8_t buf[4];

    msgpk_wr_u32_bigend(buf, num);
    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 4);

    #else
    MSGPK_REQCHK(msgpk, 5,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_MAP32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, num);
    msgpk->msgpk_sz += 5;

    return MSGPK_OK;
    #endif
}

int msgpk_add_map16(msgpk_t *msgpk, uint16_t num)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_MAP16;
    uint8_t buf[2] = {0,0};

    msgpk_wr_u16_bigend(buf, num);
    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 2);

    #else
    MSGPK_REQCHK(msgpk, 3,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_MAP16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, num);
    msgpk->msgpk_sz += 3;

    return MSGPK_OK;
    #endif
}

int msgpk_add_arr32(msgpk_t *msgpk, uint32_t num)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_ARR32;
    uint8_t buf[4] = {0,0,0,0};

    msgpk_wr_u32_bigend(buf, num);
    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 4);

    #else
    MSGPK_REQCHK(msgpk, 5,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_ARR32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, num);
    msgpk->msgpk_sz += 5;

    return MSGPK_OK;
    #endif
}

int msgpk_add_arr16(msgpk_t *msgpk, uint16_t num)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_ARR16;
    uint8_t buf[2] = {0,0};

    msgpk_wr_u16_bigend(buf, num);
    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 2);
    #else
    MSGPK_REQCHK(msgpk, 3,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_ARR16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, num);
    msgpk->msgpk_sz += 3;

    return MSGPK_OK;
    #endif
}

int msgpk_add_str32(msgpk_t *msgpk, char *str, uint32_t len)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_STR32;
    uint8_t buf[4];

    msgpk_wr_u32_bigend(buf, len);
    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 4);
    #else
    MSGPK_REQCHK(msgpk, len+5,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_STR32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+5, str, len);
    msgpk->msgpk_sz += (len+5);

    return MSGPK_OK;
    #endif
}

int msgpk_add_str16(msgpk_t *msgpk, char *str, uint16_t len)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_STR16;
    uint8_t buf[2] = {0,0};
    msgpk_wr_u16_bigend(buf, len);

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, buf, 2);
    return msgpk_write(msgpk, (uint8_t *)str, len);
    #else
    MSGPK_REQCHK(msgpk, len+3,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_STR16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+3, str, len);
    msgpk->msgpk_sz += (len+3);

    return MSGPK_OK;
    #endif
}

int msgpk_add_str8(msgpk_t *msgpk, char *str, uint8_t len)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_STR8;

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, &len, 1);
    return msgpk_write(msgpk, str, len);
    #else
    MSGPK_REQCHK(msgpk, len+2,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_STR8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = len;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+2, str, len);
    msgpk->msgpk_sz += (len+2);

    return MSGPK_OK;
    #endif
}

int msgpk_add_int64(msgpk_t *msgpk, int64_t dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_INT64;
    uint8_t buf[8];
    msgpk_wr_u64_bigend(buf, dat);

    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 8);
    #else
    MSGPK_REQCHK(msgpk, 9,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT64;
    msgpk_wr_u64_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=9;

    return MSGPK_OK;
    #endif
}

int msgpk_add_int32(msgpk_t *msgpk, int32_t dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_INT32;
    uint8_t buf[4];

    msgpk_wr_u32_bigend(buf, dat);

    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 4);
    #else
    MSGPK_REQCHK(msgpk, 5,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=5;

    return MSGPK_OK;
    #endif
}

int msgpk_add_int16(msgpk_t *msgpk, int16_t dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_INT16;
    uint8_t buf[2];

    msgpk_wr_u16_bigend(buf, dat);
    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 2);
    #else
    MSGPK_REQCHK(msgpk, 3,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=3;

    return MSGPK_OK;
    #endif
}

int msgpk_add_int8(msgpk_t *msgpk, int8_t dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_INT8;

    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, &dat, 1);
    #else
    MSGPK_REQCHK(msgpk, 2,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_INT8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat;
    msgpk->msgpk_sz += 2;
    return MSGPK_OK;
    #endif
}

int msgpk_add_uint64(msgpk_t *msgpk, uint64_t dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_UINT64;
    uint8_t buf[8];

    msgpk_wr_u64_bigend(buf, dat);

    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 8);
    #else
    MSGPK_REQCHK(msgpk, 9,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT64;
    msgpk_wr_u64_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=9;

    return MSGPK_OK;
    #endif
}

int msgpk_add_uint32(msgpk_t *msgpk, uint32_t dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_UINT32;
    uint8_t buf[4];

    msgpk_wr_u32_bigend(buf, dat);
    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 4);
    #else
    MSGPK_REQCHK(msgpk, 5,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=5;

    return MSGPK_OK;
    #endif
}

int msgpk_add_uint16(msgpk_t *msgpk, uint16_t dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_UINT16;
    uint8_t buf[2];

    msgpk_wr_u16_bigend(buf, dat);
    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 2);
    #else
    MSGPK_REQCHK(msgpk, 3,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat);
    msgpk->msgpk_sz+=3;

    return MSGPK_OK;
    #endif
}

int msgpk_add_uint8(msgpk_t *msgpk, uint8_t dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_UINT8;

    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, &dat, 1);
    #else
    MSGPK_REQCHK(msgpk, 2,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_UINT8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = dat;
    msgpk->msgpk_sz += 2;
    return MSGPK_OK;
    #endif
}

int msgpk_add_float64(msgpk_t *msgpk, double f)
{
    union{
        uint64_t f_u64;
        double f_f;
    }dat = {
        .f_f = f
    };

    MSGPK_CHK(msgpk,MSGPK_ERR);

    #if FILE_ENABLE
    uint8_t ch = FMTF_FLOAT64;
    uint8_t buf[8];

    msgpk_wr_u64_bigend(buf, dat.f_u64);
    msgpk_write(msgpk, &ch ,1);
    return msgpk_write(msgpk, buf, 8);

    #else

    MSGPK_REQCHK(msgpk, 9,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_FLOAT64;
    msgpk_wr_u64_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat.f_u64);
    msgpk->msgpk_sz += 9;

    return MSGPK_OK;
    #endif
}

int msgpk_add_float32(msgpk_t *msgpk, float f)
{
    union{
        uint32_t f_u32;
        float f_f;
    }dat = {
        .f_f = f
    };
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_FLOAT32;
    uint8_t buf[4];

    msgpk_wr_u32_bigend(buf, dat.f_u32);

    msgpk_write(msgpk, &ch, 1);
    return msgpk_write(msgpk, buf, 4);

    #else
    MSGPK_REQCHK(msgpk, 5,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_FLOAT32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, dat.f_u32);
    msgpk->msgpk_sz += 5;

    return MSGPK_OK;
    #endif
}

int msgpk_add_ext32(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint32_t len)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (type < 0)return MSGPK_ERR;

    #if FILE_ENABLE
    uint8_t ch = FMTF_EXT32;
    uint8_t buf[4];

    msgpk_wr_u32_bigend(buf, len);

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, buf, 4);
    msgpk_write(msgpk, &type, 1);
    return msgpk_write(msgpk, dat, len);
    #else
    MSGPK_REQCHK(msgpk, len+6,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_EXT32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    msgpk->msgpk_buf[msgpk->msgpk_sz+5] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+6, dat, len);
    msgpk->msgpk_sz+= (len+6);

    return MSGPK_OK;
    #endif
}

int msgpk_add_ext16(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint16_t len)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (type < 0)return MSGPK_ERR;
    #if FILE_ENABLE
    uint8_t ch = FMTF_EXT16;
    uint8_t buf[2];

    msgpk_wr_u16_bigend(buf, len);

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, buf, 2);
    return msgpk_write(msgpk, dat, len);
    #else
    MSGPK_REQCHK(msgpk, len+4,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_EXT16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+4, dat, len);
    msgpk->msgpk_sz+= (len+4);

    return MSGPK_OK;
    #endif
}

int msgpk_add_ext8(msgpk_t *msgpk, int8_t type, uint8_t *dat, uint8_t len)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (type < 0)return MSGPK_ERR;
    #if FILE_ENABLE
    uint8_t ch = FMTF_EXT8;
    
    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, &len, 1);
    msgpk_write(msgpk, &type, 1);
    return msgpk_write(msgpk, dat, len);
    #else
    MSGPK_REQCHK(msgpk, len+3,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_EXT8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = len;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+3, dat, len);
    msgpk->msgpk_sz+= (len+3);

    return MSGPK_OK;
    #endif
}

/**
 * @brief Add fixed ext 16
 * 
 * @param msgpk 
 * @param type 
 * @param dat Make sure that had 16 bytes
 * @return int 
 */
int msgpk_add_fixext16(msgpk_t *msgpk, int8_t type, uint8_t *dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (type < 0)return MSGPK_ERR;

    #if FILE_ENABLE
    uint8_t ch = FMTF_FIXEXT16;

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, &type, 1);
    return msgpk_write(msgpk, dat, 16);

    #else
    MSGPK_REQCHK(msgpk, 18,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_FIXEXT16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+2, dat, 16);
    msgpk->msgpk_sz+=18;

    return MSGPK_OK;
    #endif
}

/**
 * @brief Add fixed ext 8
 * 
 * @param msgpk 
 * @param type 
 * @param dat Make sure that had 8 bytes
 * @return int 
 */
int msgpk_add_fixext8(msgpk_t *msgpk, int8_t type, uint8_t *dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (type < 0)return MSGPK_ERR;

    #if FILE_ENABLE

    uint8_t ch = FMTF_FIXEXT8;

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, &type, 1);
    return msgpk_write(msgpk, dat, 8);

    #else
    MSGPK_REQCHK(msgpk, 10,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_FIXEXT8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+2, dat, 8);
    msgpk->msgpk_sz+=10;

    return MSGPK_OK;
    #endif
}

/**
 * @brief Add fixed ext 4
 * 
 * @param msgpk 
 * @param type 
 * @param dat Make sure that had 4 bytes
 * @return int 
 */
int msgpk_add_fixext4(msgpk_t *msgpk, int8_t type, uint8_t *dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (type < 0)return MSGPK_ERR;
    #if FILE_ENABLE

    uint8_t ch = FMTF_FIXEXT4;

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, &type, 1);
    return msgpk_write(msgpk, dat, 4);

    #else

    MSGPK_REQCHK(msgpk, 6,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_FIXEXT4;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = type;
    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz+2, dat, 4);
    msgpk->msgpk_sz+=6;

    return MSGPK_OK;
    #endif
}

/**
 * @brief Add fixed length ext
 * 
 * @param msgpk 
 * @param type 
 * @param dat Make sure that had 2 bytes
 * @return int 
 */
int msgpk_add_fixext2(msgpk_t *msgpk, int8_t type, uint8_t *dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (type < 0)return MSGPK_ERR;
    #if FILE_ENABLE

    uint8_t ch = FMTF_FIXEXT2;

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, &type, 1);
    return msgpk_write(msgpk, dat, 2);

    #else
    
    MSGPK_REQCHK(msgpk, 4,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_FIXEXT2;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = type;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = *dat++;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3] = *dat;
    msgpk->msgpk_sz+=4;

    return MSGPK_OK;
    #endif
}

/**
 * @brief Add fixed ext 1
 * 
 * @param msgpk 
 * @param type 
 * @param dat Make sure that had 1 byte
 * @return int 
 */
int msgpk_add_fixext1(msgpk_t *msgpk, int8_t type, uint8_t *dat)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (type < 0)return MSGPK_ERR;
    #if FILE_ENABLE

    uint8_t ch = FMTF_FIXEXT1;

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, &type, 1);
    return msgpk_write(msgpk, dat, 1);

    #else
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (type < 0)return MSGPK_ERR;
    MSGPK_REQCHK(msgpk, 3,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]   = FMTF_FIXEXT1;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1] = type;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2] = *dat;
    msgpk->msgpk_sz+=3;

    return MSGPK_OK;
    #endif
}

/**
 * @brief Add bin with 32bit length
 * 
 * @param msgpk 
 * @param dat 
 * @param len 
 * @return int 
 */
int msgpk_add_bin32(msgpk_t *msgpk, uint8_t *dat, uint32_t len)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_BIN32;
    uint8_t buf[4];

    msgpk_wr_u32_bigend(buf, len);

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, buf, 4);
    return msgpk_write(msgpk, dat, len);

    #else
    // if ( (uint8_t)len > 32) return MSGPK_ERR;

    MSGPK_REQCHK(msgpk, len+4,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]    = FMTF_BIN32;
    msgpk_wr_u32_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    msgpk->msgpk_buf[msgpk->msgpk_sz+1]  = len >> 24;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2]  = len >> 16;
    msgpk->msgpk_buf[msgpk->msgpk_sz+3]  = len >> 8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+4]  = len;
    msgpk->msgpk_sz                     += 4;

    memcpy(msgpk->msgpk_buf + msgpk->msgpk_sz, dat, len);
    msgpk->msgpk_sz+=len;

    return MSGPK_OK;
    #endif
}

int msgpk_add_bin16(msgpk_t *msgpk, uint8_t *dat, uint16_t len)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_BIN16;
    uint8_t buf[2];

    msgpk_wr_u16_bigend(buf, len);

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, buf, 2);
    return msgpk_write(msgpk, dat, len);

    #else
    // if ( (uint8_t)len > 32) return MSGPK_ERR;

    MSGPK_REQCHK(msgpk, len+3,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]    = FMTF_BIN16;
    msgpk_wr_u16_bigend(msgpk->msgpk_buf+msgpk->msgpk_sz+1, len);
    msgpk->msgpk_buf[msgpk->msgpk_sz+1]  = len >> 8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+2]  = len;
    msgpk->msgpk_sz                     += 3;

    memcpy(msgpk->msgpk_buf + msgpk->msgpk_sz, dat, len);
    msgpk->msgpk_sz+=len;

    return MSGPK_OK;
    #endif
}

int msgpk_add_bin8(msgpk_t *msgpk, uint8_t *dat, uint8_t len)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_BIN8;

    msgpk_write(msgpk, &ch, 1);
    msgpk_write(msgpk, &len, 4);
    return msgpk_write(msgpk, dat, len);

    #else
    MSGPK_CHK(msgpk,MSGPK_ERR);
    // if ( (uint8_t)len > 32) return MSGPK_ERR;

    MSGPK_REQCHK(msgpk, len+2,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz]    = FMTF_BIN8;
    msgpk->msgpk_buf[msgpk->msgpk_sz+1]  = len;
    msgpk->msgpk_sz                     += 2;

    memcpy(msgpk->msgpk_buf + msgpk->msgpk_sz, dat, len);
    msgpk->msgpk_sz+=len;

    return MSGPK_OK;
    #endif
}

int msgpk_add_true(msgpk_t *msgpk)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_TRUE;

    return msgpk_write(msgpk, &ch, 1);
    #else
    MSGPK_REQCHK(msgpk, 1,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_TRUE;
    msgpk->msgpk_sz++;
    return MSGPK_OK;
    #endif
}

int msgpk_add_false(msgpk_t *msgpk)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_FALSE;

    return msgpk_write(msgpk, &ch, 1);
    #else
    MSGPK_REQCHK(msgpk, 1,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_FALSE;
    msgpk->msgpk_sz++;
    return MSGPK_OK;
    #endif
}

int msgpk_add_nil(msgpk_t *msgpk)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    #if FILE_ENABLE
    uint8_t ch = FMTF_NIL;

    return msgpk_write(msgpk, &ch, 1);
    #else
    MSGPK_REQCHK(msgpk, 1,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = FMTF_NIL;
    msgpk->msgpk_sz++;
    return MSGPK_OK;
    #endif
}

int msgpk_add_fixstr(msgpk_t *msgpk, char *str, uint8_t len)
{
    uint8_t hdr = 0;
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if ( len > 32) return MSGPK_ERR;

    hdr = len & ~FMTM_FIXSTR;
    hdr |= 0xa0;

    #if FILE_ENABLE
    msgpk_write(msgpk, &hdr, 1);
    return msgpk_write(msgpk, str, len);

    #else

    MSGPK_REQCHK(msgpk, len+1,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = hdr;
    msgpk->msgpk_sz++;
    memcpy(msgpk->msgpk_buf + msgpk->msgpk_sz, str, len);
    msgpk->msgpk_sz+=len;

    return MSGPK_OK;
    #endif
}

int msgpk_add_fixarr(msgpk_t *msgpk, uint8_t num)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if ( (uint8_t)num > 16) return MSGPK_ERR;

    num &= 0x0F;
    num |= 0x90;

    #if FILE_ENABLE
    return msgpk_write(msgpk, &num, 1);
    #else

    MSGPK_REQCHK(msgpk, 1,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = num;
    msgpk->msgpk_sz++;

    return MSGPK_OK;
    #endif
}

int msgpk_add_fixmap(msgpk_t *msgpk, uint8_t num)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if ( (uint8_t)num > 15) return MSGPK_ERR;

    num &= 0x0F;
    num |= 0x80;
    #if FILE_ENABLE

    return msgpk_write(msgpk, &num, 1);
    #else

    MSGPK_REQCHK(msgpk, 1,MSGPK_ERR);

    msgpk->msgpk_buf[msgpk->msgpk_sz] = num;
    msgpk->msgpk_sz++;

    return MSGPK_OK;
    #endif
}

int msgpk_add_positive_fixint(msgpk_t *msgpk, int8_t num)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if ( (uint8_t)num > 127) return MSGPK_ERR;

    num &= 0x7f;
    #if FILE_ENABLE

    return msgpk_write(msgpk, &num, 1);
    #else


    MSGPK_REQCHK(msgpk, 1,MSGPK_ERR);
    msgpk->msgpk_buf[msgpk->msgpk_sz] = num;
    msgpk->msgpk_sz++;
    return MSGPK_OK;
    #endif
}

/**
 * @brief Write to buffer or file handle
 * 
 * @param msgpk 
 * @param data 
 * @param len 
 * @return int 
 */
int msgpk_write(msgpk_t *msgpk, void *data, size_t len)
{
    #if FILE_ENABLE
    size_t wr_sz = 0;
    #endif

    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (msgpk == NULL || data == NULL || len == 0)return MSGPK_ERR;

    #if FILE_ENABLE
    if (msgpk->msgpk_fd == NULL) {
    #endif
        MSGPK_REQCHK(msgpk, len+3,MSGPK_ERR);
    #if FILE_ENABLE
    }
    #endif

    #if FILE_ENABLE
    else
    {
        if (msgpk->msgpk_sz+len >= msgpk->buf_sz) return MSGPK_ERR_OF;
        wr_sz = fwrite(data, 1, len, msgpk->msgpk_fd);
        if (wr_sz != len) {
            printf("Write err, not match input, write size(%u), input size(%u)\n", wr_sz, len);
        }
        msgpk->msgpk_sz += len;
        return MSGPK_OK;
    }
    #endif

    memcpy(msgpk->msgpk_buf+msgpk->msgpk_sz, data, len);
    msgpk->msgpk_sz += len;
    return MSGPK_OK;
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
    uint8_t *newptr = NULL;
    size_t newsz = 0;
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if (require_sz ==0) return MSGPK_OK;

    // if mem enough
    if (require_sz < (msgpk->buf_sz - msgpk->msgpk_sz) ) {
        return MSGPK_OK;
    }

    newsz = ((1.0 * require_sz) / msgpk->buf_stepsz) < 1.0 ? msgpk->buf_sz + msgpk->buf_stepsz :
        msgpk->buf_sz + msgpk->buf_stepsz + require_sz;
    
    newptr = (uint8_t *)hooks.realloc(msgpk->msgpk_buf, newsz);
    if (newptr == NULL) {
        return MSGPK_ERR;
    } else {
        msgpk->msgpk_buf = newptr;
    }

    msgpk->buf_sz = newsz;

    return MSGPK_OK;
}

#if FILE_ENABLE
/**
 * @brief Finish file create
 * 
 * @param msgpk 
 * @param destory 
 * @return int 
 */
int msgpk_file_done(msgpk_t *msgpk, uint8_t destory)
{
    if (msgpk == NULL) return MSGPK_ERR;
    if (msgpk->msgpk_fd != NULL) fclose(msgpk->msgpk_fd);
    if (destory) hooks.free(msgpk);
    return MSGPK_OK;
}
#endif

/**
 * @brief Delete MessagePack for memory buffer
 * @note yes = 1, not = 0
 * 
 * @param msgpk MessagePack pointer
 * @param del_buf Delete msgpk_buf
 * @param destory Destory the msgpk
 * @return int 
 */
int msgpk_delete(msgpk_t *msgpk, uint8_t del_buf, uint8_t destory)
{
    MSGPK_CHK(msgpk,MSGPK_ERR);
    if ( del_buf && msgpk->msgpk_buf != NULL)hooks.free(msgpk->msgpk_buf);

    if ( destory )hooks.free(msgpk);
    return MSGPK_OK;
}

void msgpk_free(void *ptr)
{
    hooks.free(ptr);
}

/**
 * @brief Create MessagePack
 * 
 * @param init_sz Initialize buffer size
 * @param reserved_sz If the memory is not enough, the reserved size during reallocation 
 * @return msgpk_t* 
 */
msgpk_t *msgpk_create(size_t init_sz, size_t reserved_sz)
{
    msgpk_t *msgpk = (msgpk_t *)hooks.malloc(sizeof(msgpk_t));
    MSGPK_CHK(msgpk, NULL);

    msgpk->msgpk_buf = (uint8_t *)hooks.malloc(init_sz);
    if (msgpk->msgpk_buf == NULL) {
        hooks.free(msgpk);
        return NULL;
    }

    #if FILE_ENABLE
    msgpk->msgpk_fd = NULL;
    #endif

    msgpk->buf_stepsz = reserved_sz;
    msgpk->buf_sz     = init_sz;
    msgpk->msgpk_sz   = 0;
    return msgpk;
}

#if ENCODE_INSIDE

/**
 * @brief Create msgpack file
 * 
 * @param file_path 
 * @param maxLen 
 * @return msgpk_t* 
 */
msgpk_t *msgpk_file_create(const char *file_path, uint64_t maxLen)
#else
msgpk_t *msgpk_file_create(FILE *fd, int64_t maxLen);
#endif
{
    msgpk_t *msgpk = (msgpk_t *)hooks.malloc(sizeof(msgpk_t));
    MSGPK_CHK(msgpk, NULL);

    #if ENCODE_INSIDE
    msgpk->msgpk_fd = fopen(file_path, "wb");
    if (msgpk->msgpk_fd == NULL) {
        hooks.free(msgpk);
        return NULL;
    }
    #else
    msgpk->msgpk_fd = fd
    #endif

    msgpk->msgpk_buf  = NULL;
    msgpk->buf_stepsz = 0;
    msgpk->buf_sz     = maxLen;
    msgpk->msgpk_sz   = 0;
    return msgpk;
}

void msgpk_set_port(msgpk_port_t *port)
{
    if (port == NULL) {
        hooks.malloc  = malloc;
        hooks.calloc  = calloc;
        hooks.realloc = realloc;
        hooks.free    = free;
        return;
    }
    if ( port->malloc != NULL ) hooks.malloc  = port->malloc;
    if ( port->calloc != NULL ) hooks.calloc  = port->calloc;
    if ( port->realloc != NULL ) hooks.realloc = port->realloc;
    if ( port->free != NULL ) hooks.free    = port->free;
}

#if !RDWR_INLINE
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