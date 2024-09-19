# Using Gtest with CMake in Godbolt

## Intro

Perhaps no tool has been more impactful in my development as a software engineer in the past four years than Compiler Explorer (I'll call it CE for short), i.e.  godbolt.org. My experience is certainly not an outlier. At CppCon 2023, [one talk from a Google employee](https://youtu.be/torqlZnu9Ag?si=7TiXGuIBPVqMw31G) showed their workflow in creating custom rules in clang-tidy, and she explained that they use Compiler Explorer as part of their official process. In fact, I would go so far as to say that the entire C++ community (and many other languages) owes a debt of gratitude to [Matt Godbolt](https://xania.org/).

One of my most impactful uses of CE has when learning how to do Test Driven Development using Google Test. In the past, I spent hours trying to set up an environment so I could explore unit testing. With CE and a little help from [Niall Cooling's blog](https://blog.feabhas.com/2020/08/tdd-with-compiler-explorer/), I was able to get rolling on this in under 2 minutes. Plus, I was able to do this on my desktop or my old Windows tablet or even from my phone (although it doesn't work _well_ on mobile, but that might be the topic of a future blog).

This solution works great for having a single source file with your source and unit tests in one. One can simply choose to include Gtest and add a library on the command line options to get compiling and running unit tests.

However, what happens when you want to start separating source code and unit tests? Then things get a bit more complex.

## My Use Case

After attending a great talk at C++Now 2024 from [Braden Ganetsky called "Unit testing an expression template library in C++20”](https://schedule.cppnow.org/session/2024/unit-testing-an-expression-template-library-in-cpp20/), which was my first exposure to compiler-time unit testing, I decided to try my hand at creating a tiny compile-time wrapper library for Gtest, creatively called [gtest_ct](https://github.com/mokyen/gtest_ct). (Once I finish putting together V1.0 of that library, I plan to post a separate blog.) I was able to quickly get up and going with writing a bunch of interesting code and test that wrapper with, well, Gtest, of course! I wrote enough of the library that I felt like it got to the point of being useful. However, with the tests and source in the same file, I couldn't use this for anything outside of developing the library itself.

I debated going through a whole process to set up a Linux machine or getting a RaspberryPi or (gasp) brushing off my memory of setting up Cygwin to compile on Windows. I just didn't have time, and I didn't immediately need to use the library for anything. So, I moved on to other things and left gtest_ct on a branch.

Then I started a professional development project to design and implement a sudoku solver. Only having one file very quickly broke down when focusing on software design. I wanted to break the code into different files in the same way I would on a normal project.

CE added support for multiple files using Cmake several years ago, and it's [pretty quick to up and going with it](https://archive.is/AamVe). But incorporating Gtest isn't quite straightforward. CE limits your compilation/run time since processing power is finite and we all have to share, so compiling the whole Gtest library doesn't work.

## Setting Up Gtest with CMake in Compiler Explorer

I'm going to skip ahead to the solution, and then we'll circle back to a bit of explanation. A big 'thank you' to RF and French Joe for coming up with this example!

Here is [the starter example project](https://godbolt.org/z/5GYb95sz9).

## Keys to the Example

Several elements make this example work, and it turns out that they are much simpler than what I started out trying!

### Configuring CE to include Gtest

Just as described in Niall's blog that I mentioned previously, Gtest must be selected from the library drop-down. Multiple versions will be listed, and this can create challenges (as I will discuss below).

### Including the correct libraries

```cmake
target_link_libraries(the_executable PRIVATE gtest gmock gtest_main)
```

If you want to use Gtest and Gmock, then you need to specify both libraries as target libraries. Gtest also has another target that eliminates the need for a boilerplate main() function by including `gtest_main`.

### Setting the Cmake version

```cmake
cmake_minimum_required(VERSION 3.5)
```

The version is another customization variable when configuring your project that can cause some difficulties.

## The Biggest Got-cha to Avoid

Among the biggest things that make CE so amazing is the incredible number of compilers and versions it supports. It also has an ever-increasing number of libraries incorporated that are super easy to use. What I figured out is that it installs uses pre-built version of these libraries using Conan. For Gtest, these libraries are located in the cmake project directory at `/app/googletest/lib/libgtest_maind.a`.

However, with so many different compiler and library options, CE doesn't necessarily have a pre-built version of every Gtest library revision for every revision of every compiler. If this test project fails to build for any reason, try a different version of your compiler!

The Cmake version is yet another 'adjustment knob' in the configuration, so one might also need to play around with this to get something compatible with the Gtest and compiler versions.

## Limitations

I mentioned earlier that CE limits the compilation and runtime to be polite to all the other users. As my sudoku solver grew (both in source and test code), I ended up getting to a point where CE wouldn't consistently compile and run my tests. This seemed to be a bit of a crap shoot if it would be successful, which I chalk up to servers being more or less busy at a given moment in time.

My solution to this was to comment out some unit test files in the CMakeLists.txt most of the time unless I was actively modifying the related code or the tests themselves. Of course, this meant that I inevitably broke some of those commented-out tests inadvertently and had lag time before discovering it, but that was just the cost of using CE in this atypical manner.

## Putting This to Use

My next step is to finish separating out the gtest_ct library into separate files, and my first pass at doing this is going to be using CE with Cmake and Gtest!

## Go Try It!

I found this setup to be very helpful when I want to do a personal project that is a bit more complex. Let me know if you have other solutions!
