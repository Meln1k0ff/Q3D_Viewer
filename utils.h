#ifndef UTILS_H
#define UTILS_H

#include "vec.h"
#include <cctype>
#include <string>



class QString;

bool hit_sphere(const Vec3 &pos, const Vec3 &dir, const Vec3 &center, const float radius, Vec3 &hit_point);

void render_thumbnail(QString &filename, const QString &thumbname, const int w, const int h);

inline float pow10f(float n){pow(10,n);}

inline float parse_float(const char *buf)
{
    float v(0.f);
    int pos(0);
    int c = buf[pos++];
    const bool b_negative = c == '-';
    if (b_negative)
        c = buf[pos++];
    if (isdigit(c))
    {
        for(; isdigit(c) ; c = buf[pos++])
            v = v * 10.f + float(c - '0');
    }
    if (c == '.' || c == ',')
    {
        c = buf[pos++];
        for(float h = .1f; isdigit(c) ; c = buf[pos++], h *= .1f)
            v += float(c - '0') * h;
    }
    if (b_negative)
        v = -v;
    if (c == 'e' || c == 'E')
    {
        int e(0);
        c = buf[pos++];
        const bool b_e_negative = c == '-';
        if (b_e_negative)
            c = buf[pos++];
        else if (c == '+')
            ++pos;
        for(; isdigit(c) ; c = buf[pos++])
            e = e * 10 + (c - '0');
        if (b_e_negative)
            e = -e;
        v *= pow10f(float(e));
    }
    return v;
}

inline float parse_float(const std::string &str)
{
    return parse_float(str.c_str());
}

inline int parse_int(const char *buf)
{
    int v(0);
    int pos(0);
    int c = buf[pos++];
    const bool b_negative = c == '-';
    if (b_negative)
        c = buf[pos++];
    if (isdigit(c))
    {
        for(; isdigit(c) ; c = buf[pos++])
            v = v * 10 + int(c - '0');
    }
    if (b_negative)
        v = -v;
    return v;
}

inline int parse_int(const std::string &str)
{
    return parse_int(str.c_str());
}

inline unsigned int parse_uint(const char *buf)
{
    unsigned int v(0);
    int pos(0);
    int c = buf[pos++];
    if (isdigit(c))
    {
        for(; isdigit(c) ; c = buf[pos++])
            v = v * 10U + (unsigned int)(c - '0');
    }
    return v;
}

inline unsigned int parse_uint(const std::string &str)
{
    return parse_uint(str.c_str());
}

template<class T>
inline void split(const std::string &str, const char split_char, T &out, const bool keep_empty_elements = true)
{
    size_t prev = 0;
    for(size_t i = 0 ; i < str.size() ; ++i)
    {
        if (str[i] == split_char)
        {
            if (keep_empty_elements || prev != i)
                out.push_back(str.substr(prev, i - prev));
            prev = i + 1;
        }
    }
    if (keep_empty_elements || prev != str.size())
        out.push_back(str.substr(prev, str.size() - prev));
}

template<class T>
inline void split(char *str, const char split_char, T &out, const bool keep_empty_elements = true)
{
    char *prev = str;
    for(size_t i = 0 ; str[i] ; ++i)
    {
        if (str[i] == split_char)
        {
            str[i] = 0;
            if (keep_empty_elements || *prev)
                out.push_back(prev);
            prev = str + (i + 1);
        }
    }
    if (keep_empty_elements || *prev)
        out.push_back(prev);
}

inline int ilog10(int i)
{
    int n = 0;
    if (i >= 100000000)
    {
        n += 8;
        i /= 100000000;
    }
    if (i >= 10000)
    {
        n += 4;
        i /= 10000;
    }
    if (i >= 100)
    {
        n += 2;
        i /= 100;
    }
    if (i >= 10)
        ++n;
    return n;
}

inline int ilog10(unsigned int i)
{
    int n = 0;
    if (i >= 100000000U)
    {
        n += 8;
        i /= 100000000U;
    }
    if (i >= 10000U)
    {
        n += 4;
        i /= 10000U;
    }
    if (i >= 100U)
    {
        n += 2;
        i /= 100U;
    }
    if (i >= 10U)
        ++n;
    return n;
}

