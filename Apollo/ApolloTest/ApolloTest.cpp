#include "ApolloTest.h"

ApolloTest::ApolloTest() : m_TestID((UINT)-1) {
}

ApolloTest::~ApolloTest() {
}

UINT64 ApolloTest::GetTestID() const {
    return m_TestID;
}
