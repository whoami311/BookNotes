# Observer

观察者模式是一种流行且必要的模式，因此令人惊讶的是，与其他语言（如 C#）不同，C++ 和标准库都没有提供现成的实现。尽管如此，一个安全、正确实现的观察者（如果有这种东西的话）在技术上是一个复杂的构造，因此在本章中我们将研究它的所有细节。

## Property Observers

人会变老。这是生活的事实。但当一个人又老了一岁时，我们可能会想在他生日时祝贺他。但怎么祝贺呢？根据以下定义:

```c++
struct Person
{
    int age;
    Person(int age) : age{age} {}
};
```

我们如何知道一个人的年龄何时发生变化？我们不知道。要看到变化，我们可以尝试轮询：每 100 毫秒读取一个人的年龄，然后将新值与旧值进行比较。这种方法可行，但繁琐且无法扩展。我们需要更聪明地处理这个问题。

我们知道，每次写入一个人的年龄字段时，我们都希望得到通知。要做到这一点，唯一的办法就是制作一个设置器：

```c++
struct Person
{
    int get_age() const { return age; }
    void set_age(const int value) { age = value; }
private:
    int age;
};
```

在 setter `set_age()`中，我们可以通知相关人员年龄已经发生变化。但怎么通知呢？

## Observer\<T\>

那么，一种方法是定义某种基类，任何对获取 Person 的变化感兴趣的人都需要继承该基类：

```c++
struct PersonListener
{
    virtual void person_changed(Person& p, const string& property_name, const any new_value) = 0;
};
```

然而，这种方法非常憋屈，因为属性更改可能发生在 Person 以外的其他类型上，我们不想为这些类型也生成额外的类。这里有一种更通用的方法：

```c++
template<typename T>
struct Observer
{
    virtual void field_changed(T& source, const string& field_name) = 0;
};
```

`field_changed()`中的两个参数希望不言自明。第一个参数是指向字段实际发生变化的对象的引用，第二个参数是字段的名称。是的，名称是以字符串形式传递的，这确实会影响代码的可重构性（如果字段名称改变了怎么办？）

这种实现方式可以让我们观察到 Person 类的变化，并将其写入命令行：

```c++
struct ConsolePersonObserver : Observer<Person>
{
    void field_changed(Person& source, const string& field_name) override
    {
        cout << "Person's " << field_name << " has changed to " << source.get_age() << ".\n";
    }
};
```

例如，我们在方案中引入的灵活性允许我们观察多个类的属性变化。例如，如果我们将 `Creature` 类加入其中，现在就可以同时观察这两个类：

```c++
struct ConsolePersonObserver : Observer<Person>, Observer<Creature>
{
    void field_changed(Person& source, ...) { ... };
    void field_changed(Creature& source, ...) { ... };
}
```

另一种方法是使用 std::any，而不用泛型实现。试试看

## Observable\<T\>

无论如何，让我们回到 Person 的话题上来。由于 Person 即将成为一个可观察的类，它必须承担新的责任，即

- 为所有对 Person 的变化感兴趣的观察者保存一个私有列表
- 让观察者subscribe()/unsubscribe() Person 的变化
- 使用 notify() 通知所有观察者实际发生的变化

所有这些功能都可以很方便地转移到一个单独的基类中，以避免在每个潜在的观察对象中进行复制：

```c++
template <typename T>
struct Observable
{
    void notify(T& source, const string& name) { ... }
    void subscribe(Observer<T>* f) { observers.push_back(f); }
    void unsubscribe(Observer<T>* f) { ... }
private:
    vector<Observer<T>*> observers;
};
```

我们已经实现了 subscribe()——它只是将一个新的观察者添加到私有的观察者列表中。观察者列表对任何人都不可用——甚至对派生类也不可用。我们不希望有人随意操作这个集合。

接下来，我们需要实现 notify()。这个想法很简单：逐个检查每个观察者并调用其 `field_changed()`：

```c++
void notify(T& source, const string& name)
{
    for (auto obs : observers)
        obs->field_changed(source, name);
}
```

但仅仅继承自 Observable\<T\> 还不够：我们的类还需要在字段发生变化时调用 `notify()` 来完成自己的工作。

以 setter `set_age()` 为例。它现在有三个职责

- 检查名称是否真的发生了变化。如果 age 是 20，而我们赋值给它的是 20，那么执行任何赋值或通知都没有意义。
- 将字段赋值给适当的值。
- 使用正确的参数调用 `notify()`。

因此，`set_age()` 的新实现将如下所示：

```c++
struct Person : Observable<Person>
{
    void set_age(const int age)
    {
        if (this->age == age) return ;
        this->age = age;
        notify(*this, "age");
    }
private:
    int age;
};
```

## Connecting Observers and Observables

现在，我们可以开始使用我们创建的基础架构，以获取有关Person的字段更改的通知（其实，我们也可以称它们为属性）。下面提醒大家我们的观察者是什么样的：

```c++
struct ConsolePersonObserver : Observer<Person>
{
    void field_changed(Person& source, const string& field_name) override
    {
        cout << "Person's " << field_name << " has changed to " << source.get_age() << ".\n";
    }
};
```

