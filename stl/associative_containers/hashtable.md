# hashtable

```c++
template <class Value>
struct __hashtable_node {
    __hasttable_node* node;
    Value value;
};
```

```c++
template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_iterator {
    typedef hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> hashtable;
    typedef __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> iterator;
    typedef __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> const_iterator;
    typedef __hashtable_node<Value> node;

    typedef forward_iterator_tag iterator_category;
    typedef Value value_type;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef Value& reference;
    typedef Value* pointer;

    node* cur;      // 迭代器目前所指之节点
    hashtable* ht;  // 保持对容器的连结关系（因为可能需要从bucket调到bucket）

    __hashtable_iterator(node* n, hashtable* tab) : cur(n), ht(tab) {}
    __hashtable_iterator() {}
    reference operator*() const { return cur->val; }
    pointer operator->() const { return &(operator*()); }
    iterator& operator++();
    iterator operator++(int);
    bool operator==(const iterator& it) const { return cur == it.cur; }
    bool operator!=(const iterator& it) const { return cur != it.cur; }
}

template <class V, class K, class HF, class ExK, class EqK, class A>
__hashtable_iterator<V, K, HF, ExK, EqK, A>&
__hashtable_iterator<V, K, HF, ExK, EqK, A>::operator++() {
    const node* old = cur;
    cur = cur->next;    // 如果存在，就是它。否则进入以下if流程
    if (!cur) {
        // 根据元素值，定位出下一个bucket。其起头处就是我们的目的地
        size_type bucket = ht->bkt_num(old->val);
        while (!cur && ++bucket < ht->buckets.size())   // 注意，operator++
            cur = ht->buckets[bucket];
    }
    return *this;
}

template <class V, class K, class HF, class ExK, class EqK, class A>
inline __hashtable_iterator<V, K, HF, ExK, EqK, A>
__hashtable_iterator<V, K, HF, ExK, EqK, A>::operator++(int) {
    iterator tmp = *this;
    ++*this;    // 调用operator++()
    return tmp;
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey, class Alloc = alloc>
class hashtable;

// ...

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey,
            class Alloc>    // 先前声明时，已给与Alloc默认值alloc
class hashtable {
public:
    typedef HashFcn hasher;     // 为template型别参数重新定义一个名称
    typedef EqualKey key_equal; // 为template型别参数重新定义一个名称
    typedef size_t size_type;

private:
    // 以下三者都是function objects。<stl_hash_fun.h>中定义有数个
    // 标准型别（如int，c-style string等）的hasher
    hasher hash;
    key_equal equals;
    ExtractKey get_key;

    typedef __hashtable_node<Value> node;
    typedef simple_alloc<node, Alloc> node_allocator;

    vector<node*, Alloc> buckets;   // 以vector完成
    size_type num_elements;

public:
    // bucket个数即buckets vector的大小
    size_type bucket_count() const { return buckets.size(); }
...

protected:
    // 注意：假设long至少有32bits
    static const int __stl_num_primes = 28;
    static const unsigned long __stl_prime_list[__stl_num_primes] = {
        53, 97, 193, 389, 769,
        1543, 3079, 6151, 12289, 24593,
        49157, 98317, 196613, 393241, 786433,
        1572869, 3145739, 6291469, 12582917, 25165843,
        50331653, 100663319, 201326611, 402653189, 805306457,
        1610612741, 3221225473ul, 4294967291ul
    };

    // 以下找出上述28个质数之中，最接近并大于或等于n的那个质数
    inline unsigned long __stl_next_prime(unsigned long n) {
        const unsigned long* first = __stl_prime_list;
        const unsigned long* last = __stl_prime_list + __stl_num_primes;
        const unsigned long* pos = lower_bound(first, last, n);
        // 使用lower_bound()，序列需先排序。没问题，上述数组已排序
        return pos == last ? *(last - 1) : *pos;
    }

    // 总共可以有多少个buckets. 以下是hash_table的一个member function
    size_type max_bucket_count() const {
        return __stl_prime_list[__stl_num_primes - 1];
    }   // 其值为4294967291

protected:
    node* new_node(const value_type& obj) {
        node* n = node_allocator::allocate();
        n->next = 0;
        __STL_TRY {
            construct(&n->val, obj);
            return n;
        }
        __STL_UNWIND(node_allocator::deallocate(n));
    }

    void delete_node(node* n) {
        destroy(&n->val);
        node_allocator::deallocate(n);
    }

protected:
    hashtable(size_type n, const HashFcn& hf, const EqualKey& eql)
        : hash(hf), equals(eql), get_key(ExtractKey()), num_elements(0) {
        initialize_buckets(n);
    }

    void initialize_buckets(size_type n) {
        const size_Type n_buckets = next_size(n);
        // 举例：传入50，返回53,。以下首先保留53个元素空间，然后将其全部填0
        buckets.reserve(n_buckets);
        buckets.insert(buckets.end(), n_buckets, (node*)0);
        num_elements = 0;
    }

    size_type next_size(size_type n) const { return __stl_next_prime(n); }

    // 插入元素，不允许重复
    pair<iterator, bool> insert_unique(const value_type& obj) {
        resize(num_elements + 1);   // 判断是否需要重建表格，如需要就扩充
        return insert_unique_noresize(obj);
    }

    // 插入元素，允许重复
    iterator insert_equal(const value_type& obj) {
        resize(num_elements + 1);   // 判断是否需要重建表格，如需要就扩充
        return insert_equal_noresize(obj);
    }

    // 版本1：接受实值（value）和buckets个数
    size_type bkt_num(const value_type& obj, size_t n) const {
        return bkt_num_key(get_key(obj), n);    // 调用版本4
    }

    // 版本2：只接受实值（value）
    size_type bkt_num(const value_type& obj) const {
        return bkt_num_key(get_key(obj));       // 调用版本3
    }

    // 版本3：只接受键值
    size_type bkt_num_key(const key_type& key) const {
        return bkt_num_key(key, buckets.size());    // 调用版本4
    }

    // 版本4：接受键值和buckets个数
    size_type bkt_num_key(const key_type& key, size_t n) const {
        return hash(key) % n;
    }

    iterator find(const key_type& key) {
        size_type n = bkt_num_key(key); // 首先寻找落在那一个bucket内
        node* first;
        // 以下，从bucket list的头开始，一一比对每个元素的键值。比对成功就跳出
        for (first = buckets[n]; first && !equals(get_key(first->val), key); first = first->next)
            {}
        return iterator(first, this);
    }

    size_type count(const key_type& key) const {
        const size_type n = bkt_num_key(key);   // 首先寻找落在哪一个bucket内
        size_type result = 0;
        // 以下，从bucket list的头开始，一一比对每个元素的键值。比对成功就累加1。
        for (const node* cur = buckets[n]; cur; cur = cur->next)
            if (equals(get_key(cur->val), key))
                ++result;
        return result;
    }
};

// 以下函数判断是否需要重建表格。如果不需要，立刻回返。如果需要，就动手
template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::resize(size_type num_elements_hint) {
    // 以下，“表格重建与否”的判断原则颇为奇特，是拿元素个数（把新增元素计入后）和
    // bucket vector的大小来比。如果前者大于后者，就重建表格
    // 由此可判断知，每个bucket（list）的最大容量和buckets vector的大小相同
    const size_type old_n = buckets.size();
    if (num_elements_hint > old_n) {    // 确定真的需要重新配置
        const size_type n = next_size(num_elements_hint);   // 找出下一个质数
        if (n > old_n) {
            vector<node*, A> tmp(n, (node*)0);  // 设立新的buckets
            __STL_TRY {
                // 以下处理每一个旧的bucket
                for (size_type bucket = 0; bucket < old_n; ++bucket) {
                    node* first = buckets[bucket];  // 指向节点所对应之串行的起始节点
                    // 以下处理每一个旧bucket所含（串行）的每一个节点
                    while (first) {     // 串行还没结束时
                        // 以下找出节点落在哪一个新bucket内
                        size_type new_bucket = bkt_num(first->val, n);
                        // 以下四个操作颇为微妙
                        // (1) 令旧bucket指向其所对应之串行的下一个节点（以便迭代处理）
                        buckets[bucket] = first->next;
                        // （2)(3) 将当前节点插入到新bucket内，成为其对应串行的第一个节点
                        first->next = tmp[new_bucket];
                        tmp[new_bucket] = first;
                        // (4) 回到旧bucket所指的待处理串行，准备处理下一个节点
                        first = buckets[bucket];
                    }
                }
                buckets.swap(tmp);  // vector::swap。新旧两个buckets对调
                // 注意，对调两方如果大小不同，大的会变小，小的会变大
                // 离开时释放local tmp的内存
            }
        }
    }
}

template <class V, class K, class HF, class Ex, class Eq, class A>
pair<typename hashtable<V, K, HF, Ex, Eq, A>::iterator, bool>
hashtable<V, K, HF, Ex, Eq, A>::insert_unique_noresize(const value_type& obj) {
    const size_type n = bkt_num(obj);   // 决定obj应位于#n bucket
    node* first = buckets[n];   // 令first指向bucket对应之串行头部

    // 如果buckets[n]已被占用，此时first将不为0，于是进入以下循环，
    // 走过bucket所对应的整个链表
    for (node* cur = first; cur; cur = cur->next)
        if (equals(get_key(cur->val), get_key(obj)))
            // 如果发现与链表中的某键值相同，就不插入，立刻返回
            return pair<iterator, bool>(iterator(cur, this), false);
    
    // 离开以上循环（或根本未进入循环）时，first指向bucket所指链表的头部节点
    node* tmp = new_node(obj);  // 产生新节点
    tmp->next = first;
    buckets[n] = tmp;           // 令新节点成为链表的第一个节点
    ++num_elements;             // 节点个数累加1
    return pair<iterator, bool>(iterator(tmp, this), true);
}

// 在不需重建表格的情况下插入新节点。键值允许重复
template <class V, class K, class HF, class Ex, class Eq, class A>
typename hashtable<V, K, HF, Ex, Eq, A>::iterator
hashtable<V, K, HF, Ex, Eq, A>::insert_equal_noresize(const value_type& obj) {
    const size_type n = bkt_num(obj);   // 决定obj应位于 #n bucket
    node* first = buckets[n];   // 令first指向bucket对应之链表头部

    // 如果buckets[n]已被占用，此时first将不为0，于是进入以下循环，
    // 走过bucket所对应的整个链表
    for (node* cur = first; cur; cur = cur->next)
        if (equals(get_key(cur->val), get_key(obj))) {
            // 如果发现与链表中的某键值相同，就马上插入，然后返回
            node* tmp = new_node(obj);  // 产生新节点
            tmp->next = cur->next;      // 将新节点插入于目前位置之后
            cur->next = tmp;
            ++num_elements;             // 节点个数累加1
            return iterator(tmp, this); // 返回一个迭代器，指向新增节点
        }
    
    // 进行至此，表示没有发现重复的键值
    node* tmp = new_node(obj);      // 产生新节点
    tmp->next = first;              // 将新节点插入于链表头部
    buckets[n] = tmp;
    ++num_elements;                 // 节点个数累加1
    return iterator(tmp, this);     // 返回一个迭代器，指向新增节点
}

template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::clear() {
    // 针对每一个bucket.
    for (size_type i=0; i<buckets.size(); ++i) {
        node* cur = buckets[i];
        // 将bucket list中的每一个节点删除掉
        while (cur != 0) {
            node* next = cur->next;
            delete_node(cur);
            cur = next;
        } 
        buckets[i] = 0;     // 令bucket内容为null指针
    }
    num_elements = 0;       // 令总节点个数为0

    // 注意，buckets vector并未释放掉空间，仍保有原来大小
}

template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::copy_from(const hashtable& ht) {
    // 先清除己方的buckets vector. 这操作是调用vector::clear. 将整个容器清空
    buckets.clear();
    // 为己方的buckets vector保留空间，使与对方相同
    // 如果己方空间大于对方，就不动，如果己方空间小于对方，就会增大
    buckets.reserve(ht.buckets.size());
    // 从己方的buckets vector尾端开始，插入n个元素，其值为null指针
    // 注意此时buckets vector为空，所以所谓尾端，就是起头处
    buckets.insert(buckets.end(), ht.buckets.size(); (node*)0);
    __STL_TRY {
        // 针对buckets vector
        for (size_type i=0; i<ht.buckets.size(); ++i) {
            // 复制vector的每一个元素（是个指针，指向hashtable节点）
            if (const node* cur = ht.buckets[i]) {
                node* copy = new_node(cur->val);
                buckets[i] = copy;

                // 针对同一个bucket list，复制每一个节点
                for (node* next = cur->next; next; cur = next, next = cur->next) {
                    copy->next = new_node(next->val);
                    copy = copy->next;
                }
            }
        }
        num_elements = ht.num_elements; // 重新登录节点个数（hashtable的大小）
    }
    __STL_UNWIND(clear());
}
```

