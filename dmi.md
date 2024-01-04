# Decision Making Isolation - Practical Guidance for SRP and OCP

Writing code is relatively easy. Writing good code is hard.

The SOLID principles were first published by Robert C. Martin (aka Uncle Bob). The LID are relatively easy to understand and implement. The challenging principles have always been SRP and OCP. A few years ago, I began working with some colleagues on creating more valuable unit tests. That's a story for another day, but our answer came to be that code needs to be designed better to be tested. That's led us to a design philosophy that provides practical guidance on how to design better code that fulfills both SRP and OCP. We call it Design Making Isolation, or DMI.

## Entities

https://timsong-cpp.github.io/cppwp/basic#pre-3
https://en.cppreference.com/w/cpp/language/basic_concepts

## The Challenge with SRP and OCP

The 

### SRP

Among the [original definitions of SRP](http://butunclebob.com/ArticleS.UncleBob.PrinciplesOfOod) is one from Uncle Bob that reads:

> "There should never be more than one reason for a class to change."

In _Clean Architecture_, Uncle Bob refines this as:

> "A module should be responsible to one, and only one, actor."

(Note that Martin states that the term 'module' here is referring to "a cohesive set of functions and data structures", not to [C++20 modules](https://en.cppreference.com/w/cpp/language/modules).)

Klaus Iglberger states SRP a bit differently as a 

## Core Guidelines

So what do the Cpp Core Guidelines have to say about designing classes?

<details>
  <summary>F.8: Prefer pure functions</summary>
  
### <a name="Rf-pure"></a>F.8: Prefer pure functions

##### Reason

Pure functions are easier to reason about, sometimes easier to optimize (and even parallelize), and sometimes can be memoized.

##### Example

```cpp
    template<class T>
    auto square(T t) { return t * t; }
```

##### Enforcement

Not possible.

</details>

> [!Important]  
> **_Key Takeaway_** Free functions should be the default option for functionality (not classes).

<details>
  <summary>C.2: Use `class` if the class has an invariant; use `struct` if the data members can vary independently</summary>
  
### <a name="Rc-struct"></a>C.2: Use `class` if the class has an invariant; use `struct` if the data members can vary independently

##### Reason

Readability.
Ease of comprehension.
The use of `class` alerts the programmer to the need for an invariant.
This is a useful convention.

##### Note

An invariant is a logical condition for the members of an object that a constructor must establish for the public member functions to assume.
After the invariant is established (typically by a constructor) every member function can be called for the object.
An invariant can be stated informally (e.g., in a comment) or more formally using `Expects`.

If all data members can vary independently of each other, no invariant is possible.

##### Example

```cpp
    struct Pair {  // the members can vary independently
        string name;
        int volume;
    };
```

but:
```cpp
    class Date {
    public:
        // validate that {yy, mm, dd} is a valid date and initialize
        Date(int yy, Month mm, char dd);
        // ...
    private:
        int y;
        Month m;
        char d;    // day
    };
```

##### Note

If a class has any `private` data, a user cannot completely initialize an object without the use of a constructor.
Hence, the class definer will provide a constructor and must specify its meaning.
This effectively means the definer need to define an invariant.

**See also**:

* [define a class with private data as `class`](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-class)
* [Prefer to place the interface first in a class](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rl-order)
* [minimize exposure of members](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-private)
* [Avoid `protected` data](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rh-protected)

##### Enforcement

Look for `struct`s with all data private and `class`es with public members.

</details>

> [!Important]  
> **_Key Takeaway_** Classes should be used when an invariance exists. If no invariances exist, use a struct. This concept is so important that there is an (article dedicated just to invariance)[https://github.com/mokyen/okyensourcesoftware/blob/main/invariance.md].

<details>
  <summary>C.4: Make a function a member only if it needs direct access to the representation of a class</summary>
  
### <a name="Rc-member"></a>C.4: Make a function a member only if it needs direct access to the representation of a class

##### Reason

Less coupling than with member functions, fewer functions that can cause trouble by modifying object state, reduces the number of functions that needs to be modified after a change in representation.

##### Example

```cpp
    class Date {
        // ... relatively small interface ...
    };

    // helper functions:
    Date next_weekday(Date);
    bool operator==(Date, Date);
```

The "helper functions" have no need for direct access to the representation of a `Date`.

##### Note

This rule becomes even better if C++ gets ["uniform function call"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0251r0.pdf).

##### Exception

The language requires `virtual` functions to be members, and not all `virtual` functions directly access data.
In particular, members of an abstract class rarely do.

Note [multi-methods](https://web.archive.org/web/20200605021759/https://parasol.tamu.edu/~yuriys/papers/OMM10.pdf).

##### Exception

The language requires operators `=`, `()`, `[]`, and `->` to be members.

##### Exception

An overload set could have some members that do not directly access `private` data:

```cpp
    class Foobar {
    public:
        void foo(long x) { /* manipulate private data */ }
        void foo(double x) { foo(std::lround(x)); }
        // ...
    private:
        // ...
    };
```

##### Exception

Similarly, a set of functions could be designed to be used in a chain:

```cpp
    x.scale(0.5).rotate(45).set_color(Color::red);
```

Typically, some but not all of such functions directly access `private` data.

##### Enforcement

* Look for non-`virtual` member functions that do not touch data members directly.
The snag is that many member functions that do not need to touch data members directly do.
* Ignore `virtual` functions.
* Ignore functions that are part of an overload set out of which at least one function accesses `private` members.
* Ignore functions returning `this`.
</details>

> *Key Takeaway* Enter text here


<details>
  <summary>C.5: Place helper functions in the same namespace as the class they support</summary>
  
### <a name="Rc-helper"></a>C.5: Place helper functions in the same namespace as the class they support

##### Reason

A helper function is a function (usually supplied by the writer of a class) that does not need direct access to the representation of the class, yet is seen as part of the useful interface to the class.
Placing them in the same namespace as the class makes their relationship to the class obvious and allows them to be found by argument dependent lookup.

##### Example

```cpp
    namespace Chrono { // here we keep time-related services

        class Time { /* ... */ };
        class Date { /* ... */ };

        // helper functions:
        bool operator==(Date, Date);
        Date next_weekday(Date);
        // ...
    }
```

##### Note

This is especially important for [overloaded operators](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ro-namespace).

##### Enforcement

* Flag global functions taking argument types from a single namespace.
</details>

> *Key Takeaway* Enter text here


<details>
  <summary>C.8: Use `class` rather than `struct` if any member is non-public</summary>
  
### <a name="Rc-class"></a>C.8: Use `class` rather than `struct` if any member is non-public

##### Reason

Readability.
To make it clear that something is being hidden/abstracted.
This is a useful convention.

##### Example, bad

```cpp
    struct Date {
        int d, m;

        Date(int i, Month m);
        // ... lots of functions ...
    private:
        int y;  // year
    };
```

There is nothing wrong with this code as far as the C++ language rules are concerned,
but nearly everything is wrong from a design perspective.
The private data is hidden far from the public data.
The data is split in different parts of the class declaration.
Different parts of the data have different access.
All of this decreases readability and complicates maintenance.

##### Note

Prefer to place the interface first in a class, [see NL.16](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rl-order).

##### Enforcement

Flag classes declared with `struct` if there is a `private` or `protected` member.

</details>

> *Key Takeaway* Enter text here

## Decision Making Isolation

### The three types of code

In DMI, code is organized into three types of code: IO, decision making, and wiring.

#### IO

TBD

#### Decision Making

TBD

#### Wiring

TBD

### DMI Rules

From these key takeaways, we have established the following guidelines as DMI when creating a software entity:

- Start with the goal of using publicly available data, such as structs or variables, and free functions
- Consider the data that is core to the functionality of the entity. Identify the invariance that exist.
- Create a class to encapsulate any invariance.
- All decision making should be in a 'small class' (i.e. one that encapsulates the invariance(s)) or in free functions.
- All decision making should be unit tested
- Unit tests should be preferred at the highest level possible that doesn't require IO. Other objects can be included in these tests so long as they do not have external dependencies that require mocking. However, it is at the developer's discretion to add in additional lower level unit tests as needed for clarity or to evaluate edge cases.
- Decision making should not have external dependencies. There shouldn't be any calls to outside objects to get or set data. Functions (free or member) should be as pure as possible.
- Wiring code will typically not be unit test.
- Integration tests should be written at the highest level reasonable to test the 'happy path(s)'. Any evaluation of error and edge cases should be handled through the unit tests and code reviews.
- Test doubles should be avoided whenever possible. If they are required, the priority order of desirability is inversely correlated with the level of complexity. From most desirable to least should be: stub, dummy, spy, fake, mock.
- 

## Is this just clean architecture or hexagonal architecture?

For anyone familiar with [Uncle Bob's clean architecture](https://blog.cleancoder.com/uncle-bob/2012/08/13/the-clean-architecture.html) or [Alistair Cockburn's hexagonal architecture](https://alistair.cockburn.us/hexagonal-architecture), It could seem that DMI is the same thing. However, both of those are architectural philosophies. DMI is about designing software entities, specifically classes, functions, and structs. The philosophies certainly have significant parallels, and DMI absolutely interplays with these architectural paradigms.

## Quality Code

"High-quality software is easy to change, easy to extend, and easy to test." (Iglberger, 2022)

## References
- Martin, R. (n.d.). Principles of OOD. butunclebob.com. Retrieved December 28, 2023, from http://butunclebob.com/ArticleS.UncleBob.PrinciplesOfOod
- Clean Coder Blog. (2012, August 13). Retrieved January 2, 2024, from https://blog.cleancoder.com/uncle-bob/2012/08/13/the-clean-architecture.html
- Cockburn, A. (2005, January 4). Hexagonal architecture. Alistair Cockburn. Retrieved January 2, 2024, from https://alistair.cockburn.us/hexagonal-architecture/
- Martin, R. C. (2018). Clean Architecture: A Craftsman’s Guide to Software Structure and Design.
- Iglberger, K. (2023). C++ Software Design: Design Principles and Patterns for High-Quality Software. O’Reilly Media.
- C++ Core Guidelines. (2023, October 12). Retrieved January 2, 2024, from https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
- Iglberger, Klaus. (2022). Breaking Dependencies: The Path to High-Quality Software [PowerPoint slides]. https://meetingcpp.com/mcpp/slides/2022/Breaking%20Dependencies8158.pdf.

















