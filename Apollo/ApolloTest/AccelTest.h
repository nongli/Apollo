#pragma once

#include "ApolloTest.h"

class AccelTest : public ApolloTest {
public:
	bool Execute();
    const char* GetName() const { return "AccelTest"; }    
	UINT64 GetTestCategory() const { return ApolloTestAccel; }
};
