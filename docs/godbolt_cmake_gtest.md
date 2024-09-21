# Using Gtest with CMake in Godbolt

## TL;DR

Learn how to set up Google Test (Gtest) with CMake in Compiler Explorer (Godbolt) for efficient C++ unit testing in a multi-file project environment. This approach streamlines the development process, especially for personal projects or when you need a quick, sharable testing setup.

## Introduction

Compiler Explorer (CE), also known as godbolt.org, has been a game-changer for many software engineers, myself included. Over the past four years, it has significantly impacted my development process. This tool's influence extends beyond individual developers; even tech giants like Google incorporate CE [into their official processes](https://www.youtube.com/watch?v=torqlZnu9Ag). The C++ community (and many others) owes a debt of gratitude to [Matt Godbolt](https://xania.org/) for creating this invaluable resource.

One of CE's most powerful applications is in learning how to perform Test Driven Development (TDD). Previously, setting up a TDD environment could take hours. With CE and some guidance from [Niall Cooling's blog](https://blog.feabhas.com/2020/08/tdd-with-compiler-explorer/), I was able to start unit testing in under two minutes. This accessibility extends to various devices, from desktops to tablets, and even smartphones (though mobile usage has its limitations).

While CE's Gtest integration works seamlessly for single-file projects, things get more complex when you need to separate source code and unit tests. This blog post will guide you through setting up Gtest with CMake in Compiler Explorer for multi-file projects, sharing insights from my experience and offering solutions to common challenges.

## Why This Matters

As your projects grow, separating source code and unit tests becomes crucial for maintainability. This division becomes required if you're creating a library that you want to be usable by others. However, this separation often introduces complexity, especially when working with online tools like Compiler Explorer. By mautilizing  the setup of Gtest with CMake in CE, you can:

1. Quickly prototype and test complex, multi-file C++ projects
2. Share your work easily with colleagues or the community
3. Maintain a clean, organized codebase even in an online environment
4. Leverage the power of TDD without the overhead of local environment setup

Let's dive into how we can achieve this and overcome some common hurdles along the way.

## My Use Case

### The Birth of gtest_ct

After attending an enlightening talk at C++Now 2024 by Braden Ganetsky on ["Unit testing an expression template library in C++20,"](https://schedule.cppnow.org/session/2024/unit-testing-an-expression-template-library-in-cpp20/) I was inspired to create a tiny compile-time wrapper library for Gtest. I aptly named it [gtest_ct](https://github.com/mokyen/gtest_ct). (Stay tuned for a dedicated blog post on this library once I finalize version 1.0!)

### Development in Compiler Explorer

Compiler Explorer (CE) proved invaluable during the initial development of gtest_ct:

1. **Rapid Prototyping**: I quickly wrote and tested interesting code snippets.
2. **Immediate Feedback**: The library's functionality was verified using Gtest itself.
3. **Iteration Speed**: The development cycle was significantly faster than setting up a local environment.

### The Single-File Limitation

As the library grew, I hit a roadblock: with tests and source in the same file, I couldn't use gtest_ct outside of its own development. This limitation sparked my search for a multi-file solution in CE.

### The Sudoku Solver Project

The need for a multi-file setup became even more apparent when I started a professional development project to design and implement a sudoku solver. This project demanded:

1. **Modular Design**: Breaking the code into different files, mirroring a real-world project structure.
2. **Separation of Concerns**: Keeping source code and unit tests in separate files.
3. **Scalability**: Allowing for easy addition of new components and tests.

### The CMake Solution

While CE has supported multiple files using CMake for years, incorporating Gtest isn't straightforward due to the interworkings of CD and compilation time limits. This challenge led me to develop the solution I'm sharing in this blog post.

In the following sections, we'll dive into how to set up Gtest with CMake in Compiler Explorer, overcoming these limitations and enabling more complex, real-world project structures.

## Setting Up Gtest with CMake in Compiler Explorer

### The Starter Example

To get you up and running quickly, I've prepared a [starter example project](https://godbolt.org/z/5GYb95sz9). This example demonstrates the key components needed to use Gtest with CMake in Compiler Explorer. A big 'thank you' to RF and French Joe for figuring this out initially!

### Key Components of the Setup

Let's break down the essential elements that make this setup work:

1. **Configuring CE to Include Gtest**
   - Select Gtest from the library drop-down menu in CE.
   - Be aware that multiple versions may be listed, which can create challenges (more on this in the "Gotchas" section).

2. **Including the Correct Libraries**

   ```cmake
   target_link_libraries(the_executable PRIVATE gtest gmock gtest_main)
   ```

   - Include `gtest`, `gmock`, and `gtest_main` to ensure full Gtest functionality.
   - `gtest_main` eliminates the need for a boilerplate main() function.

3. **Setting the CMake Version**

   ```cmake
   cmake_minimum_required(VERSION 3.5)
   ```

   - The CMake version is a crucial configuration variable.
   - Version 3.5 works well in most cases, but you may need to adjust based on your specific needs or CE's current setup.

4. **Project Structure**
   - `CMakeLists.txt`: Main CMake configuration file
   - `source.cpp`: A source file
   - `test_source.cpp`: Test file (which can be named anything)
   - Additional `.cpp` and `.h` files as needed

This structure allows for a clear separation of concerns while remaining manageable within CE's interface.

### Example CMakeLists.txt

Here's a basic `CMakeLists.txt` file that ties everything together:

```cmake
cmake_minimum_required(VERSION 3.5)

project(MyProject)

add_executable(the_executable
    source.cpp
    test_source.cpp)

target_link_libraries(the_executable PRIVATE gtest gmock gmock_main)
```

This configuration sets up a basic project with Gtest support, compiling both your main code and tests into a single executable.

By following this setup, you'll have a solid foundation for using Gtest with CMake in Compiler Explorer, enabling more complex, multi-file projects.

## The Biggest Gotcha to Avoid

While setting up Gtest with CMake in Compiler Explorer is generally straightforward, there's one major pitfall that can cause frustration: version incompatibilities.

### Understanding the Issue

Among Compiler Explorer's greatest strengths lies is its support for numerous compilers and libraries. However, this variety can lead to compatibility issues:

1. **Pre-built Libraries**: CE uses pre-built versions of libraries like Gtest, installed using Conan.
2. **Location of Libraries**: These pre-built Gtest libraries are typically located at `/app/googletest/lib/libgtest_maind.a`. This library must be linked to the binary via CmakeLists.txt.
3. **Version Mismatches**: Not every Gtest library revision is pre-built for every compiler version in CE.

### Symptoms of Version Incompatibility

If you encounter build failures that seem unrelated to your code, it might be due to this version mismatch. Common symptoms include:

- Unexpected linker errors
- Missing symbol errors
- Compilation failures with cryptic messages

### How to Avoid the Gotcha

To navigate around this issue, follow these steps:

1. **Try Different Compiler Versions**:
   - If your project fails to build, experiment with different versions of your chosen compiler.
   - Sometimes, switching to an older or newer compiler version can resolve the issue.

2. **Adjust CMake Version**:
   - The CMake version is another 'adjustment knob' in the configuration.
   - Try different CMake versions to find one compatible with your chosen Gtest and compiler versions.

3. **Check Gtest Version**:
   - In the CE interface, verify which version of Gtest is being used.
   - If possible, try different Gtest versions to find one that works with your setup.

4. **Check C++ Version**:
   - In the CE command line arguments textbox, enter the update the version (e.g. `-std=C++17`)

5. **Simplify and Iterate**:
   - Start with a minimal working example and gradually add complexity.
   - This approach helps isolate where compatibility issues might be occurring.

### Example Workflow

Here's a step-by-step process to troubleshoot version incompatibilities:

1. Start with the latest compiler version and Gtest version.
2. If that fails, try an older compiler version.
3. If issues persist, adjust the CMake version.
4. Still having problems? Try a different Gtest version or the C++ standard.
5. Repeat this process until you find a working combination.

Remember, the goal is to find a stable configuration that allows you to focus on your actual code and tests, rather than wrestling with setup issues.

By being aware of this potential gotcha and knowing how to work around it, you'll save time and frustration in your Compiler Explorer projects.

## Limitations

While using Gtest with CMake in Compiler Explorer is powerful, it's important to be aware of its limitations:

1. **Compilation Time Limits**:
   - CE restricts compilation and runtime to ensure fair usage for all users.
   - As projects grow, you may hit these limits more frequently.

2. **Inconsistent Compilation Success**:
   - Larger projects may compile intermittently, presumably due to server load variations.
   - This can make debugging challenging, as failures might not always be due to your code.

3. **Limited Testing Scope**:
   - You may need to comment out some unit test files in `CMakeLists.txt` to stay within time limits.
   - This can lead to unintentional breaking of commented-out tests.

### Workaround Strategy

To mitigate these limitations:

1. **Modular Testing**: Focus on testing specific modules or features at a time.
2. **Regular Full Tests**: Periodically uncomment all tests to ensure overall project health.
3. **Local Backups**: Maintain a local copy of your project for more comprehensive testing.

## Putting This to Use

With this setup, I'm now able to:

1. Finish separating the gtest_ct library into separate files.
2. Develop more complex projects like my sudoku solver with proper separation of concerns.
3. Quickly prototype and test ideas without the overhead of local environment setup.

## Conclusion

Using Gtest with CMake in Compiler Explorer opens up new possibilities for C++ development and testing:

1. **Rapid Prototyping**: Quickly set up and test complex, multi-file C++ projects.
2. **Easy Sharing**: Share your work effortlessly with colleagues or the community.
3. **Clean Codebase**: Maintain an organized project structure, even in an online environment.
4. **TDD Power**: Leverage Test-Driven Development without local setup overhead.

While there are limitations, the benefits of this approach far outweigh the drawbacks for many use cases, especially for personal projects or when you need a quick, sharable testing setup.

I encourage you to try this setup in your next project. Experiment, iterate, and see how it can enhance your development workflow. Remember, the key to mastering any tool is practice and persistence.

Happy coding, and may your tests always fail and then pass!

## Further Reading

- [Compiler Explorer Documentation](https://github.com/compiler-explorer/compiler-explorer/wiki)
- [Google Test Primer](https://github.com/google/googletest/blob/master/docs/primer.md)
- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [C++ Core Guidelines on Testing](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#t-testing)

Feel free to reach out if you have any questions or want to share your experiences with this setup. Let's continue to push the boundaries of what's possible in online C++ development!
