/**
 * All Test cases should extend this class.  This provides some framework
 * functionality for the test system.
 *
 * All Tests should be added to main::EnableAllTests.
 *
 */
#pragma once

#include "ApolloEngine.h"
#include "ApolloTestFramework.h"

// Validation macros
#define VALIDATE_NOT_NULL(x) if (!x) return false
#define VALIDATE_EQUALS(x, y) if (x != y) return false
#define VALIDATE_TRUE(x) if (!x) return false
#define VALIDATE_FALSE(x) if(x) return false

class ApolloTest {
public:
    virtual ~ApolloTest();

    virtual void Init()    = 0;
    virtual bool Execute() = 0;
    virtual void Cleanup() = 0;
    virtual const char* GetName() const = 0;
    virtual UINT64 GetTestCategory() const = 0;
    
    virtual UINT32 GetElapsedTime() {
        return m_timer.GetTimeElapsed();
    }

    UINT64 GetTestID() const;   
    void SetTestID(UINT ID) { m_TestID = ID; }

protected:
    ApolloTest();
    Apollo::PoolAllocator   m_allocator;
    Apollo::Timer           m_timer;


private:
    UINT64  m_TestID;
};
