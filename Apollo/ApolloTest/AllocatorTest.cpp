#include "MiscTests.h"

using namespace Apollo;

static volatile int BarInstances = 0;
static volatile int FooInstances = 0;

struct Foo {
    double x, y, z;
    Foo() {
        FooInstances++;
        x = 1;
        y = 2;
        z = 3;
    }

    ~Foo() {
        FooInstances--;
    }
};

struct Bar {
    int a, b, c;

    Bar() {
        a = b = c = 10;
        BarInstances++;
    }

    ~Bar() {
        BarInstances--;
    }
};


AllocatorTest::AllocatorTest() : ApolloTest()
{
}

void AllocatorTest::Init()
{
}

bool AllocatorTest::Execute() {   
    {
        Foo* foo = new Foo;
        delete foo;
    }
    PoolAllocator* allocator = new PoolAllocator;

    VALIDATE_EQUALS(BarInstances, 0);
    VALIDATE_EQUALS(FooInstances, 0);

    Foo* foo = allocator->Allocate<Foo>();
    Bar* bar = allocator->Allocate<Bar>();
    
    VALIDATE_EQUALS(FooInstances, 1);
    VALIDATE_EQUALS(BarInstances, 1);
    VALIDATE_EQUALS(foo->x, 1);
    VALIDATE_EQUALS(foo->y, 2);
    VALIDATE_EQUALS(bar->a, 10);
    bar->a++;
    VALIDATE_EQUALS(bar->a, 11);

    allocator->FreeAll();
    VALIDATE_EQUALS(BarInstances, 0);
    VALIDATE_EQUALS(FooInstances, 0);

    return true;
}

void AllocatorTest::Cleanup() {
}
