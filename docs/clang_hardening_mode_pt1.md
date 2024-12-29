# Exploring Clang's Hardening Modes

*This blog post was generated with the assistance of Claude, Anthropic's AI assistant.*

I recently began a new project, and I decided to pioneer the use of clang. I have been intrigued by the safety features and tooling around this compiler. Integration with clang tidy and clangd, the clang static analysis tools, IWYU, several safety flags, ARM FuSa Run-Time-System, and the host of sanitizers were all reasons to try something different than GCC. One other feature also really caught my eye earlier in 2024: the hardening modes.

At C++Now 2024, I had the opportunity to attend the talk, "Security in C++: Hardening Techniques from the Trenches" by Louis Dionne of Apple. In this presentation, he covered a number of topics ranging from security concerns in modern C++ to practical implementation of safety features. He also discussed the hardening modes that had been integrated into the newer versions of clang. (Apple is a big contributor to clang and LLVM.) The hardening modes intrigued me in particular because of the ability to turn on checks at different levels and leave these checks on in production. The ability to tune things to meet your use case seemed extremely useful, but the ability to modify the feature on a translation unit basis was by far the most interesting aspect. I decided to finally dig in and try and get this to work. Rather than trying to integrate it on my whole project, I started where any good C++ engineer begins: godbolt.

## Understanding Hardening Modes

The core goal of hardening modes is to convert undefined behavior in C++ into defined, predictable behavior that fails safely. In standard C++, violating container bounds, using invalidated iterators, or dereferencing null pointers leads to undefined behavior - anything could happen. The hardening modes in libc++ aim to catch these issues at runtime and terminate the program in a controlled way rather than potentially continuing with corrupted state or memory.

This controlled termination is particularly valuable for security-critical code. Rather than potentially allowing an attacker to exploit undefined behavior, the program fails fast and safely. The hardening modes provide this safety with different performance tradeoffs, allowing developers to choose the level of checking that makes sense for their use case.

These features were introduced in LLVM 18 (Clang 18), which is the first release to fully support the hardening modes as described in the original RFC. The implementation will continue to evolve, with LLVM 19 and 20 planned to contain additional breaking changes and improvements.

The hardening modes in libc++ provide several levels of safety checks:
- `none` - No runtime checks (default)
- `fast` - Essential security checks with minimal overhead
- `extensive` - More comprehensive checks while maintaining reasonable performance
- `debug` - All available checks enabled with detailed error messages

| Mode | Compiler Flag | CMake Variable | Macro Definition |
|------|---------------|----------------|------------------|
| None | `-flibc++-hardening=none` | `LIBCXX_HARDENING_MODE=none` | `-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE` |
| Fast | `-flibc++-hardening=fast` | `LIBCXX_HARDENING_MODE=fast` | `-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST` |
| Extensive | `-flibc++-hardening=extensive` | `LIBCXX_HARDENING_MODE=extensive` | `-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE` |
| Debug | `-flibc++-hardening=debug` | `LIBCXX_HARDENING_MODE=debug` | `-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG` |

## What Actually Works in Practice

After extensive testing, we've found several categories of checks that work reliably and others that aren't yet catching issues. Let's look at what we found:

### Working Checks

The hardening modes successfully catch several types of issues with helpful error messages in debug mode:

#### 1. Vector Bounds Checking
```cpp
std::vector<int> vec(3);
vec[100];  // Triggers: "vector[] index out of bounds"
```

#### 2. Optional Value Access
```cpp
std::optional<int> opt;
*opt;  // Triggers: "optional operator* called on a disengaged value"
```

#### 3. String View Bounds
```cpp
std::string_view sv = "test";
sv[100];  // Triggers: "string_view[] index out of bounds"
```

#### 4. Algorithm Argument Validation
```cpp
int result = std::clamp(5, 10, 1);  // Triggers: "Bad bounds passed to std::clamp"
```

### Handled by Standard Exceptions

These checks are caught through normal exception mechanisms rather than hardening:

#### 1. Vector at() Access
```cpp
std::vector<int> vec(3);
vec.at(1000);  // Throws std::out_of_range, not hardening check
```

#### 2. String Operations
```cpp
std::string str = "test";
str.erase(str.length() + 1, 1);  // Throws std::out_of_range
```

### Currently Not Catching

Several categories of checks that we expected aren't currently triggering:

#### 1. Iterator Invalidation
```cpp
std::vector<int> vec{1, 2, 3};
auto it = vec.begin();
vec.clear();
*it = 100;  // No check triggered for invalid iterator use
```

#### 2. Overlapping Ranges
```cpp
std::vector<int> vec{1, 2, 3, 4};
std::copy(vec.begin(), vec.end(), vec.begin() + 1);  // No check for overlapping ranges
```

#### 3. Container Invariants
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

#### 4. Smart Pointer Array Bounds
```cpp
auto arr = std::make_unique<int[]>(5);
arr[10] = 42;  // No check for array bounds
```

#### 5. Mutex Operations
```cpp
std::mutex mtx;
mtx.lock();
mtx.unlock();
mtx.unlock();  // Double unlock not caught
```

#### 6. Allocator Compatibility
```cpp
using RegularSet = std::set<int>;
using CustomSet = std::set<int, std::less<>, MyAlloc<int>>;
RegularSet s1{1, 2, 3};
CustomSet s2;
auto node = s1.extract(1);
// Even with runtime type manipulation, allocator incompatibility not caught
```

## Practical Implications

The current implementation is most effective at catching:
- Basic container bounds violations
- Invalid optional access
- String view bounds violations
- Some algorithm argument validation

These checks alone can catch many common sources of undefined behavior, making the hardening modes valuable even in their current state. However, be aware that more complex issues like iterator invalidation, overlapping ranges, or allocator compatibility aren't yet being caught.

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
