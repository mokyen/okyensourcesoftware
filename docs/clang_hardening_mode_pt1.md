# Exploring Clang's Hardening Modes

*This blog post was generated with the assistance of Claude, Anthropic's AI assistant.*

I recently began a new project, and I decided to pioneer the use of clang. I have been intrigued by the safety features and tooling around this compiler. Integration with [clang-tidy](https://clang.llvm.org/extra/clang-tidy/) and [clangd](https://clangd.llvm.org/), the [clang static analysis tools](https://clang-analyzer.llvm.org/), [IWYU](https://include-what-you-use.org/), several safety flags, [ARM FuSa Run-Time-System](https://developer.arm.com/Tools%20and%20Software/Keil%20MDK/FuSa%20Run-Time%20System), and the host of [sanitizers](https://github.com/google/sanitizers) were all reasons to try something different than GCC. One other feature also really caught my eye earlier in 2024: the [hardening modes](https://libcxx.llvm.org/Hardening.html).

At C++Now 2024, I had the opportunity to attend the talk, ["Security in C++: Hardening Techniques from the Trenches" by Louis Dionne of Apple](https://youtu.be/t7EJTO0-reg?si=VpDiTv33ia26bswA). In this presentation, he covered a number of topics ranging from security concerns in modern C++ to practical implementation of safety features. He also discussed the hardening modes that had been integrated into the newer versions of clang. The hardening modes intrigued me in particular because of the ability to turn on checks at different levels and leave these checks on in production. The ability to tune things to meet your use case seemed extremely useful, but the ability to modify the feature on a translation unit basis was by far the most interesting aspect. I decided to finally dig in and try and get this to work. Rather than trying to integrate it on my whole project, I started where any good C++ engineer begins: godbolt.

## Understanding Hardening Modes

The core goal of hardening modes is to convert undefined behavior in C++ into defined, predictable behavior that fails safely. In standard C++, violating container bounds, using invalidated iterators, or dereferencing null pointers leads to undefined behavior - anything could happen. The hardening modes in libc++ aim to catch these issues at runtime and terminate the program in a controlled way rather than potentially continuing with corrupted state or memory.

## Why Use Hardening Modes?

Hardening modes serve three critical purposes that make them valuable for both development and production environments:

1. **Security**: Undefined behavior in C++ can be exploited by attackers. For example, an out-of-bounds vector access might allow an attacker to read or write memory they shouldn't have access to. Hardening modes ensure such undefined behavior results in immediate, controlled program termination rather than potentially exploitable behavior.

2. **Reliable Bug Detection**: One of the most insidious aspects of undefined behavior is its inconsistency. Consider an out-of-bounds vector access:
   ```cpp
   std::vector<int> vec(3);
   vec[10] = 42;  // Undefined behavior
   ```
   Without hardening modes, this code might:
   - Crash with a segmentation fault (if the memory access is invalid)
   - Silently corrupt other program data (if the access happens to hit valid memory)
   - Appear to work perfectly (if it hits unused but accessible memory)
   - Exhibit different behavior across different runs or platforms

   This inconsistency makes such bugs extremely difficult to detect during testing. The same code might work fine in development but fail in production, or vice versa. Hardening modes convert this undefined behavior into defined behavior that fails consistently every time, making these issues much easier to detect during testing.

3. **Enhanced Debugging**: When failures do occur, hardening modes (especially debug mode) provide clear, specific information about what went wrong. Instead of a generic segmentation fault or mysterious program behavior, you get detailed error messages identifying the exact nature of the violation.

To enable these checks in our testing, we used these compiler flags:
```cmake
-std=c++23 
-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG 
-stdlib=libc++ 
-D_LIBCPP_ABI_BOUNDED_ITERATORS 
-O0
```

## What's Working

In our testing, we found several categories of checks that successfully catch issues with helpful error messages in debug mode:

### 1. Valid Input Range Checks
These checks verify that iterator ranges are valid - the end iterator must be reachable from the begin iterator. We found two reliable ways to trigger these checks:

```cpp
// Using string_view constructor:
std::string str = "Hello World";
const char* begin = str.data() + 5;  // Points to " World"
const char* end = str.data() + 2;    // Points to "llo World"
std::string_view invalid_view(begin, end);
// Triggers: assertion (__end - __begin) >= 0 failed: std::string_view::string_view(iterator, sentinel) received invalid range
```

```cpp
// Using span constructor:
std::vector<int> vec = {1, 2, 3, 4, 5};
const int* begin = vec.data() + 3;  // Points to 4
const int* end = vec.data() + 1;    // Points to 2
std::span<const int> invalid_span(begin, end);
// Triggers: assertion last - first >= 0 failed: invalid range in span's constructor (iterator, sentinel)
```

This shows that view types (string_view, span) are particularly good at catching invalid ranges during construction.

### 2. Vector Bounds Checking
```cpp
std::vector<int> vec(3);
vec[100];  // Triggers: "vector[] index out of bounds"
```

### 3. Optional Value Access
```cpp
std::optional<int> opt;
*opt;  // Triggers: "optional operator* called on a disengaged value"
```

### 4. String View Bounds
```cpp
std::string_view sv = "test";
sv[100];  // Triggers: "string_view[] index out of bounds"
```

### 5. Allocator Compatibility
```cpp
template <typename T>
struct MyAlloc {
    // Required type definitions for C++ allocator
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::false_type;
    using is_always_equal = std::false_type;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    template <typename U>
    struct rebind {
        using other = MyAlloc<U>;
    };

    // Track allocator instances with IDs
    static int next_id;
    int id;
    
    MyAlloc() noexcept : id(next_id++) {}
    
    template <typename U>
    MyAlloc(const MyAlloc<U>& other) noexcept : id(other.id) {}
    
    // Memory management functions
    T* allocate(std::size_t n) {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }
    
    void deallocate(T* p, std::size_t) noexcept {
        ::operator delete(p);
    }
    
    // Make allocators explicitly incompatible
    bool operator==(const MyAlloc& other) const noexcept {
        return false;  // Always incompatible
    }
};

template <typename T>
int MyAlloc<T>::next_id = 0;

// Usage
std::set<int, std::less<>, MyAlloc<int>> s1(MyAlloc<int>());
std::set<int, std::less<>, MyAlloc<int>> s2(MyAlloc<int>());
s1.insert(1);
auto node = s1.extract(1);
s2.insert(std::move(node));  // Triggers: "node_type with incompatible allocator passed to set::insert()"
```

### 6. Overlapping Ranges
Our investigation revealed that overlapping range checks work differently than initially expected. Rather than triggering runtime assertions, the implementation safely handles overlapping ranges while providing compile-time checks where possible:

```cpp
// Example demonstrating safe handling of overlapping ranges:
auto buffer = std::make_unique<char[]>(100);
std::strcpy(buffer.get(), "Hello World");
size_t len = std::strlen(buffer.get());

char* dest = buffer.get() + 1;
const char* src = buffer.get();

std::char_traits<char>::copy(dest, src, len);
// Results in "HHello World" - demonstrating safe handling of overlap
```

This shows that the hardening system employs a multi-layered approach:
- Compile-time checks to prevent obviously unsafe operations
- Runtime implementation that safely handles overlapping memory operations
- Focus on preventing undefined behavior through implementation rather than runtime assertions

## Handled by Standard Exceptions

Some checks are handled through normal exception mechanisms rather than hardening:

### 1. Vector at() Access
```cpp
std::vector<int> vec(3);
vec.at(1000);  // Throws std::out_of_range, not hardening check
```

### 2. String Operations
```cpp
std::string str = "test";
str.erase(str.length() + 1, 1);  // Throws std::out_of_range
```

### 3. Null Function Calls
```cpp
std::function<void()> f = nullptr;
f();  // Throws std::bad_function_call instead of triggering hardening check
```

## Currently Not Catching

Several categories of checks that we expected aren't currently triggering:

### 1. Iterator Invalidation
```cpp
std::vector<int> vec{1, 2, 3};
auto it = vec.begin();
vec.clear();
*it = 100;  // No check triggered for invalid iterator use
```

### 2. Container Invariants
```cpp
struct BadCompare {
    bool operator()(int a, int b) const { 
        return true;  // Violates strict weak ordering
    }
};
std::set<int, BadCompare> bad_set;
bad_set.insert(1);
bad_set.insert(2);  // No check for invalid comparator
```

### 3. Smart Pointer Array Bounds
```cpp
auto arr = std::make_unique<int[]>(5);
arr[10] = 42;  // No check for array bounds
```

### 4. Mutex Operations
```cpp
std::mutex mtx;
mtx.lock();
mtx.unlock();
mtx.unlock();  // Double unlock not caught
```

### 5. Null Pointer Access
Despite multiple attempts with different standard library components, consistent null pointer checks were difficult to trigger. The standard library often handles null pointer cases through exceptions or implementation-defined behavior rather than hardening checks.

## Practical Implications

Based on our testing, the current implementation is most effective at catching:
- Basic container bounds violations
- Invalid optional access
- String view bounds violations
- Some algorithm argument validation
- Allocator compatibility issues
- Invalid iterator ranges in view types (string_view, span)

These checks alone can catch many common sources of undefined behavior, making the hardening modes valuable even in their current state. However, be aware that:

1. Some checks operate differently than expected (like overlapping ranges being handled safely rather than asserting)
2. Some potential checks are handled by standard exceptions rather than hardening
3. More complex issues like iterator invalidation or null pointer dereference aren't consistently caught
4. The effectiveness of checks can vary based on the specific standard library component being used
5. View types (string_view, span) seem to have the most robust checking mechanisms

## Next Steps

1. Reach out to the clang developers to understand:
   - Which checks are planned for future releases
   - If any additional configuration is needed for checks that aren't triggering
   - Timeline for implementing additional checks

2. Experiment with per-translation-unit hardening mode selection to understand:
   - How to effectively mix different hardening levels
   - Performance implications
   - Best practices for deployment

I'll be sharing more about these experiences in future posts as we integrate these features into our production environment. Stay tuned for more insights about using hardening modes in real-world applications.
