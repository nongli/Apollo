#pragma once

#include "ApolloTest.h"

class PerspectiveCameraBasicTest : public ApolloTest {
public:
    bool Execute();
    const char* GetName() const { return "PerspectiveCameraBasicTest"; }    
	UINT64 GetTestCategory() const { return ApolloTestRenderBase | ApolloTestCamera; }
};
