# allocator

## 具备次配置力（sub-allocation）的SGI空间配置器

### 构造和析构基本工具： construct() 和 destroy()

```c++
#include <new.h>    // 欲使用 placement new，需先包含此文件

template <class T1, class T2>
inline void construct(T1* p, const T2& value) {
    new (p) T1(value);  // placement new；调用T1::T1(value);
}

// 以下是 destroy() 第一版本，接受一个指针
template <class T>
inline void destroy(T* pointer) {
    pointer->~T();      // 调用 dtor ~T()
}

// 以下是 destroy() 第二版本，接受两个迭代器。此函数设法找出元素的数值型别，
// 进而利用 __type_traits<> 求取最适当措施
template <class ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last) {
    __destroy(first, last, value_type(first));
}

// 判断元素的数值型别（value type）是否有trivial destructor
template <class ForwardIterator, class T>
inline void __destroy(ForwardIterator first, ForwardIterator last, T*) {
    typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
    __destroy_aux(first, last, trivial_destructor());
}

// 如果元素的数值型别（value type）有 non-trivial destructor...
template <class ForwardIterator>
inline void __destroy_aux(ForwardIterator first, ForwardIterator last, __false_type) {
    for (; first < last; ++first)
        destroy(&*first);
}

// 如果元素的数值型别（value type）有 trivial destructor...
template <class ForwardIterator>
inline void __destroy_aux(ForwardIterator, ForwardIterator, __true_type) {}

// 以下是destroy() 第二版本针对迭代器为 char* 和 wchar_t* 的特化版
inline void destroy(char*, char*) {}
inline void destroy(wchar_t*, wchar_t*) {}
```

### 空间的配置与释放，std::alloc

```c++
# ifdef __USE_MALLOC
...
typedef __malloc_alloc_template<0> malloc_alloc;
typedef malloc_alloc alloc;     // 令 alloc 为第一级配置器
# else
...
// 令 alloc 为第二级配置器
typedef __default_alloc_template<__NODE_ALLOCATOR_THREADS, 0> alloc;
# endif /* ! __USE_MALLOC */

template <class T, class Alloc>
class simple_alloc {
public:
    static T *allocate(size_t n) {
        return 0 == n ? 0 : (T*)Alloc::allocate(n * sizeof(T));
    }
    static T *allocate(void) {
        return (T*) Alloc::allocate(sizeof(T));
    }
    static void deallocate(T *p, size_t n) {
        if (0 != n)
            Alloc::deallocate(p, n * sizeof(T));
    }
    static void deallocate(T *p) {
        Alloc::deallocate(p, sizeof(T));
    }
}
```

### 第一级配置器 __malloc_alloc_template

```c++
#if 0
#   include <new>
#   define __THROW_BAD_ALLOC throw bad_alloc
#elif !defined(__THROW_BAD_ALLOC)
#   include <iostream.h>
#   define __THROW_BAD_ALLOC cerr << "out of memory" << endl; exit(1)
#endif

// malloc-based allocator. 通常比稍后介绍的default alloc速度慢
// 一般而言是 thread-safe，并且对于空间的运用比较高效（efficient）
// 以下是第一级配置器
// 注意，无“template 型别参数” inst，则完全没派上用场
template <int inst>
class __malloc_alloc_template {

private:
// 以下函数将用来处理内存不足的情况
// oom : out of memory
static void *oom_malloc(size_t);
static void *oom_realloc(void*, size_t);
static void (* __malloc_alloc_oom_handler)();

public:

static void *allocate(size_t n) {
    void *result = malloc(n);   // 第一级配置器直接使用 malloc()
    // 以下无法满足需求时，改用 oom_malloc()
    if (0 == result)
        result = oom_malloc(n);
    return result;
}

static void deallocate(void *p, size_t /* n */) {
    free(p);    // 第一级配置器直接使用 free(n)
}

static void reallocate(void *p, size_t /* old_sz */, size_t new_sz) {
    void *result = realloc(p, new_sz);  // 第一级配置器直接使用 realloc()
    // 以下无法满足需求时，改用 oom_realloc()
    if (0 == result)
        result = oom_realloc(p, new_sz);
    return result;
}

// 以下仿真C++的set_new_handler()。换句话说，你可以通过它
// 指定你自己的 out-of-memory-handler
static void (* set_malloc_handler(void (*f)()))() {
    void (*old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = f;
    return(old);
}
};

// malloc-alloc out-of-memory handling
// 初值为0。有待客端设定
template <int inst>
void (* __malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

template <int inst>
void * __malloc_alloc_template<inst>::oom_malloc(size_t n) {
    void (* my_malloc_handler)();
    void *result;

    for (;;) {      // 不断尝试释放、配置、再释放、再配置...
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)(); // 调用处理例程，企图释放内存
        result = malloc(n);     // 再次尝试配置内存
        if (result)
            return(result);
    }
}

template <int inst>
void * __malloc_alloc_template<inst>::oom_realloc(void *p, size_t n) {
    void (* my_malloc_handler)();
    void *result;

    for (;;) {      // 不断尝试释放、配置、再释放、再配置...
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)(); // 调用处理例程，企图释放内存
        result = realloc(p, n); // 再次尝试配置内存
        if (result)
            return(result);
    }
}

// 注意，以下直接将参数inst指定为0
typedef __malloc_alloc_template<0> malloc_alloc;
```

