# Decision Making Isolation - Practical Guidance for SRP and OCP

Writing code is relatively easy. Writing good code is hard.

The SOLID principles were first published by Robert C. Martin (aka Uncle Bob). The LID are relatively easy to understand and implement. The challenging principles have always been SRP and OCP. A few years ago, I began working with some colleagues on creating more valuable unit tests. That's a story for another day, but our answer came to be that code needs to be designed better to be tested. That's led us to a design philosophy that provides practical guidance on how to design better code that fulfills both SRP and OCP. We call it Design Making Isolation, or DMI.

## The Challenge with SRP and OCP

The 

### SRP

Among the [original definition of SRP](http://butunclebob.com/ArticleS.UncleBob.PrinciplesOfOod) from Uncle Bob reads:

> "There should never be more than one reason for a class to change."

In _Clean Architecture_, Uncle Bob refines this as:

> "A module should be responsible to one, and only one, actor."

(Note that Martin states that the term 'module' here is referring to "a cohesive set of functions and data structures", not to [C++20 modules](https://en.cppreference.com/w/cpp/language/modules).)

Klaus Iglberger states SRP a bit differently as a 


## References
- Martin, R. (n.d.). Principles of OOD. butunclebob.com. Retrieved December 28, 2023, from http://butunclebob.com/ArticleS.UncleBob.PrinciplesOfOod
- Martin, R. C. (2018). Clean Architecture: A Craftsman’s Guide to Software Structure and Design.
- Iglberger, K. (2023). C++ Software Design: Design Principles and Patterns for High-Quality Software. O’Reilly Media.