```c++
// 以下定义于<stl_hash_fun.h>
template <class Key> struct hash {};

inline size_t __stl_hash_string(const char* s) {
    unsigned long h = 0;
    for (; *s; ++s)
        h = 5 * h + *s;

    return size_t(h);
}

// 以下所有的__STL_TEMPLATE_NULL，在<stl_config.h>中皆被定义为template<>

__STL_TEMPLATE_NULL struct hash<char*> {
    size_t operator()(const char* s) const { return __stl_hash_string(s); }
}

__STL_TEMPLATE_NULL struct hash<const char*> {
    size_t operator()(const char* s) const { return __stl_hash_string(s); }
}

__STL_TEMPLATE_NULL struct hash<char> {
    size_t operator()(char x) const { return x; }
}

__STL_TEMPLATE_NULL struct hash<unsigned char> {
    size_t operator()(unsigned char x) const { return x; }
}

__STL_TEMPLATE_NULL struct hash<signed char> {
    size_t operator()(unsigned char x) const { return x; }
}

__STL_TEMPLATE_NULL struct hash<short> {
    size_t operator()(short x) const { return x; }
}

__STL_TEMPLATE_NULL struct hash<unsigned short> {
    size_t operator()(unsigned short x) const { return x; }
}

__STL_TEMPLATE_NULL struct hash<int> {
    size_t operator()(int x) const { return x; }
}

__STL_TEMPLATE_NULL struct hash<unsigned int> {
    size_t operator()(unsigned int x) const { return x; }
}

__STL_TEMPLATE_NULL struct hash<long> {
    size_t operator()(long x) const { return x; }
}

__STL_TEMPLATE_NULL struct hash<unsigned long> {
    size_t operator()(unsigned long x) const { return x; }
}
```
