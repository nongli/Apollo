#pragma once

#include "ApolloTest.h"

class AllocatorTest : public ApolloTest {
public:
    AllocatorTest();
    void Init();
    bool Execute();
    void Cleanup();
    const char* GetName() const { return "AllocatorTest"; }    
	UINT64 GetTestCategory() const { return ApolloTestMisc; }
};
