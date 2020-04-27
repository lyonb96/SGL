# Simple Game Language
SGL is a basic scripting language I'm working on developing. Since I'm unable to obtain a CS degree, the best way for me to learn more about complicated things like compilers and bytecode is for me to just do it myself. Fortunately, I find I learn best that way anyway.

SGL uses a stack-based virtual machine and a custom bytecode to perform operations. It is meant to be fairly simple to read and write, and if it were to be developed to any real level of use, my ideal target would be as a gameplay scripting language.

# What does the language look like?
Currently, not much. It's so early in development that the syntax is fluid, and mostly serves as a guideline for writing the compiler. I do have a solid idea of what I _want_ the language to look like when complete. For example, here is what I imagine Hello World to look like in SGL:
```
func: HelloWorld()
{
    print("Hello, world!");
}
```
A more interesting sample of SGL might look like this:
```
func: CalculateDamage(float inDamage, bool headshot) -> float
{
    if (headshot == true)
    {
        return inDamage;
    }
    else
    {
        return inDamage * 2.0F;
    }
}
```
From there, the raw SGL goes through the compiler and is spit out as a custom bytecode that is probably quite similar to Java's from what I've read. This compilation can happen at runtime as the script is needed or changed, or pre-compiled and loaded from a file. Running the code from C++ will look something like this __(VERY subject to change)__:
```cpp
SGLScript myScript;
myScript.compile_from_source(some_sgl_source);
// or...
myScript.load_from_bytecode(some_sgl_bytecode);
// and then later...
float damage = myScript.execute_function<float, float, bool>("CalculateDamage", 4.0F, true); // returns 8.0F
```
Obviously not the prettiest, and once the language is actually functional, it's one of the first things I hope to improve. Again, _very subject to change_.

# What can it do right now?
SGL is in very early stages of development. It began development in mid-late April 2020. As of today (04/27/2020) it can properly compile expressions, including complex expressions with nested parentheses and variables. It can also run the compiled expressions, as long as you hand-copy the emitted bytecode to a buffer in C++ and send it through. It currently only supports 32-bit signed integer math, and function calls have not been worked in yet.

# What will it be able to do?
Long-term, here are some goals for SGL:
- Support calling C++ native functions by registering them with the VM
- Support manipulating C++ native objects by registering the objects with the VM
- Support internal function calls
- Support basic structs being declared in SGL
Given that this is a pet project so I can learn more about compilers and code execution in general, I have no timeline on any of these features.

# Can I test it?
Please! Bare in mind that it's extremely early in development and likely will have bugs. Download the source, and under Compiler.cpp you'll find a function called `execute_compiler_test`. Feel free to add your own calls to `parse_expression` and keep in mind that each call is modifying a global state, so subsequent calls can reference variables initialized in calls preceding them. The only type supported as of now is `int32` and function calls are WIP and not part of `parse_expression` yet.
