#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiny_msgpk.h"

void prHex(uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        printf("%02x", data[i]);
        printf( ((i+1) % 16) ? " " : "\n" );
    }
    if(len % 16)printf("\n");
}

int main(void)
{
    msgpk_t *msgpk = NULL;

    msgpk = msgpk_create(8, 4);
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
        uint8_t fixext8[] = {0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x11,0x22};
        msgpk_add_ext(msgpk, 4, fixext8, 8);
        
        msgpk_add_str(msgpk, "fixext16", 8);
        uint8_t fixext16[] = {
            0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
            0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
        };
        msgpk_add_ext(msgpk, 5, fixext16, 16);
        
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
    prHex(msgpk->msgpk_buf, msgpk->msgpk_sz);
    printf("Msgpk: bufsz(%u), msgsz(%u)\n", msgpk->buf_sz, msgpk->msgpk_sz);
    return 0;
}