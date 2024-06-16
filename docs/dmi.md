# Decision-Making Isolation (DMI)

Decision-Making Isolation is a design philosophy that emphasizes separating the decision-making in code from the IO (gathering inputs to the decisions and making outputs based on those decisions) and the wiring code (which connects the decision-making and IO). DMI emphasizes [invariance](dmi.md) as a key concept in choosing how to structure code and prioritizes free functions over classes. The goal is to achieve a high level of purity that makes code easy to read, modify, extend, and test.

DMI was [presented at C++Now 2024](https://schedule.cppnow.org/session/2024/developing-better-code-by-isolating-decisions/)! The recording of that presentation is expected to be posted to YouTube in the summer of 2024.

This blog is a work in progress, so please check back soon! In the meantime, check out my [invariance blog](dmi.md) in which I explore this key concept.
