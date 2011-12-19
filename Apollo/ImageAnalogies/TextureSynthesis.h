#pragma once
/**
 * Texture synthesis using ashikhmin's algorithm 
 */
#include "ApolloCommon.h"

namespace Apollo
{
    class Image;

    struct TextureSynthesisParams 
    {
	    int regionWidth;
	    int regionHeight;
    };

    class TextureSynthesis 
    {
    public:
	    static Image* SynthesizeTexture(const Image* source, Image* dest, const TextureSynthesisParams& params);
	    ~TextureSynthesis(void);

    private:
	    struct Index 
        {
		    int row;
		    int col;
	    };        

	    TextureSynthesis();
	    void createTexture();
	    void init();
	    int generateCandidates(int row, int col);
	    int generateAllCandidates(int row, int col);
	    FLOAT neighborDiff(const Index& index, int row, int col);
	    FLOAT neighborAllDiff(const Index& index, int row, int col);

	    TextureSynthesisParams	m_params;
	    const Image*			m_source;
	    Image*					m_result;
	    std::vector<Index>		m_candidates;
	    std::vector<Index>		m_atlas;

	    int						m_vrstartx, m_vrstarty;
	    int						m_vrfinishx, m_vrfinishy;
    };
};
