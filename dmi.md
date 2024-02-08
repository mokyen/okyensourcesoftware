# Decision Making Isolation - Practical Guidance for SRP and OCP

Writing code is relatively easy. Writing good code is hard.

The SOLID principles were first published by Robert C. Martin (aka Uncle Bob). For me, the 'LID' are relatively easy to understand and implement. I've found that the challenging principles have always been SRP and OCP. A few years ago, I began working with some colleagues on creating more valuable unit tests. That's a story for another day, but our answer came to be that code needs to be designed better to be tested. That's led us to a design philosophy that provides practical guidance on how to design better software entities that fulfills the SOLID and particularly SRP and OCP that also makes code more readable, maintainable, and extensible. We call it Design Making Isolation, or DMI.

## What is Quality Software?

The baseline goal of software is code that *works*, but that's what I would call "functional code."

The real measure isn't if software works when a feature is first completed or even sometimes at the first release. Instead, the true test comes as that software has to change.

Klaus Iglberger gave this definition:

"High-quality software is easy to change, easy to extend, and easy to test." (Iglberger, 2022)

I would also add 'readability' to this list. It could be argued that readability implied in the ability to change and extend, but I contend that other developers will also need to be able to read your code to understand it even if they are not modifying it in any way, such as when they use your API.

Therefore, I propose that quality software is easy to *read, modify, extend, and test.*

## Software Entities

