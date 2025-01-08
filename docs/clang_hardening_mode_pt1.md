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

Our testing has confirmed that several categories of checks work reliably:

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
// Triggers: assertion last - first >= 0 failed: invalid range in span's constructor
```

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
    // Custom allocator implementation...
    bool operator==(const MyAlloc& other) const noexcept {
        return false;  // Always incompatible
    }
};

std::set<int, std::less<>, MyAlloc<int>> s1(MyAlloc<int>());
std::set<int, std::less<>, MyAlloc<int>> s2(MyAlloc<int>());
s1.insert(1);
auto node = s1.extract(1);
s2.insert(std::move(node));  // Triggers: "node_type with incompatible allocator"
```

### 6. Bad Comparator Detection
```cpp
std::vector<int> vec = {1, 2, 3};
// Provide an invalid comparator that violates strict weak ordering
std::sort(vec.begin(), vec.end(), 
    [](int a, int b) { return true; });
// Triggers: "Comparator does not induce a strict weak ordering"
```

## Understanding Implementation Limits

Our testing has also revealed areas where the implementation has specific limitations or behaviors worth understanding:

### 1. Iterator Invalidation
```cpp
std::vector<int> vec{1, 2, 3};
auto it = vec.begin();
vec.clear();  // Invalidates iterator
*it = 100;    // No check triggered for invalid iterator use
```
The documentation explicitly notes that vector and string iterator invalidation is not checked. This is a documented limitation of the current implementation.

### 2. Overlapping Ranges
```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::copy_backward(vec.begin(), vec.end(), 
                  vec.begin() + vec.size() - 1);
```
While our testing showed this leads to memory corruption, it's detected through the allocator's mechanisms rather than direct hardening checks. The hardening system doesn't currently implement direct overlap detection.

### 3. Mutex Operations
```cpp
std::mutex mtx;
mtx.lock();
mtx.unlock();
mtx.unlock();  // Should be undefined behavior
```
Our source code analysis revealed that mutex validation is implemented through the `_LIBCPP_ASSERT_VALID_EXTERNAL_API_CALL` check, which validates the return codes from pthread operations. The effectiveness of these checks depends on the underlying pthread implementation.

### 4. Hash Function Requirements
```cpp
struct BadHash {
    size_t operator()(int x) const { return 42; }
};

std::unordered_set<int, BadHash> s;
s.insert(1);  // No hardening check triggers
```
Our testing showed that semantic requirements for hash functions are not currently validated by the hardening system. This makes sense given the statistical nature of what makes a good hash function.

## Practical Implications

Based on our extensive testing, we can summarize what works best in the current implementation:

### What Works Well
- Basic container bounds violations
- Invalid optional access
- String view bounds violations
- Some algorithm argument validation (like bad comparators)
- Allocator compatibility issues
- Invalid iterator ranges in view types (string_view, span)

### Current Limitations
1. Some checks operate differently than expected (like overlapping ranges being handled through allocator checks)
2. Some potential checks are handled by standard exceptions rather than hardening
3. More complex issues like iterator invalidation aren't currently caught
4. The effectiveness of system-dependent checks (like mutex operations) can vary by platform
5. View types (string_view, span) have the most robust checking mechanisms

## Next Steps

1. For Projects Using Hardening:
   - Consider enabling different hardening modes for different translation units based on their requirements
   - Plan additional validation where needed for areas not currently covered by hardening checks
   - Monitor LLVM release notes for new hardening features

2. Reach out to the clang developers to understand:
   - Which checks are planned for future releases
   - If any additional configuration is needed for checks that aren't triggering
   - Timeline for implementing additional checks

3. Experiment with per-translation-unit hardening mode selection to understand:
   - How to effectively mix different hardening levels
   - Performance implications
   - Best practices for deployment

I'll be sharing more about these experiences in future posts, particularly diving deeper into the implementation strategies and design decisions behind the hardening system. Stay tuned for a technical deep-dive into how these checks actually work under the hood!
