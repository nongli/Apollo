#pragma once

#include "ApolloTest.h"

class PerspectiveCameraBasicTest : public ApolloTest {
public:
    PerspectiveCameraBasicTest();
    void Init();
    bool Execute();
    void Cleanup();
    const char* GetName() const { return "PerspectiveCameraBasicTest"; }    
	UINT64 GetTestCategory() const { return ApolloTestRenderBase | ApolloTestCamera; }
};
