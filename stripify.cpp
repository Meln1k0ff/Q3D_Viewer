#include "stripify.h"
#include <unordered_set>
#include <QMultiHash>
#include <QDebug>
#include <QTime>

using namespace std;

struct Triangle
{
    quint32 a, b, c;

    Triangle(const Triangle &t) : a(t.a), b(t.b), c(t.c)	{}
    Triangle(quint32 a, quint32 b, quint32 c) : a(a), b(b), c(c)
    {
        sort();
    }

    inline void sort()
    {
        if (a > b)	qSwap(a, b);
        if (b > c)	qSwap(c, b);
        if (a > b)	qSwap(a, b);
    }

    inline bool operator==(const Triangle &v) const
    {
        return a == v.a && b == v.b && c == v.c;
    }

    inline bool operator<(const Triangle &v) const
    {
        return a < v.a
                || (a == v.a && b < v.b)
                || (a == v.a && b == v.b && c < v.c);
    }
};

inline quint32 qHash(const Triangle &v)
{
    return (qHash(v.a) * 31 + qHash(v.b)) * 31 + qHash(v.c);
}

struct Edge
{
    quint32 a, b;

    Edge(const Edge &e) : a(e.a), b(e.b)	{}
    Edge(quint32 a, quint32 b) : a(a), b(b)
    {
        sort();
    }

    inline void sort()
    {
        if (a > b)	qSwap(a, b);
    }

    inline bool operator==(const Edge &v) const
    {
        return a == v.a && b == v.b;
    }

    inline bool equal(quint32 va, quint32 vb) const
    {
        return a == va && b == vb;
    }

    inline bool operator<(const Edge &v) const
    {
        return a < v.a || (a == v.a && b < v.b);
    }
};

inline quint32 qHash(const Edge &v)
{
    return v.a * 31U + v.b;
}

namespace std
{
    template<>
    class hash<Triangle>
    {
    public:
        inline unsigned int operator()(const Triangle &v) const
        {
            return (v.a * 31 ^ v.b) * 31 ^ v.c;
        }
    };
    template<>
    class hash<Edge>
    {
    public:
        inline unsigned int operator()(const Edge &v) const
        {
            return v.a * 31 ^ v.b;
        }
    };
}

Stripify::Stripify()
{
}

void Stripify::process(std::vector<quint32> &indices)
{
    unordered_set<Triangle> triangles;
    typedef QMultiHash<Edge, Triangle> EdgeMap;
    EdgeMap edges;
    triangles.reserve(indices.size() / 3);
    edges.reserve(indices.size());
    for(vector<quint32>::const_iterator i = indices.begin() ; i != indices.end() ; ++i)
    {
        const quint32 a = *i;	++i;
        const quint32 b = *i;	++i;
        const quint32 c = *i;
        const Triangle t(a,b,c);
        triangles.insert(t);

        edges.insertMulti(Edge(a,b), t);
        edges.insertMulti(Edge(b,c), t);
        edges.insertMulti(Edge(c,a), t);
    }

    qDebug() << "data structures filled";
    QTime timer;
    timer.start();

    indices.clear();
    vector<quint32> stripE;
    vector<quint32> stripS;
    while(!triangles.empty())
    {
        stripE.clear();
        stripS.clear();
        Triangle t = *(triangles.begin());
        triangles.erase(triangles.begin());
        edges.remove(Edge(t.a, t.b), t);
        edges.remove(Edge(t.b, t.c), t);
        edges.remove(Edge(t.c, t.a), t);
        stripE.push_back(t.a);
        stripE.push_back(t.b);
        stripE.push_back(t.c);

        Edge e(t.b, t.c);
        quint32 last = t.c;
        if (!edges.count(e))
        {
            if (edges.count(Edge(t.a, t.b)))
            {
                e = Edge(t.a, t.b);
                stripE[0] = t.c;
                stripE[1] = t.a;
                stripE[2] = t.b;
                last = t.b;
            }
            else if (edges.count(Edge(t.c, t.a)))
            {
                stripE[0] = t.b;
                stripE[1] = t.c;
                stripE[2] = t.a;
                last = t.a;
            }
        }
        EdgeMap::iterator e_it;
        while((e_it = edges.find(e)) != edges.end())
        {
            t = *e_it;
            edges.remove(Edge(t.a, t.b), t);
            edges.remove(Edge(t.b, t.c), t);
            edges.remove(Edge(t.c, t.a), t);
            triangles.erase(t);
            if (e.equal(t.a, t.b))
            {
                e = Edge(last, t.c);
                last = t.c;
            }
            else if (e.equal(t.b, t.c))
            {
                e = Edge(last, t.a);
                last = t.a;
            }
            else if (e.equal(t.a, t.c))
            {
                e = Edge(last, t.b);
                last = t.b;
            }
            stripE.push_back(last);
        }

        e = Edge(stripE[0], stripE[1]);
        last = stripE[0];
        t = Triangle(stripE[0], stripE[1], stripE[2]);
        while((e_it = edges.find(e)) != edges.end())
        {
            t = *e_it;
            edges.remove(Edge(t.a, t.b), t);
            edges.remove(Edge(t.b, t.c), t);
            edges.remove(Edge(t.c, t.a), t);
            triangles.erase(t);
            if (e.equal(t.a, t.b))
            {
                e = Edge(last, t.c);
                last = t.c;
            }
            else if (e.equal(t.b, t.c))
            {
                e = Edge(last, t.a);
                last = t.a;
            }
            else if (e.equal(t.a, t.c))
            {
                e = Edge(last, t.b);
                last = t.b;
            }
            stripS.push_back(last);
        }

        if (!indices.empty())
            indices.push_back(0xFFFFFFFFU);
        indices.insert(indices.end(), stripS.rbegin(), stripS.rend());
        indices.insert(indices.end(), stripE.begin(), stripE.end());
    }

    qDebug() << "done in " << timer.elapsed() << " ms";
}
