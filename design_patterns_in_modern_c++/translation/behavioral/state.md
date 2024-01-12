# state

我必须承认：我的行为受我的状态支配。如果我睡眠不足，我就会有点累。如果我喝了酒，我就不会开车。所有这些都是状态，它们支配着我的行为：我的感觉如何，我能做什么，不能做什么。

当然，我可以从一种状态过渡到另一种状态。我可以去喝杯咖啡，这会让我从昏昏欲睡转为精神抖擞（我希望如此！）。因此，我们可以把咖啡看作是一个触发器，它能让你从困倦过渡到警觉。在这里，让我笨拙地为你说明一下：

```c++
        coffee
sleepy --------> alert
```

因此，状态设计模式是一个非常简单的想法：状态控制行为；状态可以改变；唯一不确定的是由谁来触发状态改变。

从根本上说，有两种方法：

- 状态是具有行为的实际类，这些行为将实际状态从一个切换到另一个。
- 状态和转换只是枚举。我们有一个叫做状态机的特殊组件来执行实际的转换。

这两种方法都是可行的，但实际上第二种方法才是最常见的。我们将对这两种方法都进行研究，但我必须承认，我会对第一种方法略过不看，因为人们通常不是这样做的。

## State-Driven State Transitions

我们先来看一个最微不足道的例子：电灯开关。它只能处于开和关两种状态。我们将建立一个任何状态都能切换到其他状态的模型：虽然这反映了状态设计模式的 "经典 "实现（根据 GoF 一书），但我并不推荐这样做。

首先，我们来建立电灯开关的模型：它只有一个状态和一些从一个状态切换到另一个状态的方法：

```c++
class LightSwitch
{
    State *state;
public:
    LightSwitch()
    {
        state = new OffState();
    }
    void set_state(State* state)
    {
        this->state = state;
    }
};
```

这一切看起来都非常合理。现在我们可以定义状态了，在这种特殊情况下，状态将是一个实际的类：

```c++
struct State
{
    virtual void on(LightSwitch *ls)
    {
        cout << "Light is already on\n";
    }

    virtual void off(LightSwitch *ls)
    {
        cout << "Light is already off\n"
    }
};
```

这种实现远非直观，以至于我们需要慢慢地、仔细地讨论它，因为从一开始，"State "类就没有任何意义。

首先，State 不是抽象的！你会认为一个你没有办法（或理由！）达到的状态是抽象的。但事实并非如此。

其次，`State`允许从一种状态切换到另一种状态。这......对一个有理智的人来说，毫无意义。想象一下电灯开关：是开关改变了状态。状态本身是不会改变的，但它似乎就是这样做的。

第三，也许最令人困惑的是，`State::on/off` 的默认行为声称我们已经处于这种状态！在我们实现示例的其余部分时，这些问题就会迎刃而解。

现在我们来实现On和Off状态：

```c++
struct OnState : State
{
    OnState() { cout << "Light turned on\n"; }
    void off(LightSwitch* ls) override;
};

struct OffState : State
{
    OffState() { cout << "Light turned off\n"; }
    void on(LightSwitch* ls) override;
};
```

`OnState::off` 和 `OffState::on` 的实现允许状态本身切换到另一种状态！这就是它的样子：

```c++
void OnState::off(LightSwitch* ls)
{
    cout << "Switching light off...\n";
    ls->set_state(new OffState());
    delete this;
}   // same for OffState::on
```

这就是切换发生的地方。这个实现包含了奇怪的 `delete this` 调用，这在现实世界的 C++ 中并不常见。`delete this` 对状态的初始分配位置做了非常危险的假设。这个示例可以用智能指针重写，但使用指针和堆分配清楚地表明，状态在这里被主动销毁。如果状态有一个析构函数，它就会触发，你就会在这里执行额外的清理。

当然，我们确实希望开关本身也能切换状态，看起来就像这样：

```c++
class LightSwitch
{
    ...
    void on() { state->on(this); }
    void off() { state->off(this); }
};
```

因此，综合上述情况，我们可以运行以下方案：

```c++
LightSwitch ls; // Light turned off
ls.on();        // Switching light on...
                // Light turned on
ls.off();       // Switching light off...
                // Light turned off
ls.off();       // Light is already off
```

我必须承认：我不喜欢这种方法，因为它不直观。当然，状态可以被告知（观察者模式）我们正在进入它。但状态自行切换到另一个状态的想法--也就是 GoF 书中 State 模式的 "经典 "实现--似乎并不特别讨人喜欢。

如果我们要笨拙地说明从 "OffState "到 "OnState "的转换，那么需要说明的是：