我们是这样使用它的：

```c++
Person p{ 20 };
ConsolePersonObserver cpo;
p.suscribe(&cpo);
p.set_age(21); // Person's age has changed to 21.
p.set_age(22); // Person's age has changed to 22.
```

只要你不关心属性依赖性和线程安全/重溯等问题，你就可以到此为止，接受这个实现并开始使用它。如果你想了解更复杂的方法，请继续阅读。

## Dependency Problems

年满 16 周岁（在您的国家可能有所不同）的人可以投票。因此，假设我们希望在某人的投票权发生变化时收到通知。首先，假设我们的 Person 类型具有以下getter：

```c++
bool get_can_vote() const { return age >= 16; }
```

请注意，`get_can_vote()` 没有后盾字段，也没有setter（我们可以引入这样一个字段，但它显然是多余的），但我们也觉得有义务对它进行`notify()`。但怎么做呢？我们可以试着找出导致 `can_vote()` 发生变化的原因......没错，就是 `set_age()`！因此，如果我们想要收到投票状态变化的通知，就需要在 `set_age()`中完成。准备好，你会有一个惊喜！

```c++
void set_age(int value) const
{
    if (age == value) return;

    auto old_can_vote = can_vote(); // store old value
    age = value;
    notify(*this, "age");

    if (old_can_vote != can_vote()) // check value has changed
        notify(*this, "can_vote");
}
```

前面的功能太多了。我们不仅要检查年龄是否发生变化，还要检查 can_vote 是否发生变化并发出通知！你也许能猜到这种方法并不能很好地扩展，对吗？想象一下，can_vote 依赖于两个字段，比如年龄和公民身份，这意味着这两个字段的设置者都必须处理 can_vote 通知。如果年龄还会影响其他十个属性呢？这种解决方案是行不通的，因为变量之间的关系需要手动跟踪，这将导致代码脆弱且无法维护。

说白了，在前面的方案中，can_vote 是 age 的从属属性。依赖属性所带来的挑战本质上就是 Excel 等工具所面临的挑战：如果不同单元格之间存在大量依赖关系，那么当其中一个单元格发生变化时，如何知道应该重新计算哪些单元格。

当然，属性依赖关系可以形式化为某种 `map<string, vector<string>>`，它将保存受某个属性影响的属性列表（或者反过来，保存所有影响某个属性的属性列表）。可悲的是，这个映射必须手工定义，而保持它与实际代码同步又相当棘手。

## Unsubscription and Thread Safety

我忽略了一件事，那就是观察者如何从观察对象中`unsubscribe()`。一般来说，你想把自己从观察者列表中移除，这在单线程场景中非常简单：

```c++
void unsubscribe(Observer<T>* observer)
{
    observer.earse(remove(observers.begin(), observers.end(), observer), observers.end());
};
```

虽然使用erase-remove成语在技术上是正确的，但它只在单线程情况下才是正确的。`std::vector`不是线程安全的，因此同时调用`subscribe()`和`unsubscribe()`可能会导致意想不到的后果，因为这两个函数都会修改vector。

这个问题很容易解决：只需在 observable 的所有操作上加锁即可。这看起来很简单：

```c++
template <typename T>
struct Observable
{
    void notify(T& source, const string& name)
    {
        scope_lock<mutex> lock{ mtx };
        ...
    }
    void subscribe(Observer<T>* f)
    {
        scope_lock<mutex> lock{ mtx };
        ...
    }
    void unsubscribe(Observer<T>* o)
    {
        scope_lock<mutex> lock{ mtx };
        ...
    }
private:
    vector<Observer<T>*> observers;
    mutex mtx;
};
```

另一种非常可行的方法是使用类似 TPL/PPL 中的 `concurrent_vector`。当然，这样做会失去排序保证（换句话说，一个接一个地添加两个对象并不能保证它们按顺序得到通知），但它肯定能让你不必自己管理锁。

## Reentrancy

最后一种实现方式通过在有人需要时锁定三个关键方法中的任何一个，提供了一定的线程安全性。但现在让我们想象一下下面的场景：你有一个 TrafficAdministration 组件，它一直监控着一个人，直到他们到了可以开车的年龄。当他们达到 17 岁时，组件就会取消订阅：

```c++
struct TrafficAdministration : Observer<Person>
{
    void TrafficAdministration::field_changed(Person& source, const string& filed_name) override
    {
        if (field_name == "age")
        {
            if (source.get_age() < 17)
                cout << "Whoa there, you are not old enough to drive!\n";
            else
            {
                // oh, ok, they are old enough, let's not monitor them anymore
                cout << "We no longer care!\n";
                source.unsubscribe(this);
            }
        }
    }
};
```

这是一个问题，因为当年龄变为 17 岁时，整个呼叫链将会是这样：

```c++
notify() --> field_changed() --> unsubscribe()
```

这是一个问题，因为在 `unsubscribe()` 中，我们最终会试图获取一个已经被占用的锁。这是一个重入问题。有不同的方法来处理这个问题。

- 一种方法是干脆禁止这种情况。毕竟，至少在这种特殊情况下，很明显这里发生了重入。
- 另一种方法是放弃从集合中删除元素的想法。相反，我们可以采用类似的方法：

