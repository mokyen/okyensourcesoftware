# When *Not* to Use Free Functions

Anyone who has discussed code design with me over the last few years probably associates me with free functions. They are a core part of my concept of [*Decision-Making Isolation* (DMI)](dmi.md), a design philosophy where I advocate that free functions should be the default over classes.

However, when I attended C++Now in 2024, I learned something surprising: Bloomberg Engineering actually bans free functions. Oh, the horror!!! In truth, they don’t outright ban them—they just enforce a different approach by requiring functions to be scoped within a class. The reason for this policy boils down to one key feature of C++: the One Definition Rule (ODR).

---

## The One Definition Rule

So, what is the One Definition Rule? It’s a foundational principle in C++ that states a function (or variable) can only be defined once within a given scope or namespace. The C++ standard has formal language to define this, but the gist is simple: duplicate definitions are a big no-no.

---

## _...But I scope my free functions in a namespace, so I'm good, right?_

Well, it's not quite that simple.

You’d think that putting free functions in namespaces would sidestep ODR violations since the namespace gives them a unique name. Unfortunately, this isn't always the case.

The compiler, in its effort to match your function call to the correct definition, can sometimes go astray. If there are multiple functions with the same name—even in different scopes—there’s a risk the compiler could pick the wrong one. Here’s an example scenario where an ODR violation might occur or where the compiler mistakenly calls the wrong function.

```cpp
// file1.cpp
namespace companyA {
    void send() {
        std::cout << "Sending from company A\n";
    }
}

// file2.cpp
namespace companyB {
    void send() {
        std::cout << "Sending from company B\n";
    }
}

// file3.cpp
namespace companyC {
    void send() {
        std::cout << "Sending from company C\n";
    }
}

// main.cpp
#include "file1.cpp"
#include "file2.cpp"
#include "file3.cpp"

int main() {
    send(); // Ambiguity: Which send function will the linker choose?
    return 0;
}
```

Here are some further examples.

### Example 1: Multiple Using Declarations in Different Namespaces

```cpp
// file1.cpp
namespace companyA {
    void send() {
        std::cout << "Sending from company A\n";
    }
}

// file2.cpp
namespace companyB {
    void send() {
        std::cout << "Sending from company B\n";
    }
}

// file3.cpp
namespace companyC {
    void send() {
        std::cout << "Sending from company C\n";
    }
}

// main.cpp
#include "file1.cpp"
#include "file2.cpp"
#include "file3.cpp"
using namespace companyA; // Using declaration for companyA's send function
using namespace companyB; // Using declaration for companyB's send function

namespace myNamespace {
    int main() {
        send(); // Ambiguity: Which send function will the linker choose?
        return 0;
    }
}
```

### Example 2: Using Directives in Different Namespaces

```cpp
// file1.cpp
namespace companyA {
    void send() {
        std::cout << "Sending from company A\n";
    }
}

// file2.cpp
namespace companyB {
    void send() {
        std::cout << "Sending from company B\n";
    }
}

// file3.cpp
namespace companyC {
    void send() {
        std::cout << "Sending from company C\n";
    }
}

// main.cpp
#include "file1.cpp"
#include "file2.cpp"
#include "file3.cpp"
using namespace companyA; // Using directive for companyA's send function
using namespace companyB; // Using directive for companyB's send function

namespace myNamespace {
    int main() {
        send(); // Ambiguity: Which send function will the linker choose?
        return 0;
    }
}
```

### Example 3: Using Declarations with Nested Namespaces

```cpp
// file1.cpp
namespace companyA {
    void send() {
        std::cout << "Sending from company A\n";
    }
}

// file2.cpp
namespace companyB {
    void send() {
        std::cout << "Sending from company B\n";
    }
}

// file3.cpp
namespace companyC {
    void send() {
        std::cout << "Sending from company C\n";
    }
}

// main.cpp
#include "file1.cpp"
#include "file2.cpp"
#include "file3.cpp"
using companyA::send; // Using declaration for companyA's send function

namespace myNamespace {
    using companyB::send; // Using declaration for companyB's send function

    int main() {
        send(); // Ambiguity: Which send function will the linker choose?
        return 0;
    }
}
```

### Example 4: Using Directives with Nested Namespaces

```cpp
// file1.cpp
namespace companyA {
    void send() {
        std::cout << "Sending from company A\n";
    }
}

// file2.cpp
namespace companyB {
    void send() {
        std::cout << "Sending from company B\n";
    }
}

// file3.cpp
namespace companyC {
    void send() {
        std::cout << "Sending from company C\n";
    }
}

// main.cpp
#include "file1.cpp"
#include "file2.cpp"
#include "file3.cpp"
using namespace companyA; // Using directive for companyA's send function

namespace myNamespace {
    using namespace companyB; // Using directive for companyB's send function

    int main() {
        send(); // Ambiguity: Which send function will the linker choose?
        return 0;
    }
}
```

---

## Possible Solutions

What can we do to avoid these issues? Here are some approaches:

### **Option 1: Fully Qualify Function Names**

Fully scoping the names of your functions leaves no ambiguity for the compiler. However, this defeats the purpose of using namespaces to clean up code and improve readability.

### **Option 2: Use Member Functions**

You could avoid free functions altogether and use member functions inside classes. But as I’ve discussed in previous articles, this is often unnecessary unless the function logically belongs to the class.

### **Option 3: Use Descriptive Function Names**

A more descriptive naming convention can help avoid overlaps. For instance, instead of generic names like `send`, `write`, or `publish`, you might use more specific ones like `sendOverCAN` or `writeToDisk`. However, this isn’t always practical, especially when dealing with polymorphism or template-based code.

### **Option 4: Enforce Naming Conventions with Tools**

Tools can enforce naming conventions and flag overlaps. This approach is preferable to manual code inspections or rigid guidelines, but it still has limitations—overlapping names might not always lead to ODR violations.

### **Option 5: Use Static Member Functions**

This is Bloomberg’s solution. By using static functions inside classes, you ensure that the function name must be qualified by the class name. This drastically reduces the chance of ODR violations. The downside? It adds a layer of verbosity and may obscure the intention of certain utility functions that don’t naturally belong to a specific class.

### **Option 6: Use Free Functions and Accept the Risk**

Sometimes, you just decide to live with the risk. If your codebase is small and well-contained, the chances of an ODR violation are minimal.

### **Option 7: Compiler Flags?**

Are there compiler flags that can help catch or prevent these issues? This is something I need to research further.

---

## When Does This Really Matter?

For small or medium-sized codebases, this likely isn’t a pressing concern. ODR violations and accidental overlaps become more problematic as your codebase scales.

Consider a company like Bloomberg, with over 30,000 C++ projects dating back to at least the 1990s. At that scale, the chances of two developers independently writing a function with the same name are extremely high. Similarly, the likelihood of someone pulling in a library with overlapping names is significant.

For these reasons, it could make sense for large organizations to adopt strict policies like banning namespace-scoped free functions in favor of static member functions. But for the majority of us—who don’t work at Amazon, Facebook, or Bloomberg—this problem may rarely, if ever, come up.

Still, it’s useful to understand why ODR violations occur and to be mindful of naming, namespaces, and potential pitfalls during debugging. Remember, the larger the codebase, the greater the chance of coincidental overlaps causing issues.

---

What do you think? Are free functions still worth it for your projects, or have you encountered scenarios where they caused more trouble than they’re worth? Let me know in the comments!
