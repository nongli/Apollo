#include "ApolloPCA.h"

namespace Apollo {

PCA::PCA(const Matrix<DOUBLE>& data) :
            m_inputMatrix(&data),
            m_mean(1, data.NumCols()),
            m_eigenvalues(1, 1),
            m_basis(1, 1) {
    ComputeMean();
}

const Matrix<DOUBLE>& PCA::GetMean() const {
    return m_mean;
}
    
const Matrix<DOUBLE>& PCA::GetEigenvalues() const {
    return m_eigenvalues;
}

void PCA::ComputeMean() {
}

}
