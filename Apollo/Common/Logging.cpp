#include "Logging.h"

namespace Apollo {

NullLogger* NullLogger::s_Instance = new NullLogger;    
ConsoleLogger* ConsoleLogger::s_Instance = new ConsoleLogger;

ILogger* ILogger::s_logger = NullLogger::Instance();

void ILogger::Init(ILogger* logger)
{
    if (logger == nullptr) s_logger = NullLogger::Instance();
    else s_logger = logger;
}

}