inline float ipow10f(const int e)
{
    static float exponent_table[] = {1e-40f, 1e-39f, 1e-38f, 1e-37f, 1e-36f, 1e-35f, 1e-34f, 1e-33f, 1e-32f, 1e-31f,
                                     1e-30f, 1e-29f, 1e-28f, 1e-27f, 1e-26f, 1e-25f, 1e-24f, 1e-23f, 1e-22f, 1e-21f,
                                     1e-20f, 1e-19f, 1e-18f, 1e-17f, 1e-16f, 1e-15f, 1e-14f, 1e-13f, 1e-12f, 1e-11f,
                                     1e-10f, 1e-9f, 1e-8f, 1e-7f, 1e-6f, 1e-5f, 1e-4f, 1e-3f, 1e-2f, 1e-1f,
                                     1.f, 1e1f, 1e2f, 1e3f, 1e4f, 1e5f, 1e6f, 1e7f, 1e8f, 1e9f,
                                     1e10f, 1e11f, 1e12f, 1e13f, 1e14f, 1e15f, 1e16f, 1e17f, 1e18f, 1e19f,
                                     1e20f, 1e21f, 1e22f, 1e23f, 1e24f, 1e25f, 1e26f, 1e27f, 1e28f, 1e29f,
                                     1e30f, 1e31f, 1e32f, 1e33f, 1e34f, 1e35f, 1e36f, 1e37f, 1e38f};
    return exponent_table[e + 40];
}

inline double ipow10d(const int e)
{
    static double exponent_table[] = {1e-40, 1e-39, 1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33, 1e-32, 1e-31,
                                      1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23, 1e-22, 1e-21,
                                      1e-20, 1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11,
                                      1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1,
                                      1., 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
                                      1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
                                      1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29,
                                      1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38};
    return exponent_table[e + 40];
}

inline int ilog10f(const float f)
{
    static float exponent_table[] = {1e-40f, 1e-39f, 1e-38f, 1e-37f, 1e-36f, 1e-35f, 1e-34f, 1e-33f, 1e-32f, 1e-31f,
                                     1e-30f, 1e-29f, 1e-28f, 1e-27f, 1e-26f, 1e-25f, 1e-24f, 1e-23f, 1e-22f, 1e-21f,
                                     1e-20f, 1e-19f, 1e-18f, 1e-17f, 1e-16f, 1e-15f, 1e-14f, 1e-13f, 1e-12f, 1e-11f,
                                     1e-10f, 1e-9f, 1e-8f, 1e-7f, 1e-6f, 1e-5f, 1e-4f, 1e-3f, 1e-2f, 1e-1f,
                                     1.f, 1e1f, 1e2f, 1e3f, 1e4f, 1e5f, 1e6f, 1e7f, 1e8f, 1e9f,
                                     1e10f, 1e11f, 1e12f, 1e13f, 1e14f, 1e15f, 1e16f, 1e17f, 1e18f, 1e19f,
                                     1e20f, 1e21f, 1e22f, 1e23f, 1e24f, 1e25f, 1e26f, 1e27f, 1e28f, 1e29f};
                                     //1e30f, 1e31f, 1e32f, 1e33f, 1e34f, 1e35f, 1e36f, 1e37f, 1e38f, 1e39f};
    int M = 79;
    int m = 0;
    while(m + 1 < M)
    {
        const int i = (m + M) >> 1;
        if (f < exponent_table[i])
            M = i;
        else
            m = i;
    }
    return m - 40;
}

inline int ilog10d(const double f)
{
    static double exponent_table[] = {1e-40, 1e-39, 1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33, 1e-32, 1e-31,
                                      1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23, 1e-22, 1e-21,
                                      1e-20, 1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11,
                                      1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1,
                                      1., 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
                                      1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
                                      1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29,
                                      1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38, 1e39};
    int M = 79;
    int m = 0;
    while(m + 1 < M)
    {
        const int i = (m + M) >> 1;
        if (f < exponent_table[i])
            M = i;
        else
            m = i;
    }
    return m - 40;
}




#endif // UTILS_H
