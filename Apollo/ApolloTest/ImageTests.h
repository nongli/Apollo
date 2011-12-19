#pragma once

#include "ApolloTest.h"

class BasicImageTest : public ApolloTest {
public:
    BasicImageTest();
    void Init();
    bool Execute();
    void Cleanup();
    const char* GetName() const { return "BasicImageTest"; }
    UINT64 GetTestCategory() const { return ApolloTestImage; }
};

class ImageFilterTest : public ApolloTest {
public:
    ImageFilterTest();
    void Init();
    bool Execute();
    void Cleanup();
    const char* GetName() const { return "ImageFilterTest"; }
    UINT64 GetTestCategory() const { return ApolloTestImage; }
};

class CircularImageTest : public ApolloTest {
public:
    CircularImageTest();
    void Init();
    bool Execute();
    void Cleanup();
    const char* GetName() const { return "CircularImageTest"; }
    UINT64 GetTestCategory() const { return ApolloTestImage; }
};
