/*
 * Poly2Tri Copyright (c) 2009-2010, Poly2Tri Contributors
 * http://code.google.com/p/poly2tri/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of Poly2Tri nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SHAPES_H
#define SHAPES_H

#include <vector>
#include <cstddef>
#include <assert.h>
#include <cmath>

namespace p2t {

struct Edge;

struct Point {

  double x, y;

  Point()
  {
    x = 0.0;
    y = 0.0;
  }

  std::vector<Edge*> edge_list;

  Point(double x, double y) : x(x), y(y) {}

  void set_zero()
  {
    x = 0.0;
    y = 0.0;
  }

  void set(double x_, double y_)
  {
    x = x_;
    y = y_;
  }

  Point operator -() const
  {
    Point v;
    v.set(-x, -y);
    return v;
  }

  void operator +=(const Point& v)
  {
    x += v.x;
    y += v.y;
  }

  void operator -=(const Point& v)
  {
    x -= v.x;
    y -= v.y;
  }

  void operator *=(double a)
  {
    x *= a;
    y *= a;
  }

  double Length() const
  {
    return sqrt(x * x + y * y);
  }

  double Normalize()
  {
    const double len = Length();
    x /= len;
    y /= len;
    return len;
  }

};

struct Edge {

  Point* p, *q;

  Edge(Point& p1, Point& p2) : p(&p1), q(&p2)
  {
    if (p1.y > p2.y) {
      q = &p1;
      p = &p2;
    } else if (p1.y == p2.y) {
      if (p1.x > p2.x) {
        q = &p1;
        p = &p2;
      } else if (p1.x == p2.x) {
        assert(false);
      }
    }

    q->edge_list.push_back(this);
  }
};

class Triangle {
public:

Triangle(Point& a, Point& b, Point& c);

bool constrained_edge[3];
bool delaunay_edge[3];

Point* GetPoint(int index);
Point* PointCW(const Point& point);
Point* PointCCW(const Point& point);
Point* OppositePoint(Triangle& t, const Point& p);

Triangle* GetNeighbor(int index);
void MarkNeighbor(Point* p1, Point* p2, Triangle* t);
void MarkNeighbor(Triangle& t);

void MarkConstrainedEdge(int index);
void MarkConstrainedEdge(Edge& edge);
void MarkConstrainedEdge(Point* p, Point* q);

int Index(const Point* p);
int EdgeIndex(const Point* p1, const Point* p2);

Triangle* NeighborCW(const Point& point);
Triangle* NeighborCCW(const Point& point);
bool GetConstrainedEdgeCCW(const Point& p);
bool GetConstrainedEdgeCW(const Point& p);
void SetConstrainedEdgeCCW(const Point& p, bool ce);
void SetConstrainedEdgeCW(const Point& p, bool ce);
bool GetDelunayEdgeCCW(const Point& p);
bool GetDelunayEdgeCW(const Point& p);
void SetDelunayEdgeCCW(const Point& p, bool e);
void SetDelunayEdgeCW(const Point& p, bool e);

bool Contains(const Point* p);
bool Contains(const Edge& e);
bool Contains(const Point* p, const Point* q);
void Legalize(Point& point);
void Legalize(Point& opoint, Point& npoint);
/**
 * Clears all references to all other triangles and points
 */
void Clear();
void ClearNeighbor(const Triangle *triangle);
void ClearNeighbors();
void ClearDelunayEdges();

inline bool IsInterior();
inline void IsInterior(bool b);

Triangle& NeighborAcross(const Point& opoint);

void DebugPrint();

private:

Point* points_[3];
Triangle* neighbors_[3];

bool interior_;
};

inline bool cmp(const Point* a, const Point* b)
{
  if (a->y < b->y) {
    return true;
  } else if (a->y == b->y) {
    if (a->x < b->x) {
      return true;
    }
  }
  return false;
}

inline Point operator +(const Point& a, const Point& b)
{
  return Point(a.x + b.x, a.y + b.y);
}

inline Point operator -(const Point& a, const Point& b)
{
  return Point(a.x - b.x, a.y - b.y);
}

inline Point operator *(double s, const Point& a)
{
  return Point(s * a.x, s * a.y);
}

inline bool operator ==(const Point& a, const Point& b)
{
  return a.x == b.x && a.y == b.y;
}

inline bool operator !=(const Point& a, const Point& b)
{
  return !(a.x == b.x) && !(a.y == b.y);
}

inline double Dot(const Point& a, const Point& b)
{
  return a.x * b.x + a.y * b.y;
}

inline double Cross(const Point& a, const Point& b)
{
  return a.x * b.y - a.y * b.x;
}

inline Point Cross(const Point& a, double s)
{
  return Point(s * a.y, -s * a.x);
}

inline Point Cross(double s, const Point& a)
{
  return Point(-s * a.y, s * a.x);
}

inline Point* Triangle::GetPoint(int index)
{
  return points_[index];
}

inline Triangle* Triangle::GetNeighbor(int index)
{
  return neighbors_[index];
}

inline bool Triangle::Contains(const Point* p)
{
  return p == points_[0] || p == points_[1] || p == points_[2];
}

inline bool Triangle::Contains(const Edge& e)
{
  return Contains(e.p) && Contains(e.q);
}

inline bool Triangle::Contains(const Point* p, const Point* q)
{
  return Contains(p) && Contains(q);
}

inline bool Triangle::IsInterior()
{
  return interior_;
}

inline void Triangle::IsInterior(bool b)
{
  interior_ = b;
}

}

#endif