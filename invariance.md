# What is Invariance?

The term 'invariant' is one of the linchpins of Decision Making Isolation. It is used in the CPP core guidelines as the reason to use a class versus struct. But what does it really mean?

## Defining Invariance

The Guidelines define invitations as:

> An invariant is a logical condition for the members of an object that a constructor must establish for the public member functions to assume. After the invariant is established (typically by a constructor) every member function can be called for the object. An invariant can be stated informally (e.g., in a comment) or more formally using Expects.
> 
> If all data members can vary independently of each other, no invariant is possible.

So what does this mean in practical terms? Here's a simple example.

Let's consider a simple electrical circuit which is characterized by Ohm's Law

> volts = current * resistance

This means that if you vary any one of the parameters, at least one other must also change. If the voltage increases, either the current or resistance must also increase. If the current decreases, either the resistance will increase or the voltage will decrease. They cannot be changed independently.

In our code, all of the parameters are not necessarily linked in this way. For instance, consider this class:

```cpp
class Light {
public:
void setBrightness(uint8_t brightness) {
    if (m_isLightOn) { m_brightness = brightness};
}
void flipLightStatus( ) {
    m_isLightOn =!m_isLightOn;
}
private:
bool m_isLightOn;
uint8_t m_brightness;
};
```
The variable M _ is light on can they changed completely independently from m_brightness, but the reverse is not true. Therefore, these two pieces of data still have an invariance, or, said another way, the relationship between these two is an invariant.

## 'Invariant' doesn't mean Immutability

Keep in mind that invariance means a relationship, not that the data cannot change. The term in software for something that cannot change is immutability.

## So Why do Invariants Matter?

DMI uses invariance as one of the key heuristics when determining if a class, a struct, or a free function should be implemented based on C.2 from the Guidelines.

## Straight from the Source

So with these simple examples in mind, here is what the Guidelines say about when to use a class:

> C.2: Use class if the class has an invariant; use struct if the data members can vary independently
> 
> 
> Reason
> 
> Readability. Ease of comprehension. The use of class alerts the programmer to the need for an invariant. This is a useful convention.
> 
> 
> Note
> 
> An invariant is a logical condition for the members of an object that a constructor must establish for the public member functions to assume. After the invariant is established (typically by a constructor) every member function can be called for the object. An invariant can be stated informally (e.g., in a comment) or more formally using Expects.
> 
> If all data members can vary independently of each other, no invariant is possible.
> 
> 
> Example
> 
> ```cpp
> struct Pair {  // the members can vary independently
>     string name;
>     int volume;
> };
> 
> ```
> but:
> 
> ```cpp
> class Date {
> public:
>     // validate that {yy, mm, dd} is a valid date and initialize
>     Date(int yy, Month mm, char dd);
>     // ...
> private:
>     int y;
>     Month m;
>     char d;    // day
> };
> 
> ```
> 
> Note
> 
> If a class has any private data, a user cannot completely initialize an object without the use of a constructor. Hence, the class definer will provide a constructor and must specify its meaning. This effectively means the definer need to define an invariant.
> 
> See also:
> 
> Define a class with private data as class
> Prefer to place the interface first in a class
> Minimize exposure of members
> Avoid protecteddata
> 
> Enforcement
> 
> Look for structs with all data private and classes with public members


## ...but all my data's relationships aren't that simple

Of course these simplified examples don't fully capture how data is arranged.

==Talk about an example of code where there is an invariance and a bunch of other data and how the data not part of the invariance could be part of a struct in the invariance could be a class in that struct.==

An example could be an extension of the light class. There could be a name field that has nothing to do with any other data. That could be put in a strut that contains a class with the settings data in, and that class would contain the invariance.

An issue is that this starts to break down a bit when you consider that you need a setter that is more complex for the name. What if you need to make sure the name follows certain rules? I'm not sure how that ties in here. There's no invariance that is created, but you need a custom setter.

## special types of invariance

As we dig further and further into this concept, we realized that there are lots of these relationships that don't look quite like these simplified examples. However, the same philosophy can be applied. Among the special cases are

### RAII

### CRTP

### State Design Pattern

### Nontrivial Getters and Setters

## But what about when my invariants or data change?

Talk about how putting data into classes doesn't necessarily help when you need to introduce an invariant later. You are guessing where that might occur if you are choosing to stick all your data in the classes just because they might change. By keeping data in small strucks that are related, it's fairly simple to make that change later. Changing from setting a variable directly to calling a setter function is it pretty minor refactor. 