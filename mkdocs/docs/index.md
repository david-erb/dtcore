# dtcore Library

## About this project

dtcore is a modular C library that provides core building blocks for my embedded systems work.  
_Current release: v1.0.0_

It brings together patterns that tend to recur in real systems: bounded buffers, explicit error handling, logging, and simple interface dispatch. The emphasis is on code that remains understandable over time. Control flow is visible, interfaces are explicit, and each module has a narrow, well-defined role.

Although the library is platform-agnostic, it is written with real targets in mind. The same core code is used and tested on Linux, Zephyr, ESP-IDF, and Raspberry Pi Pico.

Most of the code here is the result of iteration rather than upfront design. Interfaces have been adjusted as constraints became clearer, and modules have been reshaped when they proved difficult to test, reuse, or reason about.

This documentation provides design context and background that is not always obvious from the source code alone. It assumes familiarity with C and with embedded or systems-level development.

dtcore is maintained alongside related libraries that share the same design approach and development practices.

---

**Reference documentation begins in the navigation pane on the left.**

---