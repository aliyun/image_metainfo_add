#ifndef __JNA_BUFFER_H__
#define __JNA_BUFFER_H__

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

typedef struct jnaBuffer_t {
	uint8_t *buf;
	int64_t  off;
	int64_t  len;
	int64_t  cap;
	int64_t  md;
	int64_t  fd;
	int readonly;
} jnaBuffer;

#pragma pack()


_declspec(dllexport) void szfree(void *p);

_declspec(dllexport) void* szmalloc(size_t n);
_declspec(dllexport) void jnaBuffer_init(jnaBuffer *obj);
_declspec(dllexport) void jnaBuffer_free(jnaBuffer *obj);
_declspec(dllexport) void jnaBuffer_readonly(jnaBuffer *obj, const void* ptr, int64_t len);
_declspec(dllexport) int jnaBuffer_alloc(jnaBuffer *obj, int64_t cap);
_declspec(dllexport) int jnaBuffer_resize(jnaBuffer *obj, int64_t len);
_declspec(dllexport) int jnaBuffer_write(jnaBuffer *obj, const void *ptr, int off, int len);
_declspec(dllexport) int jnaBuffer_read(jnaBuffer *obj, void *ptr, int off, int len);
_declspec(dllexport) int jnaBuffer_put(jnaBuffer *obj, uint8_t b);
_declspec(dllexport) int jnaBuffer_get(jnaBuffer *obj);
_declspec(dllexport) int jnaBuffer_print(jnaBuffer *obj, const char* fmt, ...);
_declspec(dllexport) int jnaBuffer_readfile(jnaBuffer *obj, const char* path);
_declspec(dllexport) int jnaBuffer_writefile(jnaBuffer *obj, const char* path, int append);
_declspec(dllexport) int jnaBuffer_readmap(jnaBuffer *obj, const char* path);
_declspec(dllexport) int jnaBuffer_writemap(jnaBuffer *obj, const char* path, int64_t cap);

typedef jnaBuffer cszstr;



#define cszstr_init jnaBuffer_init
#define cszstr_free jnaBuffer_free
#define cszstr_alloc jnaBuffer_alloc
#define cszstr_resize jnaBuffer_resize
#define cszstr_write jnaBuffer_write
#define cszstr_read jnaBuffer_read
#define cszstr_readfile jnaBuffer_readfile
#define cszstr_writefile jnaBuffer_writefile
#define cszstr_readmap jnaBuffer_readmap
#define cszstr_writemap jnaBuffer_writemap



#ifdef __cplusplus
}
#endif
#endif
