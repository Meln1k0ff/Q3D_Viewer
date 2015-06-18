
#ifndef MATRIX_H
#define MATRIX_H

#include "vec.h"
#include <algorithm>
#include <ostream>
#include <iomanip>
#include <limits>

template<typename T, int N, int M>
struct Matrix
{
public:
    typedef T	component_type;
    enum { components = N * M, rows = N, cols = M };

public:
    inline Matrix()	{}
    inline Matrix(const Matrix &m)
    {
        for(size_t i = 0 ; i < N ; ++i)
            for(size_t j = 0 ; j < M ; ++j)
                _v[i][j] = m._v[i][j];
    }

    inline Matrix &operator=(const Matrix &m)
    {
        for(size_t i = 0 ; i < N ; ++i)
            for(size_t j = 0 ; j < M ; ++j)
                _v[i][j] = m._v[i][j];
        return *this;
    }

    inline const T &operator()(size_t i, size_t j) const	{	return _v[i][j];	}
    inline T &operator()(size_t i, size_t j)	{	return _v[i][j];	}

    inline void transpose()
    {
        for(size_t i = 0 ; i < N ; ++i)
            for(size_t j = i + 1 ; j < N ; ++j)
                std::swap(_v[i][j], _v[j][i]);
    }

    inline Matrix transposed() const
    {
        Matrix m(*this);
        m.transpose();
        return m;
    }

    static Matrix identity()
    {
        Matrix m;
        for(size_t i = 0 ; i < N ; ++i)
            for(size_t j = 0 ; j < M ; ++j)
                m._v[i][j] = T(0);

        for(size_t i = 0 ; i < std::min(N, M) ; ++i)
            m._v[i][i] = T(1);
        return m;
    }

    static Matrix scale(const Vec<T, N - 1> &v)
    {
        Matrix m;
        for(size_t i = 0 ; i < N ; ++i)
            for(size_t j = 0 ; j < M ; ++j)
                m._v[i][j] = T(0);

        for(size_t i = 0 ; i < N - 1 ; ++i)
            m._v[i][i] = v[i];
        m._v[N - 1][N - 1] = T(1);
        return m;
    }

    static Matrix translation(const Vec<T, N - 1> &v)
    {
        Matrix m = Matrix::identity();
        for(size_t i = 0 ; i < N - 1 ; ++i)
            m._v[i][M - 1] = v[i];
        return m;
    }

    static Matrix rotation(const T &angle, Vec<T, 3> axis)
    {
        Matrix r;
        if (std::fabs(angle) <= std::numeric_limits<T>::epsilon())
        {
            r = identity();
            return r;
        }

        T n = axis.length();
        if (n <= std::numeric_limits<T>::epsilon())
        {
            r = identity();
            return r;
        }
        n = T(1) / n;
        if (isnan(n) || isinf(n))
        {
            r = identity();
            return r;
        }
        axis /= n;
        const T c = std::cos(angle);
        const T omc = T(1) - c;
        const T s = std::sin(angle);

        r(0, 0) = axis[0] * axis[0] * omc + c;
        r(1, 0) = axis[1] * axis[0] * omc + s * axis[2];
        r(2, 0) = axis[2] * axis[0] * omc - s * axis[1];
        r(3, 0) = T(0);

        r(0, 1) = axis[0] * axis[1] * omc - s * axis[2];
        r(1, 1) = axis[1] * axis[1] * omc + c;
        r(2, 1) = axis[2] * axis[1] * omc + s * axis[0];
        r(3, 1) = T(0);

        r(0, 2) = axis[0] * axis[2] * omc + s * axis[1];
        r(1, 2) = axis[1] * axis[2] * omc - s * axis[0];
        r(2, 2) = axis[2] * axis[2] * omc + c;
        r(3, 2) = T(0);

        r(0, 3) = T(0);
        r(1, 3) = T(0);
        r(2, 3) = T(0);
        r(3, 3) = T(1);

        return r;
    }

    template<int K>
    inline Matrix<T, N, K> operator*(const Matrix<T, M, K> &rhs) const
    {
        Matrix<T, N, K> r;
        for(size_t i = 0 ; i < N ; ++i)
        {
            for(size_t j = 0 ; j < K ; ++j)
            {
                T acc(0);
                for(size_t k = 0 ; k < M ; ++k)
                    acc += _v[i][k] * rhs(k, j);
                r(i, j) = acc;
            }
        }
        return r;
    }

