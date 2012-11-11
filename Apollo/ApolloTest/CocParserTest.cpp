#include "CoCParserTest.h"
#include "CoCParser.h"

using namespace Apollo;
using namespace std;

bool CocParserTest::TestScene(const string& file, const string& reference) {
    string path = ApolloTestFramework::GetFilepath(file, ApolloTestFramework::DATA_SCENE);

    Scene scene;
    m_timer.Resume();
    CoC::CoCParser::Parse(path.c_str(), &scene);
    scene.Initialize();
    
	WhittedRayTracer::Settings settings;
    WhittedRayTracer tracer(settings);
    const Image* image = tracer.RenderAll(&scene);
    m_timer.Pause();

	return ApolloTestFramework::Instance()->ProcessResult(this, image, reference.c_str());
}

bool CocParserTest::Execute() {
	bool passed = true;
	passed &= TestScene("CoC\\cornell_box2.coc", "CornellBox.png");
	passed &= TestScene("CoC\\dragon.coc", "Dragon.png");
	passed &= TestScene("CoC\\ring2.coc", "Ring.png");
	passed &= TestScene("CoC\\teapot.coc", "Teapot.png");
//	passed &= TestScene("CoC\\piano.coc", "Piano.png");  // TODO: this passes but takes ~1min on debug
	return passed;
}