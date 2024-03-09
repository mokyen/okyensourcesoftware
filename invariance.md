# What is Invariance?

The term 'invariant' is one of the linchpins of Decision Making Isolation. It is used in the [Cpp Core Guidelines](https://isocpp.github.io/CppCoreGuidelines) as the reason to use a class versus struct. But what does it really mean?

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
> * A relationship between a data member and a resource (aka RAII).
> * A property of a data member that must always be true.

Let's break down each of these.

## Relationships Between Members

Here's a simple example.

Consider a simple electrical circuit that is characterized by Ohm's Law:

> volts = current * resistance

Ohm's law dictates that if you vary any one of the parameters, at least one other must also change. If the voltage increases, either the current or resistance must also increase. If the current decreases, either the resistance will increase or the voltage will decrease. They cannot be changed independently. If we were to code up an object that held these parameters, the parameters couldn't be public members. We'd need to enforce the invariance dictated by Ohm's Law.

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

The variable `m_isLightOn` on can change completely independently from `m_brightness`, but the reverse is not true. Therefore, these two pieces of data still have an invariance, or, said another way, the relationship between these two is an invariant.

## Relationship Between a Member and Resource

When a data member needs to have a relationship with a "resource," the [Cpp Core Guidelines' recommended programming idiom is RAII](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p8-dont-leak-any-resources).

cppreference.com [defines Resource Allocation Is Initialization](https://en.cppreference.com/w/cpp/language/raii) as:

> Resource Acquisition Is Initialization, or RAII, is a C++ programming technique that binds the life cycle of a resource that must be acquired before use (allocated heap memory, thread of execution, open socket, open file, locked mutex, disk space, database connection—anything that exists in limited supply) to the lifetime of an object.

RAII establishes a relationship between the data member and the resource. Placing a file handle as a public member would be dangerous, as changing it could create a memory leak. RAII creates an invariance between the member and the resource.

On a side note, I really prefer the term CADRe, or Constructor Acquires-Destructor Releases, as a better name for remembering RAII. It it just easier to remember the meaning. However, it's hard to change a term that's been around longer than the Super Nintendo.

## Property of a Data Member that Must Be True

The final type of invariance is similar to the first but isn't a relationship between data members. Instead, there's something that needs to be enforced about a member. Maybe our integer needs to always be even. Perhaps a string can only be a certain length. One of the most common places this is seen is in nontrivial getters or setters for an object that performs some enforcement.

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

Here, the invariance is a condition between a member and the constant *2*.

I admit that this third category could be considered broard and open-ended. Developers could choose to interpret this to justify just about any data needing to be in a class. Most of the time, the manipulation of a data member is going to be compared to some constant data or literal. Developers should really scrutinize if piece of data really needs that rule enforced before choosing to use a class.

## 'Invariant' Doesn't Mean Immutability

Keep in mind that invariance means a relationship, not that the data cannot change. The term in software for something that cannot change is immutability.

## So Why Do Invariants Matter?

[DMI](dmi.md) uses invariance as one of the key heuristics when determining if a class, a struct, or a free function should be implemented based on [C.2](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c2-use-class-if-the-class-has-an-invariant-use-struct-if-the-data-members-can-vary-independently) from the Guidelines.

## Complete Cpp Core Guideline on Invariance

With these simple examples in mind, here is what the Guidelines say about when to use a class in [C.2](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c2-use-class-if-the-class-has-an-invariant-use-struct-if-the-data-members-can-vary-independently):

> ### <a name="Rc-struct"></a>C.2: Use `class` if the class has an invariant; use > `struct` if the data members can vary independently
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

One of the concerns that can arise from choosing to put data into a struct instead of a class with getters and setters is that those methods provide an interface that insulates callers if the underlying data needs to change. I agree that this not only can but absolutely *will* happen at some point. However, I think a couple of things can serve as protections. First, [DMI](dmi.md) gives guidance that keeps the scope of data small, meaning that the scope of changes will also be small. Second, changing from setting a variable directly to calling a getter or setter is a pretty minor refactor. Third, the placement of data is often just a best guess at first anyway, so the location, type, etc. of data is likely to change over time anyway. The setter/getter interface is also likely change from the first implementation. Modern IDEs continue to make this easier and easier, and the introduction of AI will only continue this trend.

## Conclusion

Invariance is an important concept that can serve as a real guidance in the design and implementation of our code. It's integral to [DMI](dmi.md), so check out that blog if you're interested in learning more!
