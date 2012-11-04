#pragma once

#include "ApolloTest.h"

class TwoBallsNoAATest : public ApolloTest {
public:
    bool Execute();
    const char* GetName() const { return "TwoBallsNoAATest"; }    
    UINT64 GetTestCategory() const { return ApolloTestRenderBase; }
};

class BasicIntersectionTest : public ApolloTest {
public:
    bool Execute();
    const char* GetName() const { return "BasicIntersectionTest"; }    
    UINT64 GetTestCategory() const { return ApolloTestRenderBase; }
};

class BasicTextureTest : public ApolloTest {
public:
    bool Execute();
    const char* GetName() const { return "BasicTextureTest"; }    
    UINT64 GetTestCategory() const { return ApolloTestRenderBase; }
};

class CheckerboardTest : public ApolloTest {
public:
    bool Execute();
    const char* GetName() const { return "CheckerboardTest"; }    
	UINT64 GetTestCategory() const { return ApolloTestRenderBase | ApolloTestAntialiasing; }
};

class BasicSceneTest : public ApolloTest {
public:
    bool Execute();
    const char* GetName() const { return "BasicSceneTest"; }    
    UINT64 GetTestCategory() const { return ApolloTestRenderBase; }
};
