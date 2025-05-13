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

### `atexit` 实现

```c
// atexit.c

typedef struct _func_node {
    atexit_func_t func;
    void* arg;
    int is_cxa;
    struct _func_node* next;
} func_node;

static func_node* atexit_list = 0;

int register_atexit(atexit_func_t func, void* arg, int is_cxa) {
    func_node* node;
    if (!func)
        return -1;

    node = (func_node*)malloc(sizeof(func_node));

    if (node == 0)
        return -1;
    
    node->func = func;
    node->arg = arg;
    node->is_cxa = is_cxa;
    node->next = atexit_list;
    atexit_list = node;
    return 0;
}

typedef void (*cxa_func_t)(void*);
int __cxa_atexit(cxa_func_t func, void* arg, void* unused) {
    return register_atexit((atexit_func_t)func, arg, 1);
}

int atexit(atexit_func_t func) {
    return register_atexit(func, 0, 0);
}

void mini_crt_call_exit_routine() {
    func_node* p = atexit_list;
    for (; p != 0; p = p->next) {
        if (p->is_cxa)
            ((cxa_func_t)p->func)(p->arg);
        else
            p->func();
        free(p);
    }
    atexit_list = 0;
}
```

### `stream` 与 `string`

```c++
// string

namespace std {

class string {
    unsigned len;
    char* pbuf;

public:
    explicit string(const char* str);
    string(const string&);
    ~string();
    string& operator=(const string&);
    string& operator=(const char* s);
    const char* operator[](unsigned idx) const;
    char& operator[](unsigned idx);
    const char* c_str() const;
    unsigned length() const;
    unsigned size() const;
};

string::string(const char* str) :
    len(0), pbuf(0) {
    *this = str;
}

string::string(const string& s) :
    len(0), pbuf(0) {
    *this = s;
}

string::~string() {
    if (pbuf != 0) {
        delete[] pbuf;
        pbuf = 0;
    }
}

string& string::operator=(const string& s) {
    if (&s == this)
        return *this;
    this->~string();
    len = s.len;
    pbuf = strcpy(new char[len + 1], s.pbuf);
    return *this;
}

string& string::operator=(const char* s) {
    this->~string();
    len = strlen(s);
    pbuf = strcpy(new char[len + 1], s);
    return *this;
}

const char& string::operator[](unsigned idx) const {
    return pbuf[idx];
}

char& string::operator[](unsigned idx) {
    return pbuf[idx];
}

const char* string::c_str() const {
    return p_buf;
}

unsigned string::length() const {
    return len;
}

unsigned string::size() const {
    return len;
}

ofstream& operator<<(ofstream& o, const string& s) {
    return o << s.c_str();
}

}
```

```c++
// iostream
namespace std {

class ofstream {
protected:
    FILE* fp;
    ofstream(const ofstream&);
public:
    enum openmode{in = 1, out = 2, binary = 4, trunc = 8};

    ofstream();
    explicit ofstream(const char* filename, ofstream::openmode md = ofstream::out);
    ~ofstream();
    ofstream& operator<<(char c);
    ofstream& operator<<(int n);
    ofstream& operator<<(const char* str);
    ofstream& operator<<(ofstream& (*)(ofstream&));
    void open(const char* filename, ofstream::openmode md = ofstream::out);
    void close();
    ofstream& write(const char* buf, unsigned size);
};

inline ofstream& endl(ofstream& o) {
    return o << '\n';
}

class stdout_stream : public ofstream {
public:
    stdout_stream();
};

extern stdout_stream cout;
}
```

```c++
// iostream.cpp
#include "iostream"

namespace std {

stdout_stream::stdout_stream() : ofstream() {
    fp = stdout;
}

stdout_stream cout;

ofstream::ofstream() : fp(0) {}

ofstream::ofstream(const char* filename, ofstream::openmode md) : fp(0) {
    open(filename, md);
}

ofstream::~ofstream() {
    close();
}

ofstream& ofstream::operator<<(char c) {
    fputc(c, fp);
    return *this;
}

ofstream& ofstream::operator<<(int n) {
    fprintf(fp, "%d", n);
    return *this;
}

ofstream& ofstream::operator<<(const char* str) {
    fprintf(fp, "%s", str);
    return *this;
}

ofstream& ofstream::operator<<(ofstream& (*manip)(ofstream&)) {
    return manip(*this);
}

void ofstream::open(const char* filename, ofstream::openmode md) {
    char mode[4];
    close();
    switch (md) {
        case out | trunc:
            strcpy(mode, "w");
            break;
        case out | in | trunc:
            strcpy(mode, "w+");
        case out | trunc | binary:
            strcpy(mode, "wb");
            break;
        case out | in | trunc | binary:
            strcpy(mode, "wb+");
    }
    fp = fopen(filename, mode);
}

void ofstream::close() {
    if (fp) {
        fclose(fp);
        fp = 0;
    }
}

ofstream& ofstream::write(const char* buf, unsigned size) {
    fwrite(buf, 1, size, fp);
    return *this;
}

}
```