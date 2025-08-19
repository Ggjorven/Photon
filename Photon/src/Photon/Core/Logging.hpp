#pragma once

#include <Nano/Nano.hpp>

namespace Photon
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Logging & Asserting macros
    ////////////////////////////////////////////////////////////////////////////////////
    #ifndef PH_CONFIG_DIST
        #define PH_LOG_TRACE(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Trace>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define PH_LOG_INFO(fmt, ...)        ::Nano::Log::PrintLvl<::Nano::Log::Level::Info>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define PH_LOG_WARN(fmt, ...)        ::Nano::Log::PrintLvl<::Nano::Log::Level::Warn>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define PH_LOG_ERROR(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Error>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define PH_LOG_FATAL(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Fatal>(fmt __VA_OPT__(,) __VA_ARGS__)

        #if !defined(PH_ASSSERT)
            #define PH_ASSERT(x, fmt, ...)  \
                        do                          \
                        {                           \
                            if (!(x))               \
                            {                       \
                                PH_LOG_FATAL("Assertion failed: ({0}), {1}.", #x, ::Nano::Text::Format(fmt __VA_OPT__(,) __VA_ARGS__)); \
                                NANO_DEBUG_BREAK(); \
                            }                       \
                        } while (false)
        #endif

        #define PH_VERIFY(x, fmt, ...)  \
                    do                          \
                    {                           \
                        if (!(x))               \
                        {                       \
                            PH_LOG_FATAL("Verify failed: ({0}), {1}.", #x, ::Nano::Text::Format(fmt __VA_OPT__(,) __VA_ARGS__)); \
                        }                       \
                    } while (false)

        #define PH_UNREACHABLE() PH_ASSERT(false, "Unreachable")

    #else
        #define PH_LOG_TRACE(fmt, ...) 
        #define PH_LOG_INFO(fmt, ...) 
        #define PH_LOG_WARN(fmt, ...) 
        #define PH_LOG_ERROR(fmt, ...) 
        #define PH_LOG_FATAL(fmt, ...) 

        #define PH_ASSERT(x, fmt, ...)
        #define PH_VERIFY(x, fmt, ...)
        #define PH_UNREACHABLE()
    #endif

}