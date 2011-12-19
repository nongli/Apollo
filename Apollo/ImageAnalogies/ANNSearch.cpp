#include "ImageAnalogyImpl.h"
#include "ApolloCommon.h"
#include "ANN.h"

namespace Apollo
{
ANNSearch::ANNSearch(double* data, int nElements, int nDims, float error) 
{
	m_data = data;
	m_elements = nElements;
	m_dimensions = nDims;
	m_error = error;
	init();
}

ANNSearch::~ANNSearch(void) 
{
    ANNpointArray data = m_tree->thePoints();
    annDeallocPts(data);
//	delete m_tree;
}

void ANNSearch::init() 
{
	ANNpointArray data = annAllocPts(m_elements, m_dimensions);
	for (int i = 0; i < m_elements; i++) {
		memcpy(data[i], &m_data[i*m_dimensions], sizeof(double)*m_dimensions);
	}
	m_tree = new ANNkd_tree(data, m_elements, m_dimensions);
}

int ANNSearch::findANN(double* target, double& dist) {
	ANNidx index;
	m_tree->annkSearch(target, 1, &index, &dist, m_error);
	dist = sqrt(dist);
	return (int)index;
}

int ANNSearch::findNN(double* target) {
	ANNidx index;
	double dist;
	m_tree->annkSearch(target, 1, &index, &dist, 0);
	return (int)index;
}
}
