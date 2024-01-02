# Decision Making Isolation - Practical Guidance for SRP and OCP

Writing code is relatively easy. Writing good code is hard.

The SOLID principles were first published by Robert C. Martin (aka Uncle Bob). The LID are relatively easy to understand and implement. The challenging principles have always been SRP and OCP. A few years ago, I began working with some colleagues on creating more valuable unit tests. That's a story for another day, but our answer came to be that code needs to be designed better to be tested. That's led us to a design philosophy that provides practical guidance on how to design better code that fulfills both SRP and OCP. We call it Design Making Isolation, or DMI.

## The Challenge with SRP and OCP

The 

### SRP

Among the [original definitions of SRP](http://butunclebob.com/ArticleS.UncleBob.PrinciplesOfOod) is one from Uncle Bob that reads:

> "There should never be more than one reason for a class to change."

In _Clean Architecture_, Uncle Bob refines this as:

> "A module should be responsible to one, and only one, actor."

(Note that Martin states that the term 'module' here is referring to "a cohesive set of functions and data structures", not to [C++20 modules](https://en.cppreference.com/w/cpp/language/modules).)

Klaus Iglberger states SRP a bit differently as a 

## Decision Making Isolation

### The three types of code

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

## is this just clean architecture or hexagonal architecture?

For anyone familiar with Uncle Bob's clean architecture or the hexagon architecture proposed by Xxx, It could seem that DMI is the same thing. However, both of those are architectural philosophies. DMI is about designing software entities, specifically classes, functions, and structs. The philosophies certainly have significant parallels, and DMI absolutely interplays with these architectural paradigms.


## References
- Martin, R. (n.d.). Principles of OOD. butunclebob.com. Retrieved December 28, 2023, from http://butunclebob.com/ArticleS.UncleBob.PrinciplesOfOod
- Martin, R. C. (2018). Clean Architecture: A Craftsman’s Guide to Software Structure and Design.
- Iglberger, K. (2023). C++ Software Design: Design Principles and Patterns for High-Quality Software. O’Reilly Media.


## Core Guidelines

So what do the Cpp Core Guidelines have to say about designing classes?

<details>
  <summary>Click me</summary>
  
  ### Heading
  1. Foo
  2. Bar
     * Baz
     * Qux

  ### Some Javascript
  ```js
  function logSomething(something) {
    console.log('Something', something);
  }
  ```
</details>

> *Key Takeaway* Enter text here