#ifndef RAY_H
#define RAY_H

#define MAX_DIST 100
template <typename T, typename P, typename C> class kdtree_t;
class photon_t;
class photon_c;

class ray_t
{
  public:
  // constructors (overloaded)
  ray_t() : \
	dis(0.0), \
	pos(0.0,0.0,0.0), \
	dir(0.0,0.0,0.0), \
	hit(0.0,0.0,0.0), \
	N(0.0,0.0,0.0), \
        obj(NULL) \
	{ };

  ray_t(const vec_t& o, const vec_t& d, double r=0.0) : \
	dis(r), \
	pos(o), \
	dir(d), \
	hit(0.0,0.0,0.0), \
	N(0.0,0.0,0.0), \
        obj(NULL) \
	{ };

  // copy constructor
  ray_t(const ray_t& rhs) : \
	dis(rhs.dis), \
	pos(rhs.pos), \
	dir(rhs.dir), \
	hit(rhs.hit), \
	N(rhs.N), \
        obj(rhs.obj) \
	{ };

  // operators (incl. assignment operator)
  const ray_t& operator=(const ray_t& rhs)
	{
	  if(this != &rhs) {
	    dis = rhs.dis;
	    pos = rhs.pos;
	    dir = rhs.dir;
	    hit = rhs.hit;
	    N = rhs.N;
            obj = rhs.obj;
	  }
          return *this;
	}

  // methods
//  void trace(model_t&,rgb_t<double>&, int);
  void trace(model_t&, rgb_t<double>&, int, kdtree_t<photon_t, photon_t*, photon_c>&);

  vec_t getdir() { return dir; }

  // destructors (default ok, no 'new' in constructor)
  ~ray_t()
	{ };

  protected:
  double   dis;	// distance
  vec_t    pos;	// position
  vec_t    dir;	// direction
  vec_t    hit;	// hit point
  vec_t    N;	// normal at hit point
  object_t *obj;// list object hit
};

#endif
