# Adapter

我以前经常旅行，旅行适配器可以让我把欧洲的插头插入英国或美国的插座，这与适配器模式是一个很好的类比：我们得到了一个接口，但我们想要一个不同的接口，在接口上建立一个适配器就可以让我们到达我们想要到达的地方。

## Scenario

这里有一个微不足道的例子：假设你正在使用一个擅长绘制像素的库。而你的工作是处理几何对象————线条、矩形等。你想继续使用这些对象，但也需要渲染，因此你需要将几何图形调整为基于像素的表示法。

首先，让我们定义一下示例中的域对象（相当简单）：

```c++
struct Point
{
    int x, y;
};

struct Line
{
    Point start, end;
};
```

现在让我们从理论上谈谈矢量几何。典型的矢量对象可能是由线条对象集合定义的。与其从 `vector<Line>` 继承，我们可以直接定义一对纯虚拟迭代器方法：

```c++
struct VectorObject
{
    virtual std::vector<Line>::iterator begin() = 0;
    virtual std::vector<Line>::iterator end() = 0;
};
```

因此，如果您想定义一个矩形，您可以在`vector<Line>-typed`字段中保留大量线条，并简单地暴露其端点：

```c++
struct VectorRectangle : VectorObject
{
    VectorRectangle(int x, int y, int width, int height)
    {
        lines.emplace_back(Line{ Point{x, y}, Point{x + width, y} });
        lines.emplace_back(Line{ Point{x + width, y}, Point{x + width, y + hegiht} });
        lines.emplace_back(Line{ Point{x, y}, Point{x, y + hegiht} });
        lines.emplace_back(Line{ Point{x, y + height}, Point{x + width, y + hegiht} });
    }

    std::vector<Line>::iterator begin() override {
        return lines.begin();
    }
    std::vector<Line>::iterator end() override {
        return lines.end();
    }
private:
    std::vector<Line> lines;
};
```

现在，设置是这样的。假设我们想在屏幕上画线。甚至是矩形！不幸的是，我们不能，因为唯一的绘图界面就是这个：

```C++
void DrawPoints(CPaintDC& dc, std::vector<Point>::iterator start, std::vector<Point>::iterator end)
{
    for (auto i = start; i != end; ++i)
        dc.SetPixel(i->x, i->y, 0);
}
```

我在这里使用的是 MFC（微软基础类）中的 CPaintDC 类，但这不是重点。重点是我们需要像素。而我们只有线条。我们需要一个适配器。

## Adapter

好吧，假设我们想画几个矩形：

```c++
vector<shared_ptr<VectorObject>> vectorObjects{
    make_shared<VectorRectangle>(10, 10, 100, 100),
    make_shared<VectorRectandle>(30, 30, 60, 60)
}
```

为了绘制这些对象，我们需要将每一个对象从一系列线段转换成大量的点。为此，我们制作了一个单独的类来存储点，并以一对迭代器的形式显示出来：

```c++
struct LineToPointAdapter
{
    typedef vector<Point> Points;

    LineToPointAdapter(Line& line)
    {
        // TODO
    }

    virtual Points::iterator begin() { return points.begin(); }
    virtual Points::iterator end() { return points.end(); }
private:
    Points points;
};
```

从直线到点数的转换就发生在构造函数中，因此适配器是急需的。转换的实际代码也相当简单：

```c++
LineToPointAdapter(Line& line)
{
    int left = min(line.start.x, line.end.x);
    int right = max(line.start.x, line.end.x);
    int top = min(line.start.y, line.end.y);
    int bottom = max(line.start.y, line.end.y);
    int dx = right - left;
    int dy = line.end.y - line.start.y;

    // only vertical or horizontal lines
    if (dx == 0)
    {
        // vertical
        for (int y = top; y <= bottom; ++y)
        {
            points.emplace_back(Point{ left, y });
        }
    }
    else if (dy == 0)
    {
        for (int x = left; x <= right; ++x)
        {
            points.emplace_back(Point{ x, top });
        }
    }
}
```

