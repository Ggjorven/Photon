#pragma once

#include <Nano/Nano.hpp>

namespace Nano::Networking
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Logging & Asserting macros
    ////////////////////////////////////////////////////////////////////////////////////
    #ifndef NN_CONFIG_DIST
        #define NN_LOG_TRACE(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Trace>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define NN_LOG_INFO(fmt, ...)        ::Nano::Log::PrintLvl<::Nano::Log::Level::Info>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define NN_LOG_WARN(fmt, ...)        ::Nano::Log::PrintLvl<::Nano::Log::Level::Warn>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define NN_LOG_ERROR(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Error>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define NN_LOG_FATAL(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Fatal>(fmt __VA_OPT__(,) __VA_ARGS__)

        #define NN_ASSERT(x, fmt, ...)  \
                    do                          \
                    {                           \
                        if (!(x))               \
                        {                       \
                            NN_LOG_FATAL("Assertion failed: ({0}), {1}.", #x, ::Nano::Text::Format(fmt __VA_OPT__(,) __VA_ARGS__)); \
                            NANO_DEBUG_BREAK(); \
                        }                       \
                    } while (false)

        #define NN_VERIFY(x, fmt, ...)  \
                    do                          \
                    {                           \
                        if (!(x))               \
                        {                       \
                            NN_LOG_FATAL("Verify failed: ({0}), {1}.", #x, ::Nano::Text::Format(fmt __VA_OPT__(,) __VA_ARGS__)); \
                        }                       \
                    } while (false)

        #define NN_UNREACHABLE() NN_ASSERT(false, "Unreachable")

    #else
        #define NN_LOG_TRACE(fmt, ...) 
        #define NN_LOG_INFO(fmt, ...) 
        #define NN_LOG_WARN(fmt, ...) 
        #define NN_LOG_ERROR(fmt, ...) 
        #define NN_LOG_FATAL(fmt, ...) 

        #define NN_ASSERT(x, fmt, ...)
        #define NN_VERIFY(x, fmt, ...)
        #define NN_UNREACHABLE()
    #endif

}