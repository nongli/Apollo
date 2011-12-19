// ImageAnalogies.cpp : Defines the entry point for the console application.
//

#include "ApolloCommon.h"
#include "ArgsParser.h"

#include "Image.h"
#include "ImageIO.h"
#include "UIHelper.h"

using namespace Apollo;
#include "TextureSynthesis.h"
#include "ImageAnalogy.h"

//#define TEXTURE_SYNTHESIS
#define INPUTFILE "starryNightBarn"

const char* INPUT = "B:\\data\\ImageAnalogies\\Input\\"INPUTFILE".txt";
const char* OUTPUT = "B:\\data\\"INPUTFILE".png";
//const char* INPUT = ".\\data\\colorize.txt";
//const char* OUTPUT = ".\\imageanalogy.png";


struct ImageAnalogyParams 
{
	const Image*		            A;
	const Image*		            Aprime;
	const Image*		            B;
	ImageAnalogy::FEATURE_TYPE	    featureType;
	ImageAnalogy::POSTPROCESS_TYPE	postProcessType;
	int					            pyramidLevels;
	float				            coherenceFactor;
	float				            annError;
	bool				            luminanceRemapping;
};

void ParseArgs(const char* inputFile, ImageAnalogyParams& params);
void Usage();

int main(int argc, char** argv)
{    
    ILogger::Init(ConsoleLogger::Instance());
 
    std::string input = INPUT;
    std::string output = OUTPUT;

    if (argc != 1 && argc != 3)
    {
        Usage();
        goto end;
    }
    if (argc == 3)
    {
        input = argv[1];
        output = argv[2];
    }

    try
    {
        Timer::Init();
        ImageIO::Init();

#ifdef TEXTURE_SYNTHESIS
        Timer timer;
        Image* tile = ImageIO::Load("b:\\data\\tile.png");        
        Image* result = new Image(800, 640);
	    TextureSynthesisParams params;
	    params.regionWidth = 5;
	    params.regionHeight = 5;
        
        timer.Start();
	    TextureSynthesis::SynthesizeTexture(tile, result, params);
        ILogger::Logger()->Status("Time to synthesize texture: %s.",UIHelper::DisplayTime(timer.Stop()).c_str());

        ImageIO::Save("b:\\data\\texture.png", result);
        delete tile;
        delete result;
#else
        // Parse input
        ImageAnalogyParams params;
        ParseArgs(input.c_str(), params);
        
        // Create ImageAnalogy
        ImageAnalogy analogy;
	    analogy.setPyramidLevels(params.pyramidLevels);
	    analogy.setCoherence(params.coherenceFactor);
	    analogy.setANNError(params.annError);
	    analogy.setLuminanceRemapping(params.luminanceRemapping);
	    analogy.setFeatureType(params.featureType);
	    analogy.setPostProcess(params.postProcessType);

	    Timer timer;
        timer.Start();
	    Image* Bprime = analogy.createAnalogy(params.A, params.Aprime, params.B);
        ILogger::Logger()->Status("Time to create analogy: %s.", UIHelper::DisplayTime(timer.Stop()).c_str());
                
        // Save results
	    Image* result = ImageAnalogy::combineImages(params.A, params.Aprime, params.B, Bprime);
        ImageIO::Save(output.c_str(), result);

#endif
    }
    catch (const ApolloException& e)
    {
        ILogger::Logger()->Error("Image Analogies Error: %s.", e.what());
    }

end:
    ILogger::Logger()->Status("Done.");
    getchar();
	return 0;
}

void ParseArgs(const char* inputFile, ImageAnalogyParams& params) {
	/* Load default values */
	params.A					=	nullptr;
	params.Aprime				=	nullptr;
	params.B					=	nullptr;
    params.featureType			=	ImageAnalogy::FEATURE_LUMINANCE;
	params.postProcessType		=	ImageAnalogy::POSTPROCESS_B_COLORS;
	params.pyramidLevels		=	10;
	params.coherenceFactor		=	1.0f;
	params.annError				=	5.0f;
	params.luminanceRemapping	=	false;

	std::string Afile, Bfile, APrimeFile, BPrimeFile, resultFile;
	std::string featureType, postProcessType;

	BPrimeFile = "BPrime.png";

	/* Configure the parser */
	ArgsParser parser;
	parser.AddStringArg("A", Afile, true);
	parser.AddStringArg("B", Bfile, true);
	parser.AddStringArg("Aprime", APrimeFile, true);
	parser.AddStringArg("FeatureType", featureType);
	parser.AddStringArg("PostProcessType", postProcessType);
	parser.AddIntArg("PyramidLevels", params.pyramidLevels);
	parser.AddFloatArg("Coherence", params.coherenceFactor);
	parser.AddFloatArg("AnnError", params.annError);
	parser.AddBoolArg("LuminanceRemapping", params.luminanceRemapping);

	/* Parse the values */
	parser.Parse(inputFile);

	/* Post process */
	params.A = ImageIO::Load(Afile.c_str());
	params.Aprime = ImageIO::Load(APrimeFile.c_str());
	params.B = ImageIO::Load(Bfile.c_str());
    
	if (params.A == nullptr || params.Aprime == nullptr || params.B == nullptr)
        throw ApolloException("Invalid input file.");

	if (!strcmp(featureType.c_str(), "luminance")) {
		params.featureType = ImageAnalogy::FEATURE_LUMINANCE;
	}
	else if (!strcmp(featureType.c_str(), "rgb")) {
		params.featureType = ImageAnalogy::FEATURE_RGB;
	}

	if (!strcmp(postProcessType.c_str(), "Aprime")) {
		params.postProcessType = ImageAnalogy::POSTPROCESS_APRIME_COLORS;
	}
	else if (!strcmp(postProcessType.c_str(), "B")) {
		params.postProcessType = ImageAnalogy::POSTPROCESS_B_COLORS;
	}
};

void Usage()
{
    ILogger::Logger()->Message("Usage: ImageAnalogies <Input> <Output>.");
}
