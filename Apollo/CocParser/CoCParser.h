#pragma once

#include "ApolloEngine.h"

namespace CoC {

    class CoCParserException : Apollo::ParserException {
        public:
            CoCParserException(const std::string& message) : ParserException(message) {}
            static void Throw(const std::string& tag);
    };

    /** 
     * Parses scenes in the CoC format.  
     */
    class CoCParser {
        public:
            static void Parse(const char* filename, Apollo::Scene*);
    };
};
