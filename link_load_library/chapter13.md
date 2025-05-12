# 运行库实现

## C 语言运行库

### 字符串相关操作

```c
char* itoa(int n, char* str, int radix) {
    char digit[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char* p = str;
    char* head = str;
    if (!p || radix < 2 || radix > 36)
        return p;
    if (radix != 10 && n < 0)
        return p;
    if (n == 0) {
        *p++ = '0';
        *p = 0;
        return p;
    }
    if (radix == 10 && n < 0) {
        *p++ = '-';
        n = -n;
    }
    while (n) {
        *p++ = digit[n % radix];
        n /= radix;
    }
    *p = 0;
    for (--p; head < p; ++head, --p) {
        char temp = *head;
        *head = *p;
        *p = temp;
    }
    return str;
}

int strcmp(const char* src, const char* dst) {
    int ret = 0;
    unsigned char* p1 = (unsigned char*)src;
    unsigned char* p2 = (unsigned char*)dst;
    while (!(ret = *p1 - *p2) && *p2)
        ++p1, ++p2;

    if (ret < 0)
        ret = -1;
    else if (ret > 0)
        ret = 1;
    return (ret);
}

char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while (*src)
        *dest++ = *src++;
    *dest = '\0';
    return ret;
}

unsigned strlen(const char* str) {
    int cnt = 0;
    if (!str)
        return 0;
    for (; *str != '\0'; ++str)
        ++cnt;
    return cnt;
}
```

### 格式化字符串

```c
int fputc(int c, FILE* stream) {
    if (fwrite(&c, 1, 1, stream) != 1)
        return EOF;
    else
        return c;
}

int fputs(const char* str, FILE* stream) {
    int len = strlen(str);
    if (fwrite(str, 1, len, stream) != len)
        return EOF;
    else
        return len;
}

int vfprintf(FILE* stream, const char* format, va_list arglist) {
    int translating = 0;
    int ret = 0;
    const char* p = 0;
    for (p = format; *p != '\0'; ++p) {
        switch (*p) {
            case '%':
                if (!translating)
                    translating = 1;
                else {
                    if (fputc('%', stream) < 0)
                        return EOF;
                    ++ret;
                    translating = 0;
                }
                break;
            case 'd':
                if (translating) {  // %d
                    char buf[16];
                    translating = 0;
                    itoa(va_arg(arglist, int), buf, 10);
                    if (fputs(buf, stream) < 0)
                        return EOF;
                    ret += strlen(buf);
                } else if (fputc('d', stream) < 0)
                    return EOF;
                else
                    ++ret;
                break;
            case 's':
                if (translating) {  // %s
                    const char* str = va_arg(arglist, const char*);
                    translating = 0;
                    if (fputs(str, stream) < 0)
                        return EOF;
                    ret += strlen(str);
                } else if (fputc('s', stream) < 0)
                    return EOF;
                else
                    ++ret;
                break;
            default:
                if (translating)
                    translating = 0;
                if (fputc(*p, stream) < 0)
                    return EOF;
                else
                    ++ret;
                break;
        }
    }
    return ret;
}

int printf(const char* format, ...) {
    va_list(arglist);
    va_start(arglist, format);
    return vfprintf(stdout, format, arglist);
}

int fprintf(FILE* stream, const char* format, ...) {
    va_list(arglist);
    va_start(arglist, format);
    return vfprintf(stream, format, arglist);
}
```

## C++ 运行库实现

### new 与 delete

```c++
// new_delete.cpp
extern "C" void* malloc(unsigned int);
extern "C" void free(void*);

void* operator new(unsigned int size) {
    return malloc(size);
}

void operator delete(void* p) {
    free(p);
}

void* operator new[](unsigned int size) {
    return malloc(size);
}

void operator delete[](void* p) {
    free(p);
}
```

对象的构造和析构是在 `new`/`delete` 之前/之后由编译器负责产生相应的代码进行调用的，`new`/`delete` 仅仅负责堆空间的申请和释放，不负责构造和析构。

### C++ 全局构造与析构

```c++
// ctors.cpp
typedef void (*init_func)(void);

void run_hooks();
extern "C" void do_global_ctors() {
    run_hooks();
}
```

```c++
// crtbegin.cpp
typedef void (*ctor_func)(void);

ctor_func ctors_begin[1] __attribute__ ((section(".ctors"))) = {
    (ctor_func) -1
};

void run_hooks() {
    const ctor_func* list = ctors_begin;
    while ((int)*++list != -1)
        (**list)();
}
```

```c++
// crtend.cpp
typedef void (*ctor_func)(void);

ctor_func ctors_end[1] __attribute__ ((section(".ctors"))) = {
    (ctor_func) -1
};
```