Any field that is as big and fast-developing as software is going to be plagued with overloaded terminology. When I say 'software entity' or 'entity', I am referring to this [definition from CppReference](https://en.cppreference.com/w/cpp/language/basic_concepts):

> The entities of a C++ program are values, objects, references, structured bindings(since C++17), functions, enumerators, types, class members, templates, template specializations, parameter packs(since C++11), and namespaces. Preprocessor macros are not C++ entities.

I realize that 'entity layer' is a term in *Clean Architecture* and 'entity' is a type of object in *Domain Driven Design*. It's still the best term I've found to date.

In this blog, I am mostly talking about entities as classes, structs, and free functions.

## The Challenge with SRP and OCP

TBD.

### SRP

Among the [original definitions of SRP](http://butunclebob.com/ArticleS.UncleBob.PrinciplesOfOod) is one from Uncle Bob that reads:

> "There should never be more than one reason for a class to change."

In *Clean Architecture*, Uncle Bob refines this as:

> "A module should be responsible to one, and only one, actor."

(Note that Martin states that the term 'module' here is referring to "a cohesive set of functions and data structures", not to [C++20 modules](https://en.cppreference.com/w/cpp/language/modules).)

Klaus Iglberger states SRP a bit differently as...

### OCP

TBD

## Core Guidelines

If we're going to look for advice on C++, the ultimate source is the [Cpp Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).

So what do the Guidelines have to say about designing entities?

<details>
  <summary>F.8: Prefer pure functions</summary>
  
### <a name="Rf-pure"></a>F.8: Prefer pure functions

#### Reason

Pure functions are easier to reason about, sometimes easier to optimize (and even parallelize), and sometimes can be memoized.

#### Example

```cpp
    template<class T>
    auto square(T t) { return t * t; }
```

#### Enforcement

Not possible.

</details>

> [!Important]  
> **_Key Takeaway_** Free functions should be the default option for functionality (not classes).

<details>
  <summary>C.2: Use `class` if the class has an invariant; use `struct` if the data members can vary independently</summary>
  
### <a name="Rc-struct"></a>C.2: Use `class` if the class has an invariant; use `struct` if the data members can vary independently

#### Reason

Readability.
Ease of comprehension.
The use of `class` alerts the programmer to the need for an invariant.
This is a useful convention.

#### Note

An invariant is a logical condition for the members of an object that a constructor must establish for the public member functions to assume.
After the invariant is established (typically by a constructor) every member function can be called for the object.
An invariant can be stated informally (e.g., in a comment) or more formally using `Expects`.

If all data members can vary independently of each other, no invariant is possible.

#### Example

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

#### Note

If a class has any `private` data, a user cannot completely initialize an object without the use of a constructor.
Hence, the class definer will provide a constructor and must specify its meaning.
This effectively means the definer need to define an invariant.

**See also**:

* [define a class with private data as `class`](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-class)
* [Prefer to place the interface first in a class](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rl-order)
* [minimize exposure of members](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-private)
* [Avoid `protected` data](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rh-protected)

#### Enforcement

Look for `struct`s with all data private and `class`es with public members.

</details>

> [!Important]  
> **_Key Takeaway_** Classes should be used when an invariance exists. If no invariances exist, use a struct. This concept is so important that there is an [article dedicated just to invariance](invariance.md).

<details>
  <summary>C.4: Make a function a member only if it needs direct access to the representation of a class</summary>
  
### <a name="Rc-member"></a>C.4: Make a function a member only if it needs direct access to the representation of a class

#### Reason

Less coupling than with member functions, fewer functions that can cause trouble by modifying object state, reduces the number of functions that needs to be modified after a change in representation.

#### Example

```cpp
    class Date {
        // ... relatively small interface ...
    };

    // helper functions:
    Date next_weekday(Date);
    bool operator==(Date, Date);
```

The "helper functions" have no need for direct access to the representation of a `Date`.

#### Note

This rule becomes even better if C++ gets ["uniform function call"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0251r0.pdf).

#### Exception

The language requires `virtual` functions to be members, and not all `virtual` functions directly access data.
In particular, members of an abstract class rarely do.

Note [multi-methods](https://web.archive.org/web/20200605021759/https://parasol.tamu.edu/~yuriys/papers/OMM10.pdf).

#### Exception

The language requires operators `=`, `()`, `[]`, and `->` to be members.

#### Exception

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

#### Exception

Similarly, a set of functions could be designed to be used in a chain:

```cpp
    x.scale(0.5).rotate(45).set_color(Color::red);
```

Typically, some but not all of such functions directly access `private` data.

#### Enforcement

* Look for non-`virtual` member functions that do not touch data members directly.
The snag is that many member functions that do not need to touch data members directly do.
* Ignore `virtual` functions.
* Ignore functions that are part of an overload set out of which at least one function accesses `private` members.
* Ignore functions returning `this`.

</details>

> [!Important]  
> **_Key Takeaway_** Only make a function a member if it needs access to private data or is part of an interface.

<details>
  <summary>C.8: Use `class` rather than `struct` if any member is non-public</summary>
  
### <a name="Rc-class"></a>C.8: Use `class` rather than `struct` if any member is non-public

#### Reason

Readability.
To make it clear that something is being hidden/abstracted.
This is a useful convention.

#### Example, bad

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

#### Note

Prefer to place the interface first in a class, [see NL.16](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rl-order).

#### Enforcement

Flag classes declared with `struct` if there is a `private` or `protected` member.

</details>

> [!Important]  
> **_Key Takeaway_** Use a struct when all data is public, and use a class when there is any private data.

## Decision Making Isolation

The vision of DMI is this:

> [!Important]
> The most important part of our code are the places where we decide what should happen. If these decisions are isolated, that critical code is easy to read, modify, extend, and test. If those key decisions are implemented correctly, then we can feel confident that our code trying to do what we expect it to do.

To achieve this, we have four key goals in DMI:

* Separate decision-making into pure free functions or small classes without external dependencies. The remaining code is IO or wiring.
* Default to using publicly available data, such as structs or variables, and free functions instead of classes.
* Choose free functions unless the data has invariance or is part of an architectural boundary.
* Unit test all decision-making code.

Let's break down these key ideas.

### The three types of code

In DMI, code is organized into three types of code: decision making, IO, and wiring.

Let's look at a simple class example.

```cpp
    class RequestHandler
    {
    private:
        Outputter* m_outputter;
    public:
        RequestHandler (Outputter* outputter) : m_outputter(outputter)
        {}

        void processRequest(EState state){
            switch (state){
                case EState::State1:
                    m_outputter->send((int)EData::Data1);
                    break;
                case EState::State2:
                    m_outputter->send(2);
                    break;
            }
        }
    };
```

This class is fairly easy to read, but of course it's only a few lines long. As it grows, it's easy to imagine that the `switch` statement could grow increasingly complex and harder to follow.

This is also pretty simple to modify or extend at this scale. However, what if there are more considerations in determining what to send than just the `state`? I assume that most programmers have seen the twisted webs than can arise from switch statements. Being *closed to moficiation* seems a bit of stretch.

Testing this class is relatively difficult for such a simple class. I would have to create a spy, fake, or mock version of the `Outputter` object that I can inject into this class. Therefore, `RequestHandler` tests would be broken if the interface of `Outputter` changed.

In the past, I would have this class fulfills SRP. It has a Single Responsibility - to process a request. However, the coupling between the `Outputter` and the logic means that one *actor* could cause this to change.

The crux of the issue here is that all three types of code are combined into the processRequest function. DMI to the rescue!

#### Decision Making

Well, we're finally going to talk about the key term in this whole methodology. So what is decision making?

Decision-making code is where the logic occurs within the code. In the `processRequest` function, the logic is just the `switch` that determines what will occur. This is where we determine **what** we will do.

#### IO

Here, IO (inputs and outputs) does not refer to hardware or other layers of code. In DMI, IO is any injected dependency that would need to be .

Features:

* Not owned by the parent (composition)
* Cannot be created without external dependencies

The calls to `m_outputter` IO.

#### Wiring

Wiring code is all the rest. It's the code that stitches together the decision-making and the IO. This code tends to be relatively, well, *boring*, and it's easy to read.

### Example Rewritten using DMI

So what would the `RequestHandler` look like if we refactored it with DMI?

```cpp
class RequestHandler {
private:
    Outputter* m_outputter;
public:
    RequestHandler (Outputter* outputter) : m_outputter(outputter) {}
    //Wiring
    void processRequest(EState state) {
        auto data = decide(state); //Decision-making
        m_outputter->send(data); // I/O
    }};

namespace RequestHandlerHelpers {
int decide(EState state) {
    int ret;
    switch (state) {
        case EState::State1:
            ret = (int)EData::Data1;
            break;
        case EState::State2:
            ret = 2;
            break;
    }
        return ret;
}
}
```

The `switch` from before has moved to a pure free function. It has no external dependencie and is easy to read. The function has a single responsibility, so it is easy to maintain and extend. As a pure function, it is simple to test. That's our big four!

The `m_outputter` dependency is only used in the wiring code. It would be simple to code review the wiring and feel comfortable it's doing what you expect.

## Unit Testing in DMI

As previously mentioned, the original goal was to determine how to get more value from unit tests. The 'value' of testing is determined by the ratio of benfit versus cost:

> | Benefit | Cost |
> | ----------- | ----------- |
> | <ul><li>Confidence in the functionality of code</li><li>Ability to prevent regressions | Time and effort in creating and maintaining unit tests</li></ul> |

{{{CONSIDER IF THIS SHOULD BE THE "DELIVER FUNCTIONAL CODE MORE QUICKLY" DEFINITION INSTEAD}}}

### Testing the types of code

 The decision making is the most important part to test, and using DMI makes this code easy to test. Therefore, the decision making is highly valuable.

Since we've already tested the decisions, we could write tests for the remaining IO and wiring code. Testing this typically requires the use of test doubles, especially [spies, fakes, or mocks](https://martinfowler.com/bliki/TestDouble.html). All of these doubles attempt to simulate the injected object based on some assumptions which make them extremely coupled to that injected object. This leads to brittle tests. Using a mocking framework or creating a fake/stub isn't particularly difficult. However, I find that heavy testing of this IO leads to complicated tests that will break whenever the external dependencies change. The assumptions about these dependencies, especially if those dependencies are components outside of the developer's control like external libraries or embedded hardware, are also fragile. After you've made all of these assumptions, have you really increased your confidence that the code is functional? The benefit achieved is highly outweighed by the effort to create and maintain these tests.

So does DMI advocate that the only automated testing should be for the decision making? What about all the rest of the code? No, we have a solution for that too. We'll get there.

### Unit testing example

So how would we test the original `RequestHandler` implementation? Here, we use gtest.

```cpp
class RequestHandler
{
private:
    Outputter* m_outputter;
public:
    RequestHandler (Outputter* outputter) : m_outputter(outputter) {}
 
    void processRequest(EState state){
        switch (state){
            case EState::State1:
                m_Outputter->send((int)EData::Data1);
                break;
            case EState::State2:
                m_Outputter->send(2)
                break;
        }
    }
};

//Test double
class FakeOutputter : public Outputter {
public:
    void send(int a){
        m_lastDataSent = a;
    }
private:
    int m_lastDataSent; 
    EData m_dataTypeSent;
};

TEST(RequestHandlerTests, GivenState1_Return1) {
    //Arrange
    FakeOutputter outputter;
    RequestHandler uut(&outputter);
    //Act
    uut.processRequest(EState::State1);
    //Assert
    EXPECT_EQ(outputter.m_lastDataSent, 1);
} 
```

Just as we discussed before, at this level of complexity the the code doesn't look to bad. However, `FakeOutputter` will only continue to get more and more complex. This test will break if the signature of `Outputter` ever changes even if the RequestHandler stays the same. The test evaluates if the code does the right thing and the way it does that thing.

On a side note, I have increasingly come to prefer fakes over mocks. Mocking frameworks are incredibly powerful, but they have their own complexities and caveats that are sometimes ignored. This will probably turn into its own blog someday.

Now let's consider the unit test for the `RequestHandler` using DMI.

```cpp
class RequestHandler {
private:
    Outputter* m_outputter;
public:
    RequestHandler (Outputter* outputter) : m_outputter(outputter) {}
    //Wiring
    void processRequest(EState state) {
        auto data = RequestHandlerHelpers::decide(state); //Decision-making
        m_Outputter->send(data); // I/O
    }};

namespace RequestHandlerHelpers {
int decide(EState state) {
    int ret;
    switch (state) {
        case EState::State1:
            ret = 1;
            break;
        case EState::State2:
            ret = 2;
            break;
    }
        return ret;
}
}

TEST(DecideTests, GivenState1_Return1) {
    //Arrange, Act, and Assert
    EXPECT_EQ(
        RequestHandlerHelpers::decide(EState::State1), 1);
}
```

The unit test above gives confidence that the software will perform the correct action while remaining simple and readable. The test evaluates one thing and only one thing.

### DMI Rules

From these key takeaways, we have established the following guidelines as DMI when creating a software entity:

* All decision making should be in free functions or in a 'small class' (i.e. one that encapsulates the invariance(s)).
* Default to using publicly available data, such as structs or variables, and free functions instead of classes.
* Consider the data that is core to the functionality of the entity. Identify any invariance(s) that exist.
* Create a class to encapsulate any invariance.
* All decision making should be unit tested
* Unit tests should be preferred at the highest level possible that doesn't require IO. Other objects can be included in these tests so long as they do not have external dependencies that require mocking. However, it is at the developer's discretion to add in additional lower level unit tests as needed for clarity or to evaluate edge cases.
* Decision making should not have external dependencies. There shouldn't be any calls to outside objects to get or set data. Functions (free or member) should be as pure as possible.
* Wiring code will typically not be unit test.
* Integration tests should be written at the highest level reasonable to test the 'happy path(s)'. Any evaluation of error and edge cases should be handled through the unit tests and code reviews.
* Test doubles should be avoided whenever possible. If they are required, the priority order of desirability is inversely correlated with the general level of complexity. From most desirable to least should be: stub, dummy, spy, fake, mock.

## Is this just clean architecture or hexagonal architecture?

For anyone familiar with [Uncle Bob's clean architecture](https://blog.cleancoder.com/uncle-bob/2012/08/13/the-clean-architecture.html) or [Alistair Cockburn's hexagonal architecture](https://alistair.cockburn.us/hexagonal-architecture), it could seem that DMI is the same thing. However, both of those are architectural philosophies. DMI is about designing software entities, specifically classes, functions, and structs. The philosophies certainly have significant parallels, and DMI is absolutely compatible with these architectural paradigms. In some ways, DMI is the low-level design version of these philosophies.

Clean architecture defines a set of layers within the code where inner layers are unaware of the outer layers so that all source code dependencies point towards inward. The most inner layers are the 'entity layer' and 'use case layer', Martin also refers to as the 'enterprise business rules' and 'application business rules', respectively.

So isn't all of the decision-making in DMI in these inner layers? No, because decisions are made at every level of code.

Consider a hardware driver, which would be in one of the outer layers. There are decisions that are made based on the data sheet for that part, and there is value in testing those decisions.

**TODO: NEED MORE DETAILD EXAMPLE**

## References

* Martin, R. (n.d.). Principles of OOD. butunclebob.com. Retrieved December 28, 2023, from <http://butunclebob.com/ArticleS.UncleBob.PrinciplesOfOod>
* Clean Coder Blog. (2012, August 13). Retrieved January 2, 2024, from <https://blog.cleancoder.com/uncle-bob/2012/08/13/the-clean-architecture.html>
* Cockburn, A. (2005, January 4). Hexagonal architecture. Alistair Cockburn. Retrieved January 2, 2024, from <https://alistair.cockburn.us/hexagonal-architecture/>
* Martin, R. C. (2018). Clean Architecture: A Craftsman’s Guide to Software Structure and Design.
* Iglberger, K. (2023). C++ Software Design: Design Principles and Patterns for High-Quality Software. O’Reilly Media.
* C++ Core Guidelines. (2023, October 12). Retrieved January 2, 2024, from <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>
* Iglberger, Klaus. (2022). Breaking Dependencies: The Path to High-Quality Software [PowerPoint slides]. <https://meetingcpp.com/mcpp/slides/2022/Breaking%20Dependencies8158.pdf>.

OCP Article: <https://drive.google.com/file/d/0BwhCYaYDn8EgN2M5MTkwM2EtNWFkZC00ZTI3LWFjZTUtNTFhZGZiYmUzODc1/view?resourcekey=0-FsS837CGML599A_o5D-nAw>
