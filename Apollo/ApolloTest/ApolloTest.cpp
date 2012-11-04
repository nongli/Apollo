#include "ApolloTest.h"
#include "ApolloEngine.h"

using namespace Apollo;

ApolloTest::ApolloTest() : m_TestID((UINT)-1) {
}

ApolloTest::~ApolloTest() {
}

void ApolloTest::Init() {
    ImageIO::Init();
    Timer::Init();
    SamplerUtil::Init(false);
}

void ApolloTest::Cleanup() {
    Apollo::ImageIO::Destroy();
}

UINT64 ApolloTest::GetTestID() const {
    return m_TestID;
}