前面的代码非常琐碎：我们只处理完全垂直或水平的线条，而忽略其他一切。现在，我们可以使用此适配器实际渲染一些对象。我们从示例中提取两个矩形，然后像这样简单地渲染它们：

```c++
for (auto& obj : vectorObjects)
{
    for (auto& line : *obj)
    {
        LineToPointAdapter lpo{ line };
        DeawPoints(dc, lpo.begin(), lpo.end());
    }
}
```

漂亮 我们所要做的就是，为每个矢量对象获取其每条直线，为该直线构建一个 `LineToPointAdapter`，然后遍历由该适配器生成的点集，并将它们输入到 `DrawPoints()` 中。这就成功了！（相信我，确实有效。）

## Adapter Temporaries

不过，我们的代码存在一个大问题：`DrawPoints()`在我们可能需要的每次屏幕刷新时都会被调用，这意味着同一行对象的相同数据会被适配器重新生成无数次。我们能做些什么呢？

一方面，我们可以在应用程序启动时预先定义所有的点：

```c++
vector<Point> points;
for (auto& o : vectorObjects)
{
    for (auto& l : *o)
    {
        LineToPointAdapter lpo{ l };
        for (auto& P : lpo)
            points.push_back(p);
    }
}
```

然后，`DrawPoints()`的实现就简化为

```c++
DrawPoints(dc, points.begin(), points.end());
```

但是，让我们假设一下，矢量对象的原始集合可能会发生变化。那么缓存这些点就没有意义了，但我们仍然希望避免不断再生可能重复的数据。我们该如何处理呢？当然是使用缓存！

首先，为了避免再生，我们需要独特的方法来识别线，这也意味着我们需要独特的方法来识别点。ReSharper的**Generate | Hash函数**可以帮到我们：

```c++
struct Point
{
    int x, y;

    friend std::size_t hash_value(const Point& obj)
    {
        std::size_t seed = 0x725C686F;
        boost::hash_combine(seed, obj.x);
        boost::hash_combine(seed, obj.y);
        return seed;
    }
};

struct Line
{
    Point start, end;

    friend std::size_t hash_value(const Line& obj)
    {
        std::size_t seed = 0x719E6B16;
        boost::hash_combine(seed, obj.start);
        boost::hash_combine(seed, obj.end);
        return seed;
    }
};
```

在前面的示例中，我选择了Boost的hash实现。现在，我们可以创建一个新的`LineToPointCachingAdapter`，它可以缓存点，只有在必要时才重新生成。除了以下细微差别外，实现方式几乎相同。

首先，适配器现在有了缓存：

```c++
static map<size_t, Points> cache;
```

这里的`size_t`类型正是Boost的hash函数返回的类型。现在，在迭代生成的点时，我们按如下方式生成它们：

```c++
virtual Points::iterator begin() { return cache[line_hash].begin(); }
virtual Points::iterator end() { return cache[line_hash].end(); }
```

这就是算法的有趣之处：在生成点之前，我们要检查它们是否已经生成。如果已经生成，我们就退出；如果还没有，我们就生成它们并添加到缓存中：

```c++
LineToPointCachingAdapter(Line& line)
{
    static boost::hash<Line> hash;
    line_hash = hash(line); // note: line_hash is a field!
    if (cache.find(line_hash) != cache.end())
        return; // we already have it

    Points points;

    // same code as before

    cache[line_hash] = points;
}
```

耶！多亏了哈希函数和缓存，我们大大减少了转换的次数。剩下的唯一问题就是在不再需要旧点后如何将其删除。这个具有挑战性的问题留给读者自己去解决。

## Summary

适配器是一个非常简单的概念：它允许你将现有的接口适配到你需要的接口。适配器唯一真正的问题是，在适配过程中，有时会产生临时数据，以满足其他数据表示的需要。当这种情况发生时，就需要使用缓存：确保只有在必要时才生成新数据。对了，如果你想在缓存对象发生变化时清理陈旧数据，还需要做更多的工作。

我们还没有真正解决的另一个问题是懒汉模式：当前的适配器实现会在创建后立即执行转换。如果只想在实际使用适配器时才进行转换呢？这很容易做到，留待读者练习。
