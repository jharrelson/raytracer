#include        <iostream>
#include        <iomanip>
#include        <string>
#include        <cmath>

using namespace std;

#include        "point.h"

/////////////////////////// friends ////////////////////////////////////////////
istream& operator>>(istream& s,point_t& rhs)
{
	int	i=0;
	char	c,n;
	double	f;

  do {
    s >> f; rhs[i] = f; i++;
  } while( s.get(c) && (c != '\n') && ((n = s.peek()) != '\n') );

  return(s);
}

ostream& operator<<(ostream& s,const point_t& rhs)
{
  s.setf(ios::fixed,ios::floatfield);
  s.precision(2);
  for(int i=0;i<rhs.dim();i++)
    i == rhs.dim()-1 ? s << rhs.point[i] : s << rhs.point[i] << ", ";
  return s;
}

point_t::point_t(vector<double> c)
{
  point = c;
}

point_t::point_t(const point_t& rhs)
{
  for(int i=0; i<(int)rhs.point.size(); i++)
    point.push_back(rhs.point[i]);
}

point_t::point_t(point_t* rhs)
{
  for(int i=0; i<(int)rhs->point.size(); i++)
    point.push_back(rhs->point[i]);
}

point_t point_t::operator=(const point_t& rhs)
{
  if(this != &rhs) {
    point.clear();
    for(int i=0; i<(int)rhs.point.size(); i++)
      point.push_back(rhs.point[i]);
  }
  return *this;
}

double point_t::distance(point_t& rhs)
{
	double	dist=0.0;

  if((*this).dim() != rhs.dim())
    std::cerr << "Warning: point dimensions unequal!" << std::endl;

  for(int i=0; i<(*this).dim(); ++i)
    dist += pow((*this)[i] - rhs[i],2.0);
  return(sqrt(dist));
}
