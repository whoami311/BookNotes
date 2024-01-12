# state

I must confess: my behavior is governed by my state. If I didn’t get enough sleep, I’m going to be a bit tired. If I had a drink, I wouldn’t get behind the wheel. All of these are states and they govern my behavior: how I feel, and what I can and cannot do.

I can, of course, transition from one state to another. I can go get a coffee, and this will take me from sleepy to alert (I hope!). So we can think of coffee as a trigger that causes a transition of yours truly from sleepy to alert. Here, let me clumsily illustrate it for you:

```c++
        coffee
sleepy --------> alert
```

So, the State design pattern is a very simple idea: state controls behavior; state can be changed; the only thing that the jury is out on is who triggers the state change.

There are, fundamentally, two ways:

- States are actual classes with behaviors, and these behaviors switch the actual state from one to another.
- States and transitions are just enumerations. We have a special component called a state machine that performs the actual transitions.

Both of these approaches are viable, but it’s really the second approach that is the most common. We’ll take a look at both of them, but I must admit I’ll glance over the first one, since this isn’t how people typically do things.

## State-Driven State Transitions

We’ll begin with the most trivial example out there: a light switch. It can only be in the on and off states. We’re going to build a model where any state is capable of switching to some other state: while this reflects the “classic” implementation of the State design pattern (as per the GoF book), it’s not something I’d recommend.

First, let’s model the light switch: all it has is a state and some means of switching from one state to another:

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

This all looks perfectly reasonable. We can now define the State, which in this particular case, is going to be an actual class:

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

This implementation is far from intuitive, so much so that we need to discuss it slowly and carefully, because from the outset, nothing about the `State` class makes sense.

First of all, State is not abstract! You’d think that a state you have no way (or reason!) of reaching would be abstract. But it’s not.

Second, `State` allows the switching from one state to another. This… to a reasonable person, it makes no sense. Imagine the light switch: it’s the switch that changes states. The state itself isn’t expected to change itself, and yet it appears this is exactly what it does.

Third, perhaps most bewildering, the default behavior of `State::on/off` claims that we are already in this state! This will come together, somewhat, as we implement the rest of the example.

We now implement the On and Off states:

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

The implementations of OnState::off and OffState::on allow the state itself to switch itself to another state! Here’s what it looks like:

```c++
void OnState::off(LightSwitch* ls)
{
    cout << "Switching light off...\n";
    ls->set_state(new OffState());
    delete this;
}   // same for OffState::on
```

So this is where the switching happens. This implementation contains the bizarre invocation of delete this, something you don’t often see in real-world C++. delete this makes a very dangerous assumption of where the state is initially allocated. The example could be rewritten with, say, smart pointers, but using pointers and heap allocation highlights clearly that the state is being actively destroyed here. If the state had a destructor, it would trigger and you would perform additional cleanup here.

Of course, we do want the switch itself to switch states too, which looks like this:

```c++
class LightSwitch
{
    ...
    void on() { state->on(this); }
    void off() { state->off(this); }
};
```

So, putting it all together, we can run the following scenario:

```c++
LightSwitch ls; // Light turned off
ls.on();        // Switching light on...
                // Light turned on
ls.off();       // Switching light off...
                // Light turned off
ls.off();       // Light is already off
```

I must admit: I don’t like this approach, because it is not intuitive. Sure, the state can be informed (Observer pattern) that we’re moving into it. But the idea of state switching itself to another state—which is the ‘classic’ implementation of the State pattern as per the GoF book—doesn’t seem particularly palatable.

If we were to clumsily illustrate a transition from `OffState` to `OnState`, it needs to be illustrated as:

```c++
          LightSwitch::on() -> OffState::on()
OffState -------------------------------------> OnState
```

On the other hand, the transition from OnState to OnState uses the base State class, the one that tells you that you are already in that state:

```c++
         LightSwitch::on() -> State::on()
OnState -------------------------------------> OnState
```

The example presented here may seem particularly artificial, so we are now going to look at another handmade set-up, one where the states and transitions are reduced to enumeration members.

## Handmade State Machine

Let’s try to define a state machine for a typical phone conversation. First of all, we’ll describe the states of a phone:

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

We can now also define transitions between states, also as an enum class:

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

Now, the exact rules of this state machine, that is, what transitions are possible, need to be stored somewhere.

```c++
map<State, vector<pair<Trigger, State>>> rules;
```

So this is a little clumsy, but essentially the key of the map is the State we’re moving from, and the value is a set of Trigger-State pairs representing possible triggers while in this state and the state you move into when you use the trigger.

Let’s initialize this data structure:

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

We also need a starting state, and we can also add an exit (terminal) state if we want the state machine to stop executing once that state is reached:

```c++
State currentState{ State::off_hook },
exitState{ State::on_hook };
```

Having made this, we don’t necessarily have to build a separate component for actually running (we use the term orchestrating) a state machine. For example, if we wanted to build an interactive model of the telephone, we could do it thus:

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

