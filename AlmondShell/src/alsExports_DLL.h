#pragma once

#include <string>

// API Visibility Macros
#ifdef _WIN32
    // Windows DLL export/import logic
    #ifndef ENGINE_STATICLIB
        #ifdef ENGINE_DLL_EXPORTS
            #define ENGINE_API __declspec(dllexport)
        #else
            #define ENGINE_API __declspec(dllimport)
        #endif
    #else
        #define ENGINE_API  // Static library, no special attributes
    #endif
#else
    // GCC/Clang visibility attributes
    #if (__GNUC__ >= 4) && !defined(ENGINE_STATICLIB) && defined(ENGINE_DLL_EXPORTS)
        #define ENGINE_API __attribute__((visibility("default")))
    #else
        #define ENGINE_API  // Static library or unsupported compiler
    #endif
#endif

// Calling Convention Macros
#ifndef _STDCALL_SUPPORTED
    #define ALECALLCONV __cdecl
#else
    #define ALECALLCONV __stdcall
#endif
