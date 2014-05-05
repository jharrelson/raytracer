#include <iostream>
#include <cmath>

#include "vector.h"

// compute the dot product of two vectors
double vec_t::dot(const vec_t& rhs) const
{
	double	s=0.0;

  for(int i=0;i<3;i++) s += vec[i] * rhs[i];

  return(s);
}

// compute the length of the vector v1
double vec_t::len() const
{
  return(sqrt(dot(*this)));
}

// compute the dot product of two vectors
vec_t vec_t::norm() const
{
	vec_t	result(*this);
	double	length = len();

  for(int i=0;i<3;i++) result[i] /= length;

  return(result);
}

// compute the reflection vector: v = 2 (u dot n) n - u
vec_t vec_t::reflect(const vec_t& n) const
{
	vec_t	u(-(*this));	// u = -v
	vec_t	result;

  // scale n by 2 (u dot n)
  result = u - 2.0 * u.dot(n) * n;

  // result -= u;
  //result = result - u;

  return result;
}

// compute refraction ray
vec_t vec_t::refract(const vec_t& n, double n_t) const {
    vec_t u(*this);
    double n1 = 1.000293; // air
    double cos1 = u.dot(n);
    double d = 1.0 - ((n1/n_t)*(n1/n_t)) * (1.0 - (cos1*cos1));
    vec_t result;

    if (d > 0) {
        result = (n1/n_t) * (u - (cos1 * n)) - (sqrt(d) * n);
    } else {
        result = u.reflect(n);
    }

    return result;
}
