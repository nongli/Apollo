#include "ApolloTestFramework.h"
#include "ApolloTest.h"

using namespace Apollo;
using namespace std;

ImageDiffer m_differ;
ApolloTestFramework* s_instance;

ApolloTestFramework::ApolloTestFramework(bool clean) {
    m_nPassed = 0;
    m_nFailed = 0;
    m_nTestsRun = 0;
    m_tests.clear();
    m_clean = clean;
    m_saveAllOutput = false;
    m_differ.SetThreshold(.05f);
    assert(s_instance == nullptr);
    s_instance = this;
}

ApolloTestFramework* ApolloTestFramework::Instance() {
    return s_instance;
}

ApolloTestFramework::~ApolloTestFramework() {
    for (UINT32 i = 0; i < m_tests.size(); i++) {
        delete m_tests[i];
    }
}

UINT32 ApolloTestFramework::GetNumPassed() const {
    return m_nPassed;
}

UINT32 ApolloTestFramework::GetNumFailed() const {
    return m_nFailed;
}

UINT32 ApolloTestFramework::GetNumTests() const {
    return m_tests.size();
}

UINT32 ApolloTestFramework::GetNumTestsRun() const {
    return m_nTestsRun;
}

void ApolloTestFramework::AddTest(ApolloTest* test, UINT ID) {
    test->SetTestID(ID);
    m_tests.push_back(test);
}

const UINT64* ApolloTestFramework::GetFailedTest() const {
    return &m_failedTests[0];
}

void ApolloTestFramework::CleanupResults() const {
    DeleteDir(GetFileLocation(RESULT_FAILED));
    DeleteDir(GetFileLocation(RESULT_MISC));
    DeleteDir(GetFileLocation(RESULT_DIFF));
}

void ApolloTestFramework::SetSaveAllOutput(bool save) {
    m_saveAllOutput = save;
}

void ApolloTestFramework::ExecuteTest(ApolloTest* test) {
    m_nTestsRun++;
    try {
        test->Init();
        printf("    Executing Test: %s", test->GetName());
        if (test->Execute()) {
            m_nPassed++;                
            printf(": %d(ms)\n", test->GetElapsedTime());
        }
        else {
            m_failedTests.push_back(test->GetTestID());
            m_nFailed++;
            printf("\n");
        }
        test->Cleanup();
    } catch (ApolloException& e) {
        printf(": Exception caught: %s\n", e.what());
        m_failedTests.push_back(test->GetTestID());
        m_nFailed++;
    } catch (...) {
		printf(": Exception caught.\n");
        m_failedTests.push_back(test->GetTestID());
        m_nFailed++;
    }
}

void ApolloTestFramework::ExecuteAllTests() {
    if (m_clean) {
        CleanupResults();
    }

    for (UINT32 i = 0; i < m_tests.size(); i++) {
        ExecuteTest(m_tests[i]);
    }
}

void ApolloTestFramework::ExecuteTests(UINT64 TestCategories) {
    if (m_clean) {
        CleanupResults();
    }

    for (UINT32 i = 0; i < m_tests.size(); i++) {
        if (m_tests[i]->GetTestCategory() & TestCategories) {
            ExecuteTest(m_tests[i]);
        }
    }
}

void ApolloTestFramework::ExecuteTestNum(UINT64 TestID) {
    if (m_clean) {
        CleanupResults();
    }

    for (UINT32 i = 0; i < m_tests.size(); i++) {
        if (m_tests[i]->GetTestID() == TestID) {
            ExecuteTest(m_tests[i]);
        }
    }
}

const char* ApolloTestFramework::GetFileLocation(FILE_TYPE type) {    
    switch (type) {
    case DATA_IMAGE:
        return "..\\Data\\Images\\";
    case DATA_SCENE:
        return "..\\Data\\Scenes\\";
    case RESULT_DIFF:
        return "..\\Results\\Diff\\";
    case RESULT_FAILED:
        return "..\\Results\\Failed\\";
    case RESULT_MISC:
        return "..\\Results\\Misc\\";
    case RESULT_REFERENCE:
        return "..\\Results\\Reference\\";
    case RESULT_TEMP:
        return "..\\Results\\Temp\\";
    }
    return "";
}

void ApolloTestFramework::AppendTestName(string& string, const ApolloTest* test) {
    static char buf[256];
    _itoa_s((int)(test->GetTestID()), buf, 256, 10);

    string += "Test";
    string += buf;
    string += "-";
}

string ApolloTestFramework::GetFilepath(const string& file, FILE_TYPE type) {
    string path = GetFileLocation(type);
    path += file;
    return path;
}

Image* ApolloTestFramework::LoadImage(const char* path, FILE_TYPE type) {
    string p = GetFilepath(path, type);
    ImageData imageData;
    ImageIO::Load(p.c_str(), &imageData);
    Image* image = new Image(&imageData);
    ImageIO::Deallocate(&imageData);
    return image;
}

void ApolloTestFramework::SaveImage(const ApolloTest* test, const Image* image, const char* name, FILE_TYPE type) {
    string p = ApolloTestFramework::GetFileLocation(type);
    AppendTestName(p, test);
    p += name;
    ImageIO::Save(p.c_str(), image);
}

bool ApolloTestFramework::ProcessResult(const ApolloTest* test, const Image* image, const char* ref) {
    string p = ApolloTestFramework::GetFileLocation(RESULT_REFERENCE);
    AppendTestName(p, test);
    p += ref;

    bool failed = false;

    try {
        ImageData imageData;
        ImageIO::Load(p.c_str(), &imageData);
        Image imageRef(&imageData);
        ImageIO::Deallocate(&imageData);
        failed = !m_differ.Diff(*image, imageRef);

        if (failed) {
            const Image* diffImage = m_differ.GetDiffData();
            SaveImage(test, diffImage, ref, RESULT_DIFF);
        }
    } catch (...) {
        failed = true;
    }

    if (failed) {
        SaveImage(test, image, ref, RESULT_FAILED);
    }

    if (m_saveAllOutput) {
        SaveImage(test, image, ref, RESULT_TEMP);
    }

    return !failed;
}

bool ApolloTestFramework::AreImagesIdentical(const Image* image1, const Image* image2) {
    return m_differ.Diff(*image1, *image2);
}
