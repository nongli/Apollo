#pragma once

#include "ApolloTest.h"

class BasicImageTest : public ApolloTest {
public:
    bool Execute();
    const char* GetName() const { return "BasicImageTest"; }
    UINT64 GetTestCategory() const { return ApolloTestImage; }
};

class ImageFilterTest : public ApolloTest {
public:
    bool Execute();
    const char* GetName() const { return "ImageFilterTest"; }
    UINT64 GetTestCategory() const { return ApolloTestImage; }
};

class CircularImageTest : public ApolloTest {
public:
    bool Execute();
    const char* GetName() const { return "CircularImageTest"; }
    UINT64 GetTestCategory() const { return ApolloTestImage; }
};
