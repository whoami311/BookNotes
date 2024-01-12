# Adapter

I used to travel quite a lot, and a travel adapter that lets me plug a European plug into a UK or USA socket1 is a very good analogy to what’s going on with the Adapter pattern: we are given an interface, but we want a different one, and building an adapter over the interface is what gets us to where we want to be.

## Scenario

Here’s a trivial example: suppose you’re working with a library that’s great at drawing pixels. You, on the other hand, work with geometric objects—lines, rectangles, that sort of thing. You want to keep working with those objects but also need the rendering, so you need to adapt your geometry to pixel-based representation.

Let us begin by defining the (rather simple) domain objects of our example:

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

Let’s now theorize about vector geometry. A typical vector object is likely to be defined by a collection of Line objects. Instead of inheriting from a `vector<Line>`, we can just define a pair of pure virtual iterator methods:

```c++
struct VectorObject
{
    virtual std::vector<Line>::iterator begin() = 0;
    virtual std::vector<Line>::iterator end() = 0;
};
```

So this way, if you want to define, say, a Rectangle, you can keep a bunch of lines in a `vector<Line>-typed` field and simply expose its endpoints:

```c++
struct VectorRectangle : VectorObject
{
    VectorRectangle(int x, int y, int width, int height)
    {
        lines.emplace_back(Line{ Point{x,y}, Point{x + width, y} });
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

Now, here’s the set-up. Suppose we want to draw lines on screen. Rectangles, even! Unfortunately, we cannot, because the only interface for drawing is literally this:

```C++
void DrawPoints(CPaintDC& dc, std::vector<Point>::iterator start, std::vector<Point>::iterator end)
{
    for (auto i = start; i != end; ++i)
        dc.SetPixel(i->x, i->y, 0);
}
```

I’m using the CPaintDC class from MFC (Microsoft Foundation Classes) here, but that’s beside the point. The point is we need pixels. And we only have lines. We need an adapter.

## Adapter

All right, so let’s suppose we want to draw a couple of rectangles:

```c++
vector<shared_ptr<VectorObject>> vectorObjects{
    make_shared<VectorRectangle>(10, 10, 100, 100),
    make_shared<VectorRectandle>(30, 30, 60, 60)
}
```

In order to draw these objects, we need to convert every one of them from a series of lines into a rather large number of points. For this, we make a separate class that will store the points and expose them as a pair of iterators:

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

The conversion from a line to a number of points happens right in the constructor, so the adapter is eager. The actual code for the conversion is also rather simple:

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

The preceding code above trivial: we only handle perfectly vertical or horizontal lines and ignore everything else. We can now use this adapter to actually render some objects. We take the two rectangles from the examplee and simply render them like this:

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

Beautiful! All we do is, for every vector object, get each of its lines, construct a LineToPointAdapter for that line, and then iterate the set of points produced by the adapter, feeding them to DrawPoints(). And it works! (Trust me, it does.)

## Adapter Temporaries

There’s a major problem with our code, though: DrawPoints() gets called on literally every screen refresh that we might need, which means the same data for same line objects gets regenerated by the adapter, like, a zillion times. What can we do about it?

Well, on the one hand, we can predefine all the points at application start-up, for example:

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

and then the implementation of DrawPoints() simplifies to

```c++
DrawPoints(dc, points.begin(), points.end());
```

But let’s suppose, for a moment, that the original set of vectorObjects can change. Caching those points makes no sense then, but we still want to avoid the incessant regeneration of potentially repeating data. How do we deal with this? With caching, of course!

First of all, to avoid regeneration, we need unique ways of identifying lines, which transitively means we need unique ways of identifying points. ReSharper’s **Generate | Hash function** to the rescue:

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

In the preceding example, I’ve opted for Boost’s hash implementation. Now, we can build a new LineToPointCachingAdapter such that it caches the points and regenerates them only when necessary. The implementation is almost the same except for the following nuances.

First, the adapter now has a cache:

```c++
static map<size_t, Points> cache;
```

The type size_t here is precisely the type returned from Boost’s hash functions. Now, when it comes to iterating the points generated, we yield them as follows:

```c++
virtual Points::iterator begin() { return cache[line_hash].begin(); }
virtual Points::iterator end() { return cache[line_hash].end(); }
```

And here is the fun part of the algorithm: before generating the points we check whether they’ve been generated already. If they have, we just exit; if they haven’t, we generate them and add them to the cache:

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

Yay! Thanks to hash functions and caching, we’ve drastically cut down on the number of conversions being made. The only problem that remains is the removal of old points after they are no longer needed. This challenging problem is left as an exercise to the reader.

## Summary

Adapter is a very simple concept: it allows you to adapt the interface you have to the interface you need. The only real issue with adapters is that, in the process of adaptation, you sometimes end up generating temporary data so as to satisfy some other representation of data. And when this happens, turn to caching: ensuring that new data is only generated when necessary. Oh, and you’ll need to do a bit more work if you want to clean up stale data when the cached objects have changed.

Another concern that we haven’t really addressed is laziness: the current adapter implementation performs the conversion as soon as it is created. What if you only want the work to be done when the adapter is actually used? This is rather easy to do and is left as an exercise for the reader.
