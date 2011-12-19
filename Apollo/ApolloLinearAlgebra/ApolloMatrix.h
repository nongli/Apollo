#pragma once

#include "ApolloCommon.h"

namespace Apollo {
    template <typename T, UINT ROWS, UINT COLS>
    class FixedMatrix {
    public:
        FixedMatrix() { m_data.resize(ROWS * COLS); }
        FixedMatrix(const FixedMatrix<T, ROWS, COLS>& m);

        T* GetData() const { return &m_data[0]; }
        const T& GetData(UINT row, UINT col) const { return m_data[row * COLS + col]; }
        void SetData(UINT row, UINT col, const T& data) { m_data[row*COLS + col] = data; }

    private:
        std::vector<T> m_data;
    };

    template <typename T>
    class Matrix {
    public:
        Matrix(UINT rows, UINT cols) : m_rows(rows), m_cols(cols) { m_data.resize(rows * cols); }
        Matrix(const Matrix<T>& m);

        T* GetData() const {return &m_data[0]}
        const T& GetData(UINT row, UINT col) const { return m_data[row * m_cols + col]; }
        void SetData(UINT row, UINT col, const T& data) { m_data[row*m_cols + col] = data; }
        
        UINT NumCols() const { return m_cols; }
        UINT NumRows() const { return m_rows; }

        Matrix<T>& operator=(const Matrix<T>& rhs) {
            ApolloException::NotYetImplemented();
            return *this;
        }

    private:
        std::vector<T> m_data;
        const UINT m_rows;
        const UINT m_cols;
    };
}