First of all: yes, I do use goto, this is a good illustration of where it’s appropriate. As for the algorithm itself, this is fairly obvious: we let the user select one of the available triggers on the current state (`operator <<` has been implemented for both State and Trigger behind the scenes) and, provided the trigger is valid, we transition to it by using that rules map that we created earlier.

And if the state we’ve reached is the exit state, we just jump out of the loop. Here’s a sample interaction with the program:

```c++
The phone is currently off the hook
Select a trigger:
0. call dialed
1. putting phone on hook
0
The phone is currently connecting
Select a trigger:
0. hung up
1. call connected
1
The phone is currently connected
Select a trigger:
0. left message
1. hung up
2. placed on hold
2
The phone is currently on hold
Select a trigger:
0. taken off hold
1. hung up
1
The phone is currently off the hook
Select a trigger:
0. call dialed
1. putting phone on hook
1
We are done using the phone
```

This hand-rolled state machine’s main benefit is that it is very easy to understand: states and transitions are ordinary enumerations, the set of transitions is defined in a simple std::map, and the start and end states are simple variables.

## State Machines with Boost.MSM

In the real world, state machines are more complicated. Sometimes, you want some action to occur when a state is reached. At other times, you want transitions to be conditional, that is, you want a transition to occur only if some condition holds.

When using Boost.MSM (Meta State Machine), a state machine library that’s part of Boost, your state machine is a class that inherits from state_machine_def via CRTP:

```c++
struct PhoneStateMachine : state_machine_def<PhoneStateMachine>
{
    bool angry{ false };
```

I’ve added a bool indicating whether the caller is angry (e.g., at being put on hold); we’ll use it a little bit later. Now, each state can also reside in the state machine, and is expected to inherit from the state class:

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

As you can see, the state can also define behaviors that happen when you enter or exit a particular state.

You can also define behaviors to be executed on a transition (rather than when you’ve reached a state): these are also classes, but they don’t need to inherit from anything; instead, they need to provide `operator()` with a particular signature:

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

As you may have guessed, the arguments give you references to the state machine and the states you’re going from and to.

Last, we have guard conditions: these dictate whether or not we can actually use a transition in the first place. Now, our Boolean variable angry is not in the form usable by MSM, so we need to wrap it:

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

The preceding makes a guard condition called CanDestroyPhone, which we can later use when we define the state machine.

For defining state machine rules, Boost.MSM uses MPL (metaprogramming library). Specifically, the transition table is defined as an mpl::vector with each row containing, in turn,

- The source state
- The transition
- The target state
- An optional action to execute
- An optional guard condition

So with all of that, we can define some phone-calling rules as follows:

```c++
struct transition_table : mpl::vector<
    Row<OffHook, CallDialed, Connecting>,
    Row<Connecting, CallConnected, Connected>,
    Row<Connected, PlacedOnHold, OnHold>,
    Row<OnHold, PhoneThrownIntoWall, PhoneDestroyed, PhoneBeingDestroyed, CanDestroyPhone>
> {};
```

In the preceding, unlike states, transitions such as `CallDialed` are classes that can be defined *outside* the state machine class. They don’t have to inherit from any base class, and can easily be empty, but they do have to be types.

The last row of our `transition_table` is the most interesting: it specifies that we can only attempt to destroy phone subject to the `CanDestroyPhone` guard condition, and when the phone is actually being destroyed, the `PhoneBeingDestroyed` action should be executed.

Now, there are a couple more things we can add. First, we add the starting condition: since we’re using `Boost.MSM`, the starting condition is a typedef, not a variable:

```c++
typedef OffHook initial_state;
```

Finally, we can define an action to occur if there are no possible transitions. It could happen! For example, after you smash the phone, you cannot use it anymore, right?

```c++
template <class FSM, class Event>
void no_transition(Event const& e, FSM&, int state)
{
    cout << "No transition from state " << state_names[state] << " on event " << typeid(e).name() << endl;
}
```

Boost MSM divides the state machine into the front end (that’s what we just wrote) and the back end (the part that runs it). Using the back-end API, we can construct the state machine from the preceding state machine definition:

```c++
msm::back::state_machine<PhoneStateMachine> phone;
```

Now, assuming the existence of the info() function that just prints the state we’re in, we can try orchestrating the following scenario:

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

So this is how you define a more sophisticated, industry-strength state machine.

## Summary

First of all, it’s worth highlighting that Boost.MSM is one of two alternative state machine implemenetations in Boost, the other being Boost.Statechart. I’m pretty sure there are plenty of other state machine implementations out there.

Second, there’s a lot more to state machines than that. For example, many libraries support the idea of *hierarchical* state machines: for example, a state of Sick can *contain* many different substates such as Flu or Chickenpox. If you are in state Flu, you are also assumed to be in the state Sick.

Finally, it’s worth highlighting again how far modern state machines are from the State design pattern in its original form. The existence of duplicate APIs (e.g., `LightSwitch::on/off` vs. `State::on/off`) as well as the presence of self-deletion are definite code smells in my book. Don’t get me wrong—the approach works, but it’s unintuitive and cumbersome.