    inline Matrix operator*(const T &rhs) const
    {
        Matrix r;
        for(size_t i = 0 ; i < N ; ++i)
            for(size_t j = 0 ; j < M ; ++j)
                r._v[i][j] = _v[i][j] * rhs;
        return r;
    }

    T *data()	{	return &_v[0][0];	}
    const T *data() const	{	return &_v[0][0];	}

    inline T det() const;

    Matrix comatrix() const
    {
        Matrix com;
        for(size_t i = 0 ; i < N ; ++i)
        {
            for(size_t j = 0 ; j < M ; ++j)
            {
                const Matrix<T, N - 1, N - 1> &s = sub(i, j);
                const T d = s.det();
                com(i, j) = ((i + j) & 1) ? -d : d;
            }
        }
        return com;
    }

    Matrix inverse() const
    {
        return comatrix().transposed() * (T(1) / det());
    }

    Matrix<T, N - 1, M - 1> sub(size_t si, size_t sj) const
    {
        Matrix<T, N - 1, M - 1> r;
        for(size_t i = 0 ; i < si ; ++i)
            for(size_t j = 0 ; j < sj ; ++j)
                r(i,j) = _v[i][j];
        for(size_t i = si + 1 ; i < N ; ++i)
            for(size_t j = 0 ; j < sj ; ++j)
                r(i - 1,j) = _v[i][j];
        for(size_t i = 0 ; i < si ; ++i)
            for(size_t j = sj + 1 ; j < M ; ++j)
                r(i,j -1) = _v[i][j];
        for(size_t i = si + 1 ; i < N ; ++i)
            for(size_t j = sj + 1 ; j < M ; ++j)
                r(i - 1,j - 1) = _v[i][j];
        return r;
    }

private:
    T _v[N][M];
};

template<typename T, int N, int M>
inline T Matrix<T, N, M>::det() const
{
    T acc(0);
    for(size_t i = 0 ; i < N ; ++i)
    {
        const Matrix<T, N - 1, N - 1> &s = sub(i, 0);
        const T d = s.det();
        if (i & 1)
            acc -= _v[i][0] * d;
        else
            acc += _v[i][0] * d;
    }
    return acc;
}

template<>
inline double Matrix<double, 1, 1>::det() const
{
    return _v[0][0];
}

template<>
inline float Matrix<float, 1, 1>::det() const
{
    return _v[0][0];
}

template<typename T, int N, int M>
inline Vec<T, N> operator*(const Matrix<T, N, M> &lhs, const Vec<T, M> &rhs)
{
    Vec<T, N> r;
    for(size_t i = 0 ; i < N ; ++i)
    {
        T acc(0);
        for(size_t j = 0 ; j < M ; ++j)
            acc += rhs[j] * lhs(i, j);
        r[i] = acc;
    }
    return r;
}

template<typename T, int N>
inline Vec<T, N - 1> operator*(const Matrix<T, N, N> &lhs, const Vec<T, N - 1> &rhs)
{
    Vec<T, N - 1> r;
    for(size_t i = 0 ; i < N - 1 ; ++i)
    {
        T acc(0);
        for(size_t j = 0 ; j < N - 1 ; ++j)
            acc += rhs[j] * lhs(i, j);
        acc += lhs(i, N - 1);
        r[i] = acc;
    }
    T w(0);
    for(size_t i = 0 ; i < N - 1 ; ++i)
        w += rhs[i] * lhs(N - 1, i);
    w += lhs(N - 1, N - 1);
    return r / w;
}

template<typename T, int N, int M>
std::ostream &operator<<(std::ostream &out, const Matrix<T, N, M> &rhs)
{
    for(size_t i = 0 ; i < N ; ++i)
    {
        if (i == 0)
        {
            if (N > 1)
                out << '/';
            else
                out << '[';
        }
        else if (i + 1 < N)
            out << '|';
        else
            out << '\\';

        out << ' ';
        for(size_t j = 0 ; j < M ; ++j)
            out << std::setfill(' ') << std::setw(8) << rhs(i, j) << std::setw(0) << ' ';

        if (i == 0)
        {
            if (N > 1)
                out << '\\';
            else
                out << ']';
        }
        else if (i + 1 < N)
            out << '|';
        else
            out << '/';
        out << std::endl;
    }
    return out;
}

typedef Matrix<float, 3, 3>		Mat33;
typedef Matrix<float, 3, 3>		Mat33f;
typedef Matrix<double, 3, 3>	Mat33d;
typedef Matrix<float, 4, 4>		Mat44;
typedef Matrix<float, 4, 4>		Mat44f;
typedef Matrix<double, 4, 4>	Mat44d;

#endif // MATRIX_H
