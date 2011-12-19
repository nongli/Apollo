#pragma once

#include "ApolloTest.h"

class TwoBallsNoAATest : public ApolloTest {
public:
    TwoBallsNoAATest();
    void Init();
    bool Execute();
    void Cleanup();
    const char* GetName() const { return "TwoBallsNoAATest"; }    
    UINT64 GetTestCategory() const { return ApolloTestRenderBase; }
};

class BasicIntersectionTest : public ApolloTest {
public:
    BasicIntersectionTest();
    void Init();
    bool Execute();
    void Cleanup();
    const char* GetName() const { return "BasicIntersectionTest"; }    
    UINT64 GetTestCategory() const { return ApolloTestRenderBase; }
};

class BasicTextureTest : public ApolloTest {
public:
    BasicTextureTest();
    void Init();
    bool Execute();
    void Cleanup();
    const char* GetName() const { return "BasicTextureTest"; }    
    UINT64 GetTestCategory() const { return ApolloTestRenderBase; }
};

class CheckerboardTest : public ApolloTest {
public:
    CheckerboardTest();
    void Init();
    bool Execute();
    void Cleanup();
    const char* GetName() const { return "CheckerboardTest"; }    
	UINT64 GetTestCategory() const { return ApolloTestRenderBase | ApolloTestAntialiasing; }
};

class BasicSceneTest : public ApolloTest {
public:
    BasicSceneTest();
    void Init();
    bool Execute();
    void Cleanup();
    const char* GetName() const { return "BasicSceneTest"; }    
    UINT64 GetTestCategory() const { return ApolloTestRenderBase; }
};
