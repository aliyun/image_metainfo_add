#define _CRT_SECURE_NO_WARNINGS
//
// Created by jinghua on 2021/8/19.
//
#include"JnaBuffer.h"
#include <fcntl.h>
//#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include "error.h"
//#include <fileapi.h>

#ifdef _WIN32
#define fseek _fseeki64
#define ftell _ftelli64

#else
#define fseek fseeko
#define ftell ftello
#endif

int jnaBuffer_remap(jnaBuffer *obj, int64_t cap);
void jnaBuffer_closemap(jnaBuffer *obj);

void* szmalloc(size_t n)
{
    return malloc(n);
}

void szfree(void *p)
{
    if (p != NULL) {
        free(p);
    }
}

void jnaBuffer_init(jnaBuffer *obj)
{
    obj->buf = NULL;
    obj->off = 0;
    obj->len = 0;
    obj->cap = 0;
    obj->md = 0;
    obj->fd = -1;
    obj->readonly = 0;
}

void jnaBuffer_free(jnaBuffer *obj)
{
    if (obj->buf != NULL) {
        if (obj->fd != -1) {
            jnaBuffer_closemap(obj);
        }
        else if (obj->readonly == 0) {
            szfree(obj->buf);
        }
        jnaBuffer_init(obj);
    }
}

void jnaBuffer_readonly(jnaBuffer *obj, const void* ptr, int64_t len)
{
    jnaBuffer_free(obj);
    obj->cap = obj->len = len;
    obj->buf = (uint8_t*)ptr;
    obj->readonly = 1;
}

int jnaBuffer_alloc(jnaBuffer *obj, int64_t n)
{
    assert(n >= 0);
    if (obj->readonly != 0)
        return ERR_READONLY;
    if (n > obj->cap) {
        if (obj->fd != -1) {
            return jnaBuffer_remap(obj, n);
        }
        else if (obj->buf == NULL) {
            obj->cap = ((n + 0x40) & ~0x3F) - 1;
            obj->buf = (uint8_t*)szmalloc(obj->cap + 1);
            if (obj->buf == NULL)
                return ERR_MEMORY;
        }
        else {
            int64_t ncap = (((n > obj->cap * 2 ? n : obj->cap * 2) + 0x40) & ~0x3F) - 1;
            uint8_t* nbuf = (uint8_t*)szmalloc(ncap + 1);
            if (nbuf == NULL)
                return ERR_MEMORY;
            memcpy(nbuf, obj->buf, obj->len);
            szfree(obj->buf);
            obj->buf = nbuf;
            obj->cap = ncap;
        }
    }
    if (n < obj->cap || obj->fd == -1) {
        obj->buf[n] = 0;
    }
    return 0;
}

int jnaBuffer_resize(jnaBuffer *obj, int64_t len)
{
    int ret = jnaBuffer_alloc(obj, len);
    if (ret == 0) {
        obj->len = len;
    }
    return ret;
}

int jnaBuffer_write(jnaBuffer *obj, const void *ptr, int off, int len)
{
    int ret;
    assert(len >= 0);
    ret = jnaBuffer_alloc(obj, obj->len + len);
    if (ret != 0)
        return ret;
    memcpy(obj->buf + obj->len, (char*)ptr + off, len);
    obj->len += len;
    return 0;
}

int jnaBuffer_read(jnaBuffer *obj, void *ptr, int off, int len)
{
    assert(len >= 0);
    if (obj->off >= obj->len)
        return ERR_EOF;
    len = (int)(len <= obj->len - obj->off ? len : obj->len - obj->off);
    memcpy((char*)ptr + off, obj->buf + obj->off, len);
    obj->off += len;
    return len;
}

int jnaBuffer_put(jnaBuffer *obj, uint8_t b)
{
    int ret = jnaBuffer_alloc(obj, obj->len + 1);
    if (ret != 0)
        return ret;
    obj->buf[obj->len++] = b;
    return 0;
}

int jnaBuffer_get(jnaBuffer *obj)
{
    if (obj->off >= obj->len)
        return ERR_EOF;
    return obj->buf[obj->off++];
}

int jnaBuffer_print(jnaBuffer *obj, const char* fmt, ...)
{
    char buf[4096];
    va_list ap;
    int len;
    va_start(ap, fmt);
    len = vsnprintf(buf, 4095, fmt, ap);
    va_end(ap);
    return jnaBuffer_write(obj, buf, 0, len);
}

#ifdef _WIN32

#include <windows.h>

static wchar_t* Utf8ToUnicode(const char* str)
{
	int n = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	wchar_t* ws = (wchar_t*)malloc(n * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, ws, n);
	return ws;
}

