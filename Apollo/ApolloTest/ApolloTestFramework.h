#pragma once

#include "ApolloEngine.h"
#include "ImageDiffer.h"

const UINT64 ApolloTestAll          =   0;
const UINT64 ApolloTestMisc         =   1 << 0;
const UINT64 ApolloTestImage        =   1 << 1;
const UINT64 ApolloTestRenderBase   =   1 << 2;
const UINT64 ApolloTestCamera		=	1 << 3;
const UINT64 ApolloTestAntialiasing	=	1 << 4;
const UINT64 ApolloTestParser	    =	1 << 5;

class ApolloTest;

class ApolloTestFramework {
public:
    ApolloTestFramework(bool clean);
    ~ApolloTestFramework();

    void AddTest(ApolloTest* test, UINT ID);

    void ExecuteAllTests();
    void ExecuteTestNum(UINT64 TestID);
    void ExecuteTests(UINT64 TestCategories);

    UINT32 GetNumPassed() const;
    UINT32 GetNumFailed() const;
    UINT32 GetNumTests() const;
    UINT32 GetNumTestsRun() const;

    const UINT64* GetFailedTest() const;

    void SetSaveAllOutput(bool save);

public:
    static ApolloTestFramework* Instance();
    bool ProcessResult(const ApolloTest* test, const Apollo::Image* image, const char* ref);

public:
    enum FILE_TYPE {
        DATA_IMAGE,
        DATA_SCENE,
        RESULT_REFERENCE,
        RESULT_FAILED,
        RESULT_DIFF,
        RESULT_MISC,
        RESULT_TEMP,
    };

public:
    static void AppendTestName(std::string& string, const ApolloTest* test);
    static const char* GetFileLocation(FILE_TYPE type);
    static std::string GetFilepath(const std::string& fileName, FILE_TYPE type);

    static Apollo::Image* LoadImage(const char* path, FILE_TYPE);
    static void SaveImage(const ApolloTest* test, const Apollo::Image* image, const char* path, FILE_TYPE type);

    static bool AreImagesIdentical(const Apollo::Image* image1, const Apollo::Image* image2);

private:
    void CleanupResults() const;
    void DeleteDir(const char* dir) const;
    void ExecuteTest(ApolloTest* test);

    UINT32                      m_nPassed;
    UINT32                      m_nFailed;
    UINT32                      m_nTestsRun;
    std::vector<ApolloTest*>    m_tests;
    std::vector<UINT64>         m_failedTests;
    bool                        m_clean;
    bool                        m_saveAllOutput;
};
