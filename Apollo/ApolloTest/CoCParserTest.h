#pragma once

#include "ApolloTest.h"

class CocParserTest : public ApolloTest {
public:
    CocParserTest();
    void Init();
    bool Execute();
    void Cleanup();
    const char* GetName() const { return "CocParserTest"; }    
	UINT64 GetTestCategory() const { return ApolloTestParser; }
};