```c++
          LightSwitch::on() -> OffState::on()
OffState -------------------------------------> OnState
```

另一方面，从 OnState 到 OnState 的转换使用的是基本 State 类，也就是告诉你已经处于该状态的那个类：

```c++
         LightSwitch::on() -> State::on()
OnState -------------------------------------> OnState
```

这里所举的例子可能显得特别矫揉造作，因此我们现在来看看另一个手工制作的例子，在这个例子中，状态和转换被简化为枚举式成员。

## Handmade State Machine

让我们试着定义一个典型电话通话的状态机。首先，我们将描述电话的状态：

```c++
enum class State
{
    off_hook,
    connecting,
    connected,
    on_hold,
    on_hook
};
```

现在，我们还可以定义状态之间的转换，同样作为一个枚举类：

```c++
enum class Trigger
{
    call_dialed,
    hung_up,
    call_connected,
    placed_on_hold,
    taken_off_hold,
    left_message,
    stop_using_phone
};
```

现在，这个状态机的确切规则，即哪些转换是可能的，需要存储在某个地方。

```c++
map<State, vector<pair<Trigger, State>>> rules;
```

这有点笨拙，但本质上，map的key是我们正在转移的状态，value是一组Trigger-State对，代表在此状态下可能触发的触发器，以及使用触发器时移动到的状态。

让我们初始化这个数据结构：

```c++
rules[State::off_hook] = {
    {Trigger::call_dialed, State::connecting},
    {Trigger::stop_using_phone, State::on_hook}
};

rules[State::connecting] = {
    {Trigger::hung_up, State::off_hook},
    {Trigger::call_connected, State::connected}
};
// more rules here
```

我们还需要一个起始状态，如果我们希望状态机在达到该状态后停止执行，还可以添加一个退出（结束）状态：

```c++
State currentState{ State::off_hook },
exitState{ State::on_hook };
```

这样，我们就不一定要为实际运行（我们使用的术语是协调）状态机而构建一个单独的组件了。例如，如果我们想建立一个交互式的电话模型，我们可以这样做：

```c++
while(true)
{
    cout << "The phone is currently " << currentState << endl;
select_trigger:
    cout << "Select a trigger:" << "\n";

    int i = 0;
    for (auto item : rules[currentState])
    {
        cout << i++ << ". " << item.first << "\n";
    }

    int input;
    cin >> input;
    if (input < 0 || (input+1) > rules[currentState].size())
    {
        cout << "Incorrect option. Please try again." << "\n";
        goto select_trigger;
    }

    currentState = rules[currentState][input].second;
    if (currentState == exitState) break;
}
```

首先：是的，我确实使用了 goto，这个例子很好地说明了在什么情况下使用它是合适的。至于算法本身，这是很明显的：我们让用户在当前状态下选择一个可用的触发器（`State`和`Trigger`的`operator <<`已在幕后实现），如果触发器可用，我们就使用之前创建的规则图过渡到它。

如果我们到达的状态是退出状态，我们就跳出循环。下面是一个与程序交互的示例：

```c++
The phone is currently off the hook
Select a trigger:
1. call dialed
2. putting phone on hook
0
The phone is currently connecting
Select a trigger:
1. hung up
2. call connected
1
The phone is currently connected
Select a trigger:
1. left message
2. hung up
3. placed on hold
2
The phone is currently on hold
Select a trigger:
1. taken off hold
2. hung up
1
The phone is currently off the hook
Select a trigger:
1. call dialed
2. putting phone on hook
1
We are done using the phone
```

这种 hand-rolled 状态机的主要优点是非常容易理解：状态和转换是普通的枚举，转换集定义在一个简单的 std::map 中，开始和结束状态是简单的变量。

## State Machines with Boost.MSM

在现实世界中，状态机更为复杂。有时，你希望在达到某个状态时发生某些动作。有时，你希望转换是有条件的，也就是说，只有当某个条件成立时，转换才会发生。

使用 Boost 的状态机库 Boost.MSM（Meta State Machine）时，状态机是一个通过 CRTP 继承自 state_machine_def 的类：

```c++
struct PhoneStateMachine : state_machine_def<PhoneStateMachine>
{
    bool angry{ false };
```

我添加了一个 bool，表示呼叫者是否生气（例如，被搁置）；我们稍后会用到它。现在，每个状态也可以驻留在状态机中，并继承于状态类：

```c++
struct OffHook : state<> {};
struct Connecting : state<>
{
    template <class Event, class FSM>
    void on_entry(Event const& evt, FSM&)
    {
        cout << "We are connecting..." << endl;
    }
    // also on_exit
};
// other states omitted
```