### 第二级配置器 __default_alloc_template

```c++
enum {__ALIGN = 8};     // 小型区块的上调边界
enum {__MAX_BYTES = 128};   // 小型区块的上限
enum {__NFREELISTS = __MAX_BYTES/__ALIGN};  // free-lists 个数

// 以下是第二级配置器
// 注意，无“template型别参数”，且第二参数完全没派上用场
// 第一参数用于多线程环境下。
template <bool threads, int inst>
class __default_alloc_template {

private:
    // ROUND_UP() 将 bytes 上调至 8 的倍数
    static size_t ROUND_UP(size_t bytes) {
        return (((bytes) + __ALIGN-1) & !(__ALIGN - 1));
    }
private:
    union obj {     // free-lists 的节点构造
        union obj * free_list_link;
        char client_data[1];    /* The client sees this. */
    };
private:
    // 16个 free-lists
    static obj * volatile free_list[__NFREELISTS];
    // 以下函数根据区块大小，决定使用第n号free-list。n从0起算
    static size_t FREELIST_INDEX(size_t bytes) {
        return (((bytes) + __ALIGN - 1) / __ALIGN - 1);
    }

    // 返回一个大小为n的对象，并可能加入大小为n的其它区块到free list
    static void *refill(size_t n);
    // 配置一大块空间，可容纳nobjs个大小为 "size" 的区块
    // 如果配置 nobjs 个区块有所不便，nobjs 可能会降低
    static char *chunk_alloc(size_t size, int &nobjs);

    // Chunk allocation state
    static char *start_free;    // 内存池起始位置，只在chunk_alloc()中变化
    static char *end_free;      // 内存池结束位置，只在chunk_alloc()中变化
    static size_t heap_size;

public:
    static void *allocate(size_t n) { /*详述于后*/ }
    static void deallocate(void *p, size_t n) { /*详述于后*/ }
    static void *reallocate(void *p, size_t old_sz, size_t new_sz);
};

// 以下是 static data member 的定义与初值设定
template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::start_free = 0;

template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::end_free = 0;

template <bool threads, int inst>
size_t __default_alloc_template<threads, inst>::heap_size = 0;

template <bool threads, int inst>
__default_alloc_template<threads, inst>::obj * volatile
__default_alloc_template<threads, inst>::free_list[__NFREELISTS] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
```

### 空间配置函数 allocate()

```c++
// n must be > 0
static void* allocate(size_t n) {
    obj * volatile * my_free_list;
    obj * result;
    // 大于128就调用第一级配置器
    if (n > (size_t) __MAX_BYTES) {
        return(malloc_alloc::allocate(n));
    }
    // 寻找16个free lists中适当的一个
    my_free_list = free_list + FREELIST_INDEX(n);
    result = *my_free_list;
    if (result == 0) {
        // 没找到可用的 free list，准备重新填充free list
        void *r = refill(ROUND_UP(n));
        return r;
    }
    // 调整free list
    *my_free_list = result->free_list_link;
    return (result);
};
```

### 空间释放函数 deallocate()

```c++
// p 不可以是 0
static void deallocate(void *p, size_t n) {
    obj *q = (obj*)p;
    obj * volatile * my_free_list;

    // 大于128就调用第一级配置器
    if (n > (size_t) __MAX_BYTES) {
        malloc_alloc::deallocate(p, n);
        return;
    }
    // 寻找对应的free list
    my_free_list = free_list + FREELIST_INDEX(n);
    // 调整free list，回收区块
    q->free_list_link = *my_free_list;
    *my_free_list = q;
}
```

