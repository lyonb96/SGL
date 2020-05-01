#pragma once

#include <string>

/**
 * Compiler V2
 * 
 * The old compiler file was littered with... lessons learned, to put it nicely.
 * I wanted to start fresh and keep only the parts that have worked so far, as 
 * well as begin building out a more sensible architecture.
 *
 * I don't want the compiler to be tied to an instance of some class, because I
 * would like for scripts to be able to handle their own recompilation at runtime.
 * Therefore instead of something like "class SGLCompiler" I'd rather have functions
 * exposed to handle it.
 */

namespace SGL
{
    bool compile_source(std::string source);
}