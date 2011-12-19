#include "AdaptiveSuperSampler.h"
#include "RenderBucket.h"

namespace Apollo 
{

//static FLOAT NODE_SIZES[] = {.25f, .0625f, .015625f, .00390625f };
static FLOAT NODE_SIZES[] = {.33f, .0625f, .156625f, .00390625f };
#define GET_NODE_SIZE(node) NODE_SIZES[node.depth]

bool AdaptiveSuperSampler::IsColorDifferenceAboveThreshold(const Color4f& c1, const Color4f& c2) const {
    FLOAT diffR = fabs(c1.r - c2.r);
    FLOAT diffG = fabs(c1.g - c2.g);
    FLOAT diffB = fabs(c1.b - c2.b);
    FLOAT diffA = fabs(c1.a - c2.a);

    diffR /= MAX(c1.r, c2.r);    
    diffG /= MAX(c1.g, c2.g);
    diffB /= MAX(c1.b, c2.b);
    diffA /= MAX(c1.a, c2.a);
    
    return  diffR > m_settings.threshold.r ||
            diffR > m_settings.threshold.g ||
            diffB > m_settings.threshold.b ||
            diffA > m_settings.threshold.a;
}

void AdaptiveSuperSampler::AddFourSamples(AdaptiveSampleNode& node, const Color4f& c, const Model* model) {
    m_stack[m_size].x           = node.x - GET_NODE_SIZE(node);
    m_stack[m_size].y           = node.y - GET_NODE_SIZE(node);
    m_stack[m_size].weight      = node.weight / 5;
    m_stack[m_size].depth       = node.depth + 1;
    m_stack[m_size].color       = c;
    m_stack[m_size].model       = model;
    m_size++;

    m_stack[m_size].x           = node.x + GET_NODE_SIZE(node);
    m_stack[m_size].y           = node.y - GET_NODE_SIZE(node);
    m_stack[m_size].weight      = node.weight / 5;
    m_stack[m_size].depth       = node.depth + 1;
    m_stack[m_size].color       = c;
    m_stack[m_size].model       = model;
    m_size++;

    m_stack[m_size].x           = node.x - GET_NODE_SIZE(node);
    m_stack[m_size].y           = node.y + GET_NODE_SIZE(node);
    m_stack[m_size].weight      = node.weight / 5;
    m_stack[m_size].depth       = node.depth + 1;
    m_stack[m_size].color       = c;
    m_stack[m_size].model       = model;
    m_size++;

    m_stack[m_size].x           = node.x + GET_NODE_SIZE(node);
    m_stack[m_size].y           = node.y + GET_NODE_SIZE(node);
    m_stack[m_size].weight      = node.weight / 5;
    m_stack[m_size].depth       = node.depth + 1;
    m_stack[m_size].color       = c;
    m_stack[m_size].model       = model;
    m_size++;
}

bool AdaptiveSuperSampler::GetNextSample(PrimarySample& sample) {
    // Common case
    //      - sample.GetSampleColor() is color of the evaluated sub-pixel
    //      - m_stack[m_size] is the color to compare against    
    if (m_commonCase) {
        const Color4f& color1 = m_stack[m_size].color;
        const Color4f& color2 = sample.GetSampleColor();
        if (m_stack[m_size].depth < m_settings.maxLevels &&
            (m_stack[m_size].model != sample.model ||
            IsColorDifferenceAboveThreshold(color1, color2))
           ) {
            AddFourSamples(m_stack[m_size], color2, sample.model);
        } else {
            // The weights assume the sample is going to be split so 
            // readd the value
            GetSamplePixelColor(sample).MultAggregate(color2, m_stack[m_size].weight * 4);
        }
    }

    // Case for setting up the first level sub pixels
    if (m_firstSample) {
        AdaptiveSampleNode node = m_stack[0];
        AddFourSamples(node, sample.GetSampleColor(), sample.model);
        m_firstSample = false;
        m_commonCase  = true;
    }

    // Case for just starting the pixel
    if (m_size == 0) {
        SetCurrentPixel(sample);
        if (!m_bucket->GetNextPixel(m_row, m_col)) return false;

        m_firstSample = true;
        m_commonCase  = false;
        
        // Add the center pixel
        m_stack[0].x        = (FLOAT)m_col;
        m_stack[0].y        = (FLOAT)m_row;
        m_stack[0].weight   = .20f;
        m_stack[0].depth    = 0;
        m_size++;         
    }

    m_size--;
    sample.xFilm    = m_stack[m_size].x;
    sample.yFilm    = m_stack[m_size].y;
    SetSampleRowCol(sample, m_row, m_col);
    SetSampleWeight(sample, m_stack[m_size].weight);
    sample.SetColor(Color4f::ZERO());

    return true; 
}

AdaptiveSuperSampler::AdaptiveSuperSampler() : PrimarySampler() {
    m_settings  = Settings();
    Init();
}

AdaptiveSuperSampler::AdaptiveSuperSampler(Settings& settings) : PrimarySampler() {
    m_settings	    = settings;
    Init();
}

AdaptiveSuperSampler::~AdaptiveSuperSampler() {
    SAFE_ARRAY_DELETE(m_stack);
}

void AdaptiveSuperSampler::Init() {	
    m_maxNodes  = (UINT32)pow(5.0, m_settings.maxLevels);
    m_stack     = new AdaptiveSampleNode[m_maxNodes];
    m_size	    = 0;
    m_color	    = Color4f::BLACK();

    m_firstSample = false;
    m_commonCase  = false;
}

}