int jnaBuffer_readfile(jnaBuffer *obj, const char* path)
{
	int ret = ERR_FAIL;
	int64_t n, m = 0;
	wchar_t *ws = Utf8ToUnicode(path);
	FILE *fin = _wfopen(ws, L"rb");
	free(ws);
	if (fin) {
		fseek(fin, 0, SEEK_END);
		n = ftell(fin);
		if (n > 0) {
			ret = jnaBuffer_alloc(obj, n);
			if (ret != 0) {
				fclose(fin);
				return ret;
			}
			fseek(fin, 0, SEEK_SET);
			while (m < n) {
				int64_t mm = fread(obj->buf + m, 1, n - m, fin);
				if (mm < 0) {
					fclose(fin);
					return ERR_EOF;
				}
				m += mm;
			}
			obj->len = m;
		}
		else {
			obj->len = 0;
		}
		obj->off = 0;
		fclose(fin);
		ret = 0;
	}
	return ret;
}

int jnaBuffer_writefile(jnaBuffer *obj, const char* path, int append)
{
	int ret = ERR_FAIL;
	wchar_t *ws = Utf8ToUnicode(path);
	FILE *fout = _wfopen(ws, append ? L"ab" : L"wb");
	free(ws);
	if (fout) {
		int64_t len = fwrite(obj->buf, 1, obj->len, fout);
		fclose(fout);
		ret = (len == obj->len ? 0 : ERR_EOF);
	}
	return ret;
}

//int jnaBuffer_readmap(jnaBuffer *obj, const char* path)
//{
//	HANDLE fd, md;
//	LARGE_INTEGER i64;
//	wchar_t *ws = Utf8ToUnicode(path);
//	jnaBuffer_free(obj);
//	fd = CreateFileW(ws, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//	free(ws);
//	if (fd == INVALID_HANDLE_VALUE) {
//		return ERR_FAIL;
//	}
//	md = CreateFileMapping(fd, NULL, PAGE_READONLY, 0, 0, NULL);
//	if (md == NULL) {
//		CloseHandle(fd);
//		return ERR_MEMORY;
//	}
//	obj->buf = (uint8_t*)MapViewOfFile(md, FILE_MAP_READ, 0, 0, 0);
//	if (obj->buf == NULL) {
//		CloseHandle(md);
//		CloseHandle(fd);
//		return ERR_MEMORY;
//	}
//	i64.LowPart = GetFileSize(fd, &i64.HighPart);
//	obj->cap = obj->len = i64.QuadPart;
//	obj->md = (size_t)md;
//	obj->fd = (size_t)fd;
//	obj->readonly = 1;
//	return 0;
//}

//int jnaBuffer_writemap(jnaBuffer *obj, const char* path, int64_t n)
//{
//	int64_t len, ncap;
//	uint8_t *nbuf;
//	HANDLE fd, md;
//	LARGE_INTEGER i64;
//	wchar_t *ws = Utf8ToUnicode(path);
//	fd = CreateFileW(ws, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
//	free(ws);
//	if (fd == INVALID_HANDLE_VALUE) {
//		return ERR_FAIL;
//	}
//	if (n < 0) {
//		i64.LowPart = GetFileSize(fd, &i64.HighPart);
//		len = ncap = i64.QuadPart;
//	}
//	else {
//		ncap = ((n + 0x100000) & ~0xFFFFF);
//		i64.QuadPart = ncap;
//		len = 0;
//	}
//	md = CreateFileMapping(fd, NULL, PAGE_READWRITE, i64.HighPart, i64.LowPart, NULL);
//	if (md == NULL) {
//		CloseHandle(fd);
//		return ERR_MEMORY;
//	}
//	nbuf = (uint8_t*)MapViewOfFile(md, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
//	if (nbuf == NULL) {
//		CloseHandle(md);
//		CloseHandle(fd);
//		return ERR_MEMORY;
//	}
//	if (obj->buf != NULL) {
//		len = ncap < obj->len ? ncap : obj->len;
//		memcpy(nbuf, obj->buf, len);
//		jnaBuffer_free(obj);
//	}
//	obj->buf = nbuf;
//	obj->cap = ncap;
//	obj->len = len;
//	obj->md = (size_t)md;
//	obj->fd = (size_t)fd;
//	return 0;
//}

