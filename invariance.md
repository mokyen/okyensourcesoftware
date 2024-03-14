# What is Invariance?

The term 'invariant' is one of the linchpins of Decision Making Isolation. It is used in the [Cpp Core Guidelines](https://isocpp.github.io/CppCoreGuidelines) as the reason to use a class versus struct. But what does it really mean?

## Struct vs Class

Before we discuss invariance, let's take a look at what else the [Cpp Core Guidelines](https://isocpp.github.io/CppCoreGuidelines) say about the difference between a class and struct.

* [C.8](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c8-use-class-rather-than-struct-if-any-member-is-non-public): Use `class` rather than `struct` if any member is non-public

While `class` and `struct` are relatively equivalent in C++, the Guidelines set the convention that structs are intended to only contain public members. Because all public data members can be directly modified, I believe it can be inferred that structs also shouldn't have public member functions. In the [DMI blog](dmi.md), we'll go more into the preference for free functions. For now, we'll assume that structs only have public data members and no functions.

* [C.2](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c2-use-class-if-the-class-has-an-invariant-use-struct-if-the-data-members-can-vary-independently): Use `class` if the class has an invariant; use `struct` if the data members can vary independently.

The Guidelines also state the guidance that a class should be used when the data has an invariant. This guidance seems relatively clear-cut for deciding which type of object to use. However, the concept of *invariance* isn't trivial!

## Defining Invariance

In [C.2](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c2-use-class-if-the-class-has-an-invariant-use-struct-if-the-data-members-can-vary-independently), the Guidelines define invariance as:

> An invariant is a logical condition for the members of an object that a constructor must establish for the public member functions to assume. After the invariant is established (typically by a constructor) every member function can be called for the object. An invariant can be stated informally (e.g., in a comment) or more formally using Expects.
>
> If all data members can vary independently of each other, no invariant is possible.

I've read this explanation over and over, but it still never quite made sense in those terms. Let's propose an alternative definition of invariance:

> [!Important]  
> An invariant is a logical condition for the members of an object that can be expected to be true from the time the constructor exits to the time the destructor is called. These logical conditions fall into three categories.
>
> * A relationship between at least two data members such that all members cannot vary independently.
> * A guarantee that any function with access to a resource may be called at any time, i.e. implementing RAII.
> * A property of a data member that must always be true.

A key for me is remembering that an invariance is **a logical condition**.

Let's break down each of these types.

## Relationships Between Members

Here's a simple example.

Consider a simple electrical circuit that is characterized by Ohm's Law:

> volts = current * resistance

Ohm's law dictates that if you vary any one of the parameters, at least one other must also change. If the voltage increases, either the current or resistance must also increase. If the current decreases, either the resistance will increase or the voltage will decrease. They cannot be changed independently. If we were to code up an object that held these parameters, the parameters couldn't be public members. We'd need to enforce the invariance dictated by Ohm's Law, so then we'd choose to use a class instead of a struct.

```cpp
class SimpleCiruit {
public:
  SimpleCiruit(Voltage volts, Current amperage)
      : m_volts(volts), m_amperage(current) {
    m_resistance = volts / current;
  }
  SimpleCiruit(Current amperage, Resistance resistance)
      : m_amperage(current), m_resistance(resistance) {
    m_volts = current * resistance;
  }
  SimpleCiruit(Voltage volts, Resistance resistance)
      : m_volts(volts), m_resistance(resistance) {
    m_amperage = volts / resistance;
  }
  void setResistanceFixedVoltage(Resistance resistance) {
    m_resistance = resistance;
    m_amperage = m_volts / m_resistance;
  }
  void setResistanceFixedCurrent(Resistance resistance) {
    m_resistance = resistance;
    m_volts = m_amperage * m_resistance;
  }
  void setVoltsFixedCurrent(Voltage volts) {
    m_volts = volts;
    m_resistance = m_volts / m_amperage;
  }
  // ...
private:
  Voltage m_volts;
  Current m_amperage;
  Resistance m_resistance;
};
```

In our code, all the parameters are not necessarily linked in this way, and it's not necessarily the constructor that needs to enforce these relationships. For instance, consider this class:

```cpp
class Light {
public:
  void setBrightness(uint8_t brightness) {
    if (m_isLightOn) {
      m_brightness = brightness;
    }
  }
  void flipLightStatus() { m_isLightOn = !m_isLightOn; }

private:
  bool m_isLightOn;
  uint8_t m_brightness;
};
```

The variable `m_isLightOn` on can change completely independently from `m_brightness`, but the reverse is not true. Therefore, these two pieces of data still have an invariance: the brightness will only be updated when the light is on.

## Guaranteeing Manipulation of a Resource, or RAII

Resources such as files, sockets, or heap memory need to be managed so that the right steps are made to open/close, connect/close, or allocate/deallocate these assets. It can be dangerous to assume that a caller will properly implement the correct initialization/deinitialization, so the [Resource Allocation is Initialization, or RAII,](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p8-dont-leak-any-resources) technique was devised to encapsulate the assets. When RAII is utilized, the wrapping entity guarantees that any function that can access that object can be called from the end of its constructor until its destructor is called.

This guarantee is our second form of invariance.

cppreference.com [defines Resource Allocation Is Initialization](https://en.cppreference.com/w/cpp/language/raii) as:

> Resource Acquisition Is Initialization, or RAII, is a C++ programming technique that binds the life cycle of a resource that must be acquired before use (allocated heap memory, thread of execution, open socket, open file, locked mutex, disk space, database connection—anything that exists in limited supply) to the lifetime of an object.
>
> RAII guarantees that the resource is available to any function that may access the object (resource availability is a [class invariant](https://en.wikipedia.org/wiki/Class_invariant), eliminating redundant runtime tests). It also guarantees that all resources are released when the lifetime of their controlling object ends, in reverse order of acquisition.

Since the goal of RAII is to insulate the resource from direct manipulation, this necessitates that we use a class instead of a struct based on our previous discussion.

*On a side note, I really prefer the term CADRe, or Constructor Acquires-Destructor Releases, as a better name for remembering RAII. It is just easier to remember the meaning. However, it's hard to change a term that's been around longer than the Super Nintendo.*

## Property of a Data Member that Must Be True

The final type of invariance is similar to the first but isn't a relationship between data members. Instead, some condition of a data member exists that must be enforced unrelated to another data member. Maybe our integer needs to always be even. Perhaps a string can only be a certain length. One of the most common places this is seen is in nontrivial getters or setters for an object that performs some enforcement.

Consider this example:

```cpp
class EvenNumber {
public:
  EvenNumber(int i) { setNumber(i); }
  void setNumber(int i) {
    if ((i % 2) != 0) { 
      throw std::runtime_error("Invalid input");
    } else {
      m_i = i;
    }
  }

private:
  int m_i;
};
```

Here, the invariance is a condition between a member and the constant *2*.The most common case of this type of invariance is comparisons to a constant or literal, like the previous example. However, a similar case occurs when the invariance is a mathematical algorithm that must be applied to a member. For instance, a class could exists that stores an angle in radians but must convert the value to be in the range -2π to 2π. This is more involved than simply comparing to a constant, but it's a similar logical assertion that necessitates using a class.

## 'Invariant' Doesn't Mean Immutability

Remember that invariance means a logical condition that must always be true, not that the data cannot change. The term in software for something that cannot change is *immutability*.

## So Why Do Invariants Matter?

[DMI](dmi.md) uses invariance as one of the key heuristics when determining if a class, a struct, or a free function should be implemented based on [C.2](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c2-use-class-if-the-class-has-an-invariant-use-struct-if-the-data-members-can-vary-independently) from the Guidelines. In that blog, I discuss more about applying invariance in the Decision Making Isolation philosophy.

## Complete Cpp Core Guideline on Invariance

With these simple examples in mind, here is what the Guidelines say about when to use a class in [C.2](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c2-use-class-if-the-class-has-an-invariant-use-struct-if-the-data-members-can-vary-independently):

> ### <a name="Rc-struct"></a>C.2: Use `class` if the class has an invariant; use `struct` if the data members can vary independently
>
> #### Reason
>
> Readability.
> Ease of comprehension.
> The use of `class` alerts the programmer to the need for an invariant.
> This is a useful convention.
>
> #### Note
>
> An invariant is a logical condition for the members of an object that a constructor must establish for the public member functions to assume.
> After the invariant is established (typically by a constructor) every member function can be called for the object.
> An invariant can be stated informally (e.g., in a comment) or more formally using `Expects`.
>
> If all data members can vary independently of each other, no invariant is possible.
>
> #### Example
>
> ```cpp
>     struct Pair {  // the members can vary independently
>         string name;
>         int volume;
>     };
> ```
>
> but:
>
> ```cpp
>     class Date {
>     public:
>         // validate that {yy, mm, dd} is a valid date and initialize
>         Date(int yy, Month mm, char dd);
>         // ...
>     private:
>         int y;
>         Month m;
>         char d;    // day
>     };
> ```
>
> #### Note
>
> If a class has any `private` data, a user cannot completely initialize an object without the use of a constructor.
> Hence, the class definer will provide a constructor and must specify its meaning.
> This effectively means the definer needs to define an invariant.
>
> **See also**:
>
> * [define a class with private data as `class`](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-class)
> * [Prefer to place the interface first in a class](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rl-order)
> * [minimize exposure of members](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-private)
> * [Avoid `protected` data](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rh-protected)
>
> #### Enforcement
>
> Look for `struct`s with all data private and `class`es with public members.

## But what about when my invariants or data change?

One potential critique of these guidelines for choosing a struct versus a class is that public member functions provide an interface that isolates the private data members from the users of that object. If I have a class with a getter, I can change the type of that private data without modifying calls to the getter as long as the return type remains the same. If a struct is used and the data members are manipulated directly, any change to the types of those data members impacts the struct's consumer. Additionally, if a struct is used and we need to introduce an invariance for the private data (which would necessitate a switch from a struct to a class), then we'll also have to modify all the callers. Using a class is less likely to cause the callers of its member functions to change.

I agree that this is definitely a tradeoff. I think a couple of things can serve to minimize the impact of these scenarios. First, [DMI](dmi.md) gives guidance that keeps the scope of data small, meaning that the scope of updates when objects change will also be small. Second, changing from setting a variable directly to calling a getter or setter when a new invariance is introduced is a minor refactor. Third, the placement of data is often just a best guess at first, so the location, type, etc. of data is likely to change over time anyway. The setter/getter interface is also fluid early in development. Modern IDEs significantly simplify these types of refactors, and the introduction of AI will only continue this trend.

## Conclusion

Invariance is an important concept that can serve as guidance in designing and implementing code. It's integral to [DMI](dmi.md), so check out that blog to learn more about how invariance is applied in DMI to fulfill the Single Responsibility and Open-Closed Principles!
