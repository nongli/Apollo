#pragma once
#include <stdio.h>
#include <stdarg.h>

namespace Apollo {
    class ILogger {
    public:
        virtual void Log(const char* format, ...) = 0;
        virtual void Error(const char* format, ...) = 0;
        virtual void Warn(const char* format, ...) = 0;
        virtual void Status(const char* format, ...) = 0;
        virtual void Debug(const char* format, ...) = 0;
        virtual void Message(const char* format, ...) = 0;

    public:
        static ILogger* Logger() {
            return s_logger;
        }

        static void Init(ILogger* logger = nullptr);

    private:
        static ILogger* s_logger;
    };

    class NullLogger : public ILogger {
    public:
        void Log(const char*, ...) {}
        void Error(const char*, ...) {}
        void Warn(const char*, ...) {}
        void Status(const char*, ...) {}
        void Debug(const char*, ...) {}
        void Message(const char*, ...) {}

    public:        
        static NullLogger* Instance() { return s_Instance; }

    private:
        static NullLogger* s_Instance;
    };

    class ConsoleLogger : public ILogger {
    public:
#define CONSOLE_OUT         \
    va_list ap;             \
    va_start(ap, format);   \
    vprintf(format, ap);    \
    va_end(ap);             \
    printf("\n");
    
        void Log(const char* format, ...) {
            printf("Log: ");
            CONSOLE_OUT
        }        

        void Error(const char* format, ...) {
            printf("Error: ");
            CONSOLE_OUT
        }
        
        void Warn(const char* format, ...) {
            printf("Warn: ");
            CONSOLE_OUT
        }

        void Status(const char* format, ...) {
            printf("Status: ");
            CONSOLE_OUT
        }
        
        void Debug(const char* format, ...) {
            printf("Debug: ");
            CONSOLE_OUT
        }        
        void Message(const char* format, ...) {
            CONSOLE_OUT
        }

        static ConsoleLogger* Instance() { return s_Instance; }

    private:
        static ConsoleLogger* s_Instance;
    };
}
