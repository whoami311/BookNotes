// book
class VisualComponent {
public:
    VisualComponent();

    virtual void Draw();
    virtual void Resize();
    // ...
}

class Decorator : public VisualComponent {
public:
    Decorator(VisualComponent*);

    virtual void Draw();
    virtual void Resize();
    // ...

private:
    VisualComponent* _component;
};

void Decorator::Draw() {
    _component->Draw();
}

void Decorator::Resize() {
    _component->Resize();
}

class BorderDecorator : public Decorator {
public:
    BorderDecorator(VisualComponent*, int borderWidth);

    virtual void Draw();

private:
    void DrawBorder(int);
private:
    int _width;
};

void BorderDecorator::Draw() {
    Decorator::Draw();
    DrawBorder(_width);
}

// 使用Decorator创建一个具有边界的可滚动TextView
void Window::SetContents(VisualComponent* content) {
    // ...
}

Window* window = new Window;
TextView* textView = new TextView;

window->SetContents(textView);

// 有边界的可以滚动的TextView
window->SetContents(new BorderDecorator(new ScrollDecorator(textView), 1));
