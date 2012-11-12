#include "ApolloTestFramework.h"
#include "ApolloTest.h"

#include "CameraTests.h"
#include "CoCParserTest.h"
#include "ImageTests.h"
#include "MiscTests.h"
#include "RenderTests.h"
#include "AccelTest.h"

ApolloTestFramework g_TestFramework(true);

void EnableAllTests() {    
    g_TestFramework.AddTest(new BasicImageTest(), 1);
    g_TestFramework.AddTest(new ImageFilterTest(), 2);
    g_TestFramework.AddTest(new TwoBallsNoAATest(), 3);
	g_TestFramework.AddTest(new BasicIntersectionTest(), 4);
    g_TestFramework.AddTest(new BasicTextureTest(), 5);
	g_TestFramework.AddTest(new PerspectiveCameraBasicTest(), 6);
	g_TestFramework.AddTest(new CheckerboardTest(), 7);
    g_TestFramework.AddTest(new CircularImageTest(), 8);
    g_TestFramework.AddTest(new BasicSceneTest(), 9);
    g_TestFramework.AddTest(new AllocatorTest(), 10);
    g_TestFramework.AddTest(new CocParserTest(), 11);
    g_TestFramework.AddTest(new AccelTest(), 12);
    g_TestFramework.AddTest(new CornellBoxTest(), 13);
}

void DisplayResults() {
    printf("Total Tests: %d\n", g_TestFramework.GetNumTestsRun());
    printf("Tests Passed: %d\n", g_TestFramework.GetNumPassed());
    printf("Tests Failed: %d\n", g_TestFramework.GetNumFailed());

    if (g_TestFramework.GetNumFailed()) {
        printf("\n");
        printf("Tests Failed (ID):");
        const UINT64* failed = g_TestFramework.GetFailedTest();
        for (UINT32 i = 0; i < g_TestFramework.GetNumFailed(); i++)
        {
            printf(" %d", failed[i]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char** argv) {
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    printf("Running Test Framework.\n\n");
    EnableAllTests();    
    g_TestFramework.SetSaveAllOutput(true);

    printf("Executing Tests...\n");
	//g_TestFramework.ExecuteAllTests();
    //g_TestFramework.ExecuteTests(ApolloTestRenderBase);
	
    //g_TestFramework.ExecuteTestNum(1);
	//g_TestFramework.ExecuteTestNum(2);
	//g_TestFramework.ExecuteTestNum(3);
	//g_TestFramework.ExecuteTestNum(4);
	//g_TestFramework.ExecuteTestNum(5);
	//g_TestFramework.ExecuteTestNum(6);
	//g_TestFramework.ExecuteTestNum(7);
	//g_TestFramework.ExecuteTestNum(8);
	//g_TestFramework.ExecuteTestNum(9);
	//g_TestFramework.ExecuteTestNum(10);
	//g_TestFramework.ExecuteTestNum(11);
	//g_TestFramework.ExecuteTestNum(12);
	g_TestFramework.ExecuteTestNum(13);

    printf("\n\n");
    DisplayResults();

    printf("Done runing Tests.\n");
    char c;
	scanf_s("%c", &c);
    return 0;
}
