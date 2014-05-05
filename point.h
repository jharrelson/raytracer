#ifndef POINT_H
#define POINT_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <list>
#include <cstdlib>
#include <cmath>

using namespace std;

class point_t
{
	public:
	// constructors (overloaded)
	point_t(double x=0.0, double y=0.0) \
					{	point.push_back(x);
						point.push_back(y);
					}

	point_t(const point_t& rhs);
	point_t(point_t* rhs);
	point_t(vector<double> c);

	// destructors

	// operators
	const double&	operator[](int k) const	{ return(point[k]); }
	double&		operator[](int k)	{ return(point[k]); }
	point_t		operator=(const point_t& rhs);

	// friends
        friend istream&	operator>>(istream& s, point_t& rhs);
        friend istream&	operator>>(istream& s, point_t* rhs)
          { return(s >> (*rhs)); }
        friend ostream&	operator<<(ostream& s, const point_t& rhs);
        friend ostream&	operator<<(ostream& s, point_t* rhs)
          { return(s << (*rhs)); }

	// member functions
	double		distance(point_t& rhs);
	double		distance(point_t* rhs)	{ return distance(*rhs); }
	int		dim() const		{ return point.size(); }

	private:
	vector<double>	point;

};

class point_c
{
  public:
  point_c(int inaxis=0) : axis(inaxis) {};
  bool operator()(const point_t& p1, const point_t& p2) const
    { return(p1[axis] < p2[axis]); }
  bool operator()(const point_t * const & p1, const point_t * const & p2) const
    { return((*p1)[axis] < (*p2)[axis]); }

  private:
  int axis;
};

#endif
