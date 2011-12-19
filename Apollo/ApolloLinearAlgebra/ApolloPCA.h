#pragma once
#include "ApolloCommon.h"
#include "ApolloMatrix.h"

namespace Apollo {
    class PCA {
    public:
        PCA(const Matrix<DOUBLE>& data);  
        const Matrix<DOUBLE>& GetMean() const;
        const Matrix<DOUBLE>& GetEigenvalues() const;     
        const Matrix<DOUBLE>& GetBasisVector(UINT index) const;

    private:     
        void ComputeMean();

        const Matrix<DOUBLE>* m_inputMatrix;
        Matrix<DOUBLE> m_eigenvalues;
        Matrix<DOUBLE> m_mean;
        Matrix<DOUBLE> m_basis;
    };
}
