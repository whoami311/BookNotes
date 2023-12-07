# multimap

```c++
template <class Key, class T, class Compare = less<Key>, class Alloc = alloc>
class multimap {
public:
    // typedefs:
    ... （与set相同）

    // allocation/deallocation
    // 注意，multimap一定使用insert_equal()而不使用insert_unique()
    // mao才使用insert_unique()

    template <class InputIterator>
    multimap(InputIterator first, InputIterator last)
        : t(Compare()) { t.insert_equal(first, last); }
    
    template <class InputIterator>
    multimap(InputIterator first, InputIterator last, const Compare& comp)
        : t(comp) { t.insert_equal(first, last); }
    ... （其它与map相同）

    // insert/erase

    iterator insert(const value_type& x) {
        return t.insert_equal(x);
    }
    iterator insert(iterator position, const value_type& x) {
        return t.insert_equal(position, x);
    }
    template <class InputIterator>
    void insert(InputIterator first, InputIterator last) {
        t.insert_equal(first, last);
    }
    ... （其它与map相同）
};
```