```c++
void unsubscribe(Observer<T>* o)
{
    auto it != find(observers.begin(), observers.end(), o);
    if (it != observers.end())
        *it = nullptr;  // cannot do this for a set
}
```

随后，在使用`notify()`时，只需进行额外检查即可：

```c++
void notify(T& source, const string& name)
{
    for (auto obs : observers)
    {
        if (obs)
            obs->field_changed(source, name);
    }
}
```

当然，前面提到的只是解决了 `notify()` 和 `subscribe()` 之间可能存在的争用问题。举例来说，如果你同时进行 `subscribe()` 和 `unsubscribe()` 操作，这仍然是对集合的并发修改，而且仍然可能失败。因此，你可能至少想在这里加个锁。

还有一种方法是在 `notify()` 中复制整个集合。你仍然需要锁，只是不把它应用于 nofications。我的意思是：

```c++
void notify(T& source, const string& name)
{
    vector<Observer<T>*> observers_copy;
    {
        lock_guard<mutex_t> lock{ mtx };
        observers_copy = observers;
    }
    for (auto obs : observers_copy)
        if (obs)
            obs->field_changed(source, name);
}
```

在前面的实现中，我们确实使用了锁，但当我们调用 field_changed 时，锁已经被释放了，因为它只是在用于复制向量的人工作用域中创建的。我并不担心这里的效率，因为一个指针向量并不会占用那么多内存。

最后，用`recursive_mutex`替换`mutex`也是可行的。一般来说，大多数开发人员都讨厌递归互斥器（SO 上的证明），这不仅仅是因为它对性能的影响，更多的是因为在大多数情况下（就像观察者的例子一样），如果你的代码设计得更好一些，你可以使用普通的、非递归的变体。

还有一些有趣的实际问题我们没有在此讨论。这些问题包括：

- 如果同一个观察者被添加两次，会发生什么情况？
- 如果我允许重复的观察者，`ubsubscribe()` 会删除每个实例吗？
- 如果我们使用不同的容器，行为会受到什么影响？例如，我们决定使用 `std::set` 或 `boost::unordered_set` 来防止重复，这对普通操作意味着什么？
- 如果我想要按优先级排序的observers怎么办？

只要基础扎实，这些问题和其他实际问题都是可以解决的。在此，我们不再花时间讨论这些问题。

## Observer via Boost.Signals2

观察者模式有许多预打包的实现，其中最著名的可能是 Boost.Signals2 库。本质上，该库提供了一种名为 signal 的类型，用 C++ 术语表示信号（其他地方称为事件）。可以通过提供函数或 lambda 来订阅该信号。它也可以被取消订阅，当你想就此发出通知时，它就会被触发。

使用 Boost.Signals2，我们可以定义 Observer\<T\> 如下：

```c++
template <typename T>
struct Observable
{
    signal<void(T&, const string&)> property_changed;
};
```

其调用过程如下：

```c++
struct Person : Observable<Person>
{
    ...
    void set_age(const int age)
    {
        if (this->age == age) return;

        this->age = age;
        property_changed(*this, "age");
    }
};
```

API 的实际使用将直接使用信号，当然，除非您决定添加更多的 API 陷阱来简化使用：

```c++
Person p{123};
auto conn = p.property_changed.connect([](Person&, const string& prop_name)
{
    cout << prop_name << " has been changed" << endl;
});
p.set_age(20);  // name has been changed

// later, optionally
conn.disconnect();
```

`connect()` 调用的结果是一个连接对象，当你不再需要信号通知时，也可以用它来取消订阅。

## Summary

毫无疑问，本章中介绍的代码是一个明显的例子，它过度思考和过度设计了一个问题，远远超出了大多数人想要实现的目标。

让我们回顾一下实现 Observer 时的主要设计决策：

- 决定您希望观察结果传达哪些信息。例如，如果要处理字段/属性的更改，可以包含属性名称。您也可以指定新值或旧值，但传递类型可能会有问题。
- 你希望你的观察者是轮胎类，还是只希望有一个虚拟函数列表？
- 如何处理观察者取消订阅？
    - .如果你不打算支持取消订阅，那么恭喜你，你将省去很多实施观察者的工作，因为在重入场景中没有移除问题。
    - 如果你计划支持一个显式的 unsubscribe() 函数，你可能不想在函数中直接erase-remove，而是先标记要移除的元素，然后再移除它们。
    - 如果不喜欢对原始指针（可能为空）进行分派，可以考虑使用 `weak_ptr` 代替。
- 一个 Observer\<T\> 的功能可能会被多个不同的线程调用吗？如果是这样，你就需要保护你的订阅列表：
    - 您可以在所有相关函数上设置一个 `scoped_lock`；或者
    - 你可以使用线程安全的集合，如 TBB/PPL 的`concurrent_vector`。但会失去排序保证。
- 是否允许来自同一源的多个订阅？如果允许，就不能使用 `std::set`。

遗憾的是，没有一种理想的 Observer 实现方式能满足所有要求。无论采用哪种实现方式，都需要做出一些妥协。
