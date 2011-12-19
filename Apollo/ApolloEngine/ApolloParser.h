#pragma once

#include "ApolloCommon.h"

namespace Apollo {
    class ParserException : public ApolloException {
    public:
	    ParserException(std::string message) : ApolloException(message) {}
    };
}