### 重新填充free lists

```c++
// 返回一个大小为n的对象，并且有时候会为适当的free list增加节点
// 假设n已经适当上调至8的倍数
template <bool threads, int inst>
void* __default_alloc_template<threads, inst>::refill(size_t n) {
    int nobjs = 20;
    // 调用 chunk_alloc()，尝试取得nobjs个区块作为free list的新节点
    // 注意参数nobjs是 pass by reference
    char * chunk = chunk_alloc(n, nobjs);
    obj * volatile * my_free_list;
    obj * result;
    obj * current_obj, * next_obj;
    int i;

    // 如果只获得一个区块，这个区块就分配给调用者用，free list 无新节点
    if (1 == nobjs)
        return(chunk);
    // 否则准备调整free list，纳入新节点
    my_free_list = free_list + FREELIST_INDEX(n);

    // 以下在chunk空间内建立 free list
    result = (obj *)chunk;  // 这一块准备返回给客端
    // 以下导引free list指向新配置的空间（取自内存池）
    *my_free_list = next_obj = (obj*)(chunk + n);
    // 以下将free list的各节点串接起来
    for (i = 1; ; i++) {    // 从1开始，因为第0个将返回给客端
        current_obj = next_obj;
        next_obj = (obj*)((char*)next_obj + n);
        if (nobjs - 1 == i) {
            current_obj->free_list_link = 0;
            break;
        } else {
            current_obj->free_list_link = next_obj;
        }
    }
    return (result);
}
```

### 内存池（memory pool）

```c++
// 假设size已经适当上调至8的倍数
// 注意参数nobjs是 pass by reference
template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::chunk_alloc(size_t size, int& nobjs) {
    char* result;
    size_t total_bytes = size * nobjs;
    size_t bytes_left = end_free - start_free;  // 内存池剩余空间

    if (bytes_left >= total_bytes) {
        // 内存池剩余空间完全满足需求量
        result = start_free;
        start_free += total_bytes;
        return(result);
    } else if (bytes_left >= size) {
        // 内存池剩余空间不能完全满足需求量，但足够供应一个（含）以上的区块
        nobjs = bytes_left / size;
        total_bytes = size * nobjs;
        result = start_free;
        start_free += total_bytes;
        return(result);
    } else {
        // 内存池剩余空间连一个区块的大小都无法提供
        size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
        // 以下试着让内存池中的残余零头还有利用价值
        if (bytes_left > 0) {
            // 内存池内还有一些零头，先配给适当的free list
            // 首先寻找适当的free list
            obj * volatile * my_free_list = free_list + FREELIST_INDEX(bytes_left);
            // 调整free list，将内存池中的残余空间编入
            ((obj*)start_free)->free_list_link = *my_free_list;
            *my_free_list = (obj*)start_free;
        }

        // 配置 heap 空间，用来补充内存池
        start_free = (char *)malloc(bytes_to_get);
        if (0 == start_free) {
            // heap空间不足，malloc()失败
            int i;
            obj * volatile * my_free_list, *p;
            // 试着检视我们手上拥有的东西。这不会造成伤害。我们不打算尝试配置
            // 较小的区块，因为那在多进程（multi-process）机器上容易导致灾难
            // 以下搜寻适当的 free list
            // 所谓适当是指“尚有未用区块，且区块够大”之free list
            for (i = size; i <= __MAX_SIZE; i += __ALIGN) {
                my_free_list = free_list + FREELIST_INDEX(i);
                p = *my_free_list;
                if (0 != p) {
                    // 调整free list以释出未用区块
                    *my_free_list = p->free_list_link;
                    start_free = (char *)p;
                    end_free = start_free + i;
                    // 递归调用自己，为了修正nobjs
                    return(chunk_alloc(size, nobjs));
                    // 注意，任何残余零头终将被编入适当的free-list中备用
                }
            }
            end_free = 0;   // 如果出现意外（山穷水尽，到处都没内存可用了）
            // 调用第一级配置器，看看out-of-memory机制能否尽点力
            start_free = (char*)malloc_alloc::allocate(bytes_to_get);
            // 这会导致抛出异常（exception），或内存不足的情况下获得改善
        }
        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        // 递归调用自己，为了修正nobjs
        return(chunk_alloc(size, nobjs));
    }
}

```

