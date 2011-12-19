#include "CoCParserTest.h"
#include "CoCParser.h"

using namespace Apollo;
using namespace std;

CocParserTest::CocParserTest() : ApolloTest() {
}

void CocParserTest::Init() {
    ImageIO::Init();
    Timer::Init();
    SamplerUtil::Init(false);
}

void CocParserTest::Cleanup() {
    Apollo::ImageIO::Destroy();
}

bool CocParserTest::Execute() {
    Scene scene;
    m_timer.Start();
    string file = ApolloTestFramework::GetFilepath("CoC\\cornell_box_ray.coc", ApolloTestFramework::DATA_SCENE);
    CoC::CoCParser::Parse(file.c_str(), &scene);
    scene.Initialize();
    WhittedRayTracer tracer;
    const Image* image = tracer.RenderAll(&scene);
    m_timer.Stop();
    
    ApolloTestFramework::SaveImage(this, image, "Result.png", ApolloTestFramework::RESULT_MISC);
    return true;
}