int jnaBuffer_remap(jnaBuffer *obj, int64_t n)
{
	int64_t ncap = ((n + 0x100000) & ~0xFFFFF);
	HANDLE fd = (HANDLE)(size_t)obj->fd;
	HANDLE md = (HANDLE)(size_t)obj->md;
	LARGE_INTEGER i64;
	i64.QuadPart = ncap;
	UnmapViewOfFile(obj->buf);
	CloseHandle(md);
	md = CreateFileMappingA(fd, NULL, PAGE_READWRITE, i64.HighPart, i64.LowPart, NULL);
	if (md == NULL) {
		CloseHandle(fd);
		jnaBuffer_init(obj);
		return ERR_MEMORY;
	}
	obj->buf = (uint8_t*)MapViewOfFile(md, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (obj->buf == NULL) {
		CloseHandle(md);
		CloseHandle(fd);
		jnaBuffer_init(obj);
		return ERR_MEMORY;
	}
	obj->cap = ncap;
	obj->md = (size_t)md;
	return 0;
}

void jnaBuffer_closemap(jnaBuffer *obj)
{
	HANDLE fd = (HANDLE)(size_t)obj->fd;
	HANDLE md = (HANDLE)(size_t)obj->md;
	UnmapViewOfFile(obj->buf);
	CloseHandle(md);
	if (obj->readonly == 0 && obj->len != obj->cap) {
		LARGE_INTEGER liDistanceToMove;
		liDistanceToMove.QuadPart = obj->len;
		SetFilePointerEx(fd, liDistanceToMove, NULL, FILE_BEGIN);
		SetEndOfFile(fd);
	}
	CloseHandle(fd);
}

#else

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int jnaBuffer_readfile(jnaBuffer *obj, const char* path)
{
    int ret = ERR_FAIL;
    int64_t n, m = 0;
    FILE *fin = fopen(path, "rb");
    if (fin) {
        fseek(fin, 0, SEEK_END);
        n = ftell(fin);
        if (n > 0) {
            ret = jnaBuffer_alloc(obj, n);
            if (ret != 0) {
                fclose(fin);
                return ret;
            }
            fseek(fin, 0, SEEK_SET);
            while (m < n) {
                int64_t mm = fread(obj->buf + m, 1, n - m, fin);
                if (mm < 0) {
                    fclose(fin);
                    return ERR_EOF;
                }
                m += mm;
            }
            obj->len = m;
        }
        else {
            obj->len = 0;
        }
        obj->off = 0;
        fclose(fin);
        ret = 0;
    }
    return ret;
}

int jnaBuffer_writefile(jnaBuffer *obj, const char* path, int append)
{
    int ret = ERR_FAIL;
    FILE *fout = fopen(path, append ? "ab" : "wb");
    if (fout) {
        int64_t len = fwrite(obj->buf, 1, obj->len, fout);
        fclose(fout);
        ret = (len == obj->len ? 0 : ERR_EOF);
    }
    return ret;
}

int jnaBuffer_readmap(jnaBuffer *obj, const char* path)
{
    int fd;
    struct stat st;
    jnaBuffer_free(obj);
    fd = open(path, O_RDONLY);
    if (-1 == fd) {
        return ERR_FAIL;
    }
    if (-1 == fstat(fd, &st)) {
        close(fd);
        return ERR_FAIL;
    }
    obj->buf = (uint8_t*)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (obj->buf == (void*)-1) {
        obj->buf = NULL;
        close(fd);
        return ERR_MEMORY;
    }
    obj->cap = obj->len = st.st_size;
    obj->fd = (size_t)fd;
    obj->readonly = 1;
    return 0;
}

int jnaBuffer_writemap(jnaBuffer *obj, const char* path, int64_t n)
{
    int fd;
    uint8_t *nbuf;
    struct stat st;
    int64_t len, ncap;
    fd = open(path, O_RDWR | O_CREAT, 0666);
    if (-1 == fd) {
        return ERR_FAIL;
    }
    if (n < 0) {
        if (-1 == fstat(fd, &st)) {
            close(fd);
            return ERR_FAIL;
        }
        len = ncap = st.st_size;
    }
    else {
        ncap = ((n + 0x100000) & ~0xFFFFF);
        if (-1 == ftruncate(fd, ncap)) {
            close(fd);
            return ERR_FAIL;
        }
        len = 0;
    }
    nbuf = (uint8_t*)mmap(NULL, ncap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (nbuf == (void*)-1) {
        close(fd);
        return ERR_MEMORY;
    }
    if (obj->buf != NULL) {
        len = ncap < obj->len ? ncap : obj->len;
        memcpy(nbuf, obj->buf, len);
        jnaBuffer_free(obj);
    }
    obj->buf = nbuf;
    obj->cap = ncap;
    obj->len = len;
    obj->fd = (size_t)fd;
    return 0;
}

int jnaBuffer_remap(jnaBuffer *obj, int64_t n)
{
    int64_t ncap = ((n + 0x100000) & ~0xFFFFF);
    int fd = (int)obj->fd;
    munmap(obj->buf, obj->cap);
    if (-1 == ftruncate(fd, ncap)) {
        close(fd);
        jnaBuffer_init(obj);
        return ERR_FAIL;
    }
    obj->buf = (uint8_t*)mmap(NULL, ncap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (obj->buf == (void*)-1) {
        close(fd);
        jnaBuffer_init(obj);
        return ERR_MEMORY;
    }
    obj->cap = ncap;
    return 0;
}

void jnaBuffer_closemap(jnaBuffer *obj)
{
    int fd = (int)obj->fd;
    munmap(obj->buf, obj->cap);
    if (obj->readonly == 0 && obj->len != obj->cap) {
        ftruncate(fd, obj->len);
    }
    close(fd);
}

#endif