## 内存基本处理工具

```c++
template <class ForwardIterator, class Size, class T>
inline ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& x) {
    return __uninitialized_fill_n(first, n, x, value_type(first));
    // 以上，利用value_type()取出first的value type
}

template <class ForwardIterator, class Size, class T, class T1>
inline ForwardIterator __uninitialized_fill_n(ForwardIterator first, Size n, const T& x, T1*) {
    // 以下 __type_traits<>技法
    typedef typename __type_traits<T1>::is_POD_type is_POD;
    return __uninitialized_fill_n_aux(first, n, x, is_POD());
}

// 如果copy construction 等同于assignment，而且
// destructor是 trivial，以下就有效
// 如果是POD型别，执行流程就会转进到以下函数。这是藉由function template
// 的参数推导机制而得
template <class ForwardIterator, class Size, class T>
inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, __true_type) {
    return fill_n(first, n, x); // 交由高阶函数执行
}

// 如果不是POD型别，执行流程就会转进到以下函数。这是藉由function template
// 的参数推导机制而得
template <class ForwardIterator, class Size, class T>
ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, __false_type) {
    ForwardIterator cur = first;
    // 为求阅读顺畅，以下将原本该有的异常处理（exception handling）省略
    for (; n > 0; --n, ++cur)
        construct(&*cur, x);
    return cur;
}

template <class InputIterator, class ForwardIterator>
inline ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result) {
    return __uninitialized_copy(first, last, result, value_type(result));
    // 以上，利用 value_type() 取出first的value_type
}

template <class InputIterator, class ForwardIterator, class T>
inline ForwardIterator __uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result, T*) {
    typedef typename __type_traits<T>::is_POD_type is_POD;
    return __uninitialized_copy_aux(first, last, result, is_POD());
    // 以上，企图利用 is_POD() 所获得的结果，让编译器做参数推导
}

// 如果 copy construction 等同于 assignment，而且
// destructor 是 trivial，以下就有效
// 如果是POD型别，执行流程就会转进到以下函数。这是藉由function template
// 的参数推导机制而得
template <class InputIterator, class ForwardIterator>
inline ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, __true_type) {
    return copy(first, last, result);   // 调用STL算法copy()
}

// 如果是non-POD型别，执行流程就会转进到以下函数。这是藉由function template
// 的参数推导机制而得
template <class InputIterator, class ForwardIterator>
inline ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, __false_type) {
    ForwardIterator cur = result;
    // 为求阅读顺畅，以下将原本该有的异常处理（exception handling）省略
    for (; first != last; ++first, ++cur)
        construct(&*cur, *first);   // 必须一个一个元素地构造，无法批量进行
    return cur;
}

// 以下是针对 const char* 的特化版本
inline char* uninitialized_copy(const char* first, const char* last, char* result) {
    memmove(result, first, last - first);
    return result + (last - first);
}

// 以下是针对 const wchar_t* 的特化版本
inline char* uninitialized_copy(const wchar_t* first, const wchar_t* last, char* result) {
    memmove(result, first, sizeof(wchar_t) * (last - first));
    return result + (last - first);
}

template <class ForwardIterator, class T>
inline void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x) {
    __uninitialized_fill(first, last, x, value_type(first));
}

template <class ForwardIterator, class T, class T1>
inline void __uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x, T1*) {
    typedef typename __type_traits<T1>::is_POD_type is_POD;
    __uninitialized_fill_aux(first, last, x, is_POD());
}

// 如果copy construction 等同于 assignment，而且
// destructor 是 trivial，以下就有效
// 如果是POD型别，执行流程就会转进到以下函数。这是藉由function template
// 的参数推导机制而得
template <class ForwardIterator, class T>
inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, __true_type) {
    fill(first, last, x);   // 调用STL算法fill()
}

// 如果是non-POD型别，执行流程就会转进到以下函数。这是藉由function template
// 的参数推导机制而得
template <class ForwardIterator, class T>
inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, __false_type) {
    ForwardIterator cur = first;
    // 为求阅读顺畅，以下将原本该有的异常处理（exception handling）省略
    for (; cur != last; ++cur)
        construct(&*cur, x);    // 必须一个一个元素构造，无法批量进行
}
```
