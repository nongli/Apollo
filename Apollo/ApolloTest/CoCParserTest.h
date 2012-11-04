#pragma once

#include "ApolloTest.h"

class CocParserTest : public ApolloTest {
public:
    bool Execute();
    const char* GetName() const { return "CocParserTest"; }    
	UINT64 GetTestCategory() const { return ApolloTestParser; }
};
