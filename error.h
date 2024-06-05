﻿#ifndef __ERROR_H_INCLUDE__
#define __ERROR_H_INCLUDE__

enum {
    RV_OK = 0,
    ERR_FAIL = 0x8000,
    ERR_PARAM = 0x8001,
    ERR_DATA = 0x8002,
    ERR_INLEN = 0x8003,
    ERR_MEMORY = 0x8004,
    ERR_READFILE = 0x8005,
    ERR_WRITEFILE = 0x8006,
    ERR_CORCODE = 0x8007,
    ERR_TAG = 0x8008,
    ERR_CRC = 0x8009,
    ERR_TYPE = 0x800A,
    ERR_READONLY = 0x800B,
    ERR_EOF = 0x800C,
};


#endif