如你所见，状态也可以定义进入或退出特定状态时发生的行为。

你还可以定义在转换时（而不是在到达某个状态时）执行的行为：这些行为也是类，但它们不需要从任何东西继承；相反，它们需要提供具有特定签名的 `operator()`：

```c++
struct PhoneBeingDestroyed
{
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM&, SourceState&, TargetState&)
    {
        cout << "Phone breaks into a million pieces" << endl;
    }
};
```

正如你可能已经猜到的那样，参数提供了状态机的引用，以及你要去往的状态。

最后，我们还有保护条件：这些条件决定了我们是否可以首先使用转换。现在，我们的布尔变量 angry 并不是 MSM 可用的形式，因此我们需要对它进行封装：

```c++
struct CanDestroyPhone
{
    template <class EVT, class FSM, class SourceState, class TargetState>
    bool operator()(EVT const&, FSM& fsm, SourceState&, class TargetState)
    {
        return fsm.angry;
    }
};
```

前面的代码提供了一个名为`CanDestroyPhone`的保护条件，我们可以在以后定义状态机时使用它。

为了定义状态机规则，Boost.MSM 使用了 MPL（元编程库）。具体来说，过渡表定义为`mpl::vector`，每一行依次包含

- 源状态
- 过渡
- 目标状态
- 执行的可选操作
- 一个可选的保护条件

综上所述，我们可以定义一些电话呼叫规则如下：

```c++
struct transition_table : mpl::vector<
    Row<OffHook, CallDialed, Connecting>,
    Row<Connecting, CallConnected, Connected>,
    Row<Connected, PlacedOnHold, OnHold>,
    Row<OnHold, PhoneThrownIntoWall, PhoneDestroyed, PhoneBeingDestroyed, CanDestroyPhone>
> {};
```

在前文中，与状态不同，诸如 "CallDialed "之类的转换是可以在状态机类之外定义的类。它们不必从任何基类继承，也可以很容易地为空，但它们必须是类型。

我们的 `transition_table` 的最后一行是最有趣的：它规定我们只能在符合 `CanDestroyPhone` 防护条件的情况下尝试销毁手机，而当手机真正被销毁时，应该执行 `PhoneBeingDestroyed` 动作。

现在，我们还可以添加一些内容。首先，我们要添加起始条件：由于我们使用的是 `Boost.MSM`，所以起始条件是一个`typedef`，而不是一个变量：

```c++
typedef OffHook initial_state;
```

最后，如果没有可能的转换，我们可以定义一个动作为发生。它可能发生！例如，当你砸碎手机后，你就不能再使用它了，对吗？

```c++
template <class FSM, class Event>
void no_transition(Event const& e, FSM&, int state)
{
    cout << "No transition from state " << state_names[state] << " on event " << typeid(e).name() << endl;
}
```

Boost MSM 将状态机分为前端（也就是我们刚才写的）和后端（运行状态机的部分）。使用back-end API，我们可以根据前面的状态机定义构建状态机：

```c++
msm::back::state_machine<PhoneStateMachine> phone;
```

现在，假设 `info()` 函数的存在只是为了打印我们所处的状态，我们可以尝试协调以下场景：

```c++
info(); // The phone is currently off hook
phone.process_event(CallDialed{}); // We are connecting...
info(); // The phone is currently connecting
phone.process_event(CallConnected{});
info(); // The phone is currently connected
phone.process_event(PlacedOnHold{});
info(); // The phone is currently on hold

phone.process_event(PhoneThrownIntoWall{});
// Phone breaks into a million pieces

info(); // The phone is currently destroyed

phone.process_event(CallDialed{});
// No transition from state destroyed on event struct CallDialed
```

因此，这就是如何定义一个更复杂、更具行业实力的状态机。

## Summary

首先，值得强调的是，Boost.MSM 是 Boost 中两个可供选择的状态机实现之一，另一个是 Boost.Statechart。我确信还有很多其他的状态机实现。

其次，状态机的作用远不止于此。例如，许多库都支持*hierarchical*状态机的想法：比如，Sick状态可以*包含*许多不同的子状态，如 Flu 或 Chickenpox。如果你处于 Flu 状态，也就意味着你处于 Sick 状态。

最后，值得再次强调的是，现代状态机与最初形式的状态设计模式相去甚远。在我看来，存在重复的应用程序接口（例如， `LightSwitch::on/off` 与`State::on/off`以及自删除（self-deletion）都是明显的代码缺陷。别误会我的意思——这种方法是可行的，但它既不直观又繁琐。
