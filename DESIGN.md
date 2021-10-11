# Design

This browser should be usable both from C++ and, as opcodes, from Csound.

It thus makes sense to develop first for C++, in order to define the interface and implement the functionality, and then to expose the interface through very thin opcodes.

There should be both synchronous and asynchronous versions of most functions.
