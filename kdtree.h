#ifndef KDTREE_H
#define KDTREE_H

// forward declcarations
template <typename T, typename P, typename C>
class kdtree_t;
template <typename T, typename P, typename C>
std::ostream& operator<<(std::ostream&, const kdtree_t<T,P,C>&);

template <typename T, typename P, typename C>
class kdtree_t {
  private:
  struct kdnode_t    // an all public class with data only, no member ftns
  {
	P         data;
	T         min,max;
	kdnode_t  *left;
	kdnode_t  *right;
	int       axis;

    kdnode_t(const P& d = P(), const T& in = T(), const T& ix = T(),
             kdnode_t *l = NULL, kdnode_t *r = NULL, int a = 0) : \
      data(d), min(in), max(ix), left(l), right(r), axis(a) \
      { };

friend std::ostream& operator<<(std::ostream& s, const kdnode_t& rhs)
      {
        s << rhs.left;
        s << rhs.data;
        s << std::endl;
        s << rhs.right;
        return s;
      }
friend std::ostream& operator<<(std::ostream& s, kdnode_t *rhs)
      {
        if(rhs) return(s << (*rhs));
        else    return s;
      }
  };

  public:
  // constructors (overloaded)
  kdtree_t() : root(NULL)                      { };

  // copy constructor
//kdtree_t(const kdtree_t& rhs);

  // destructors
  ~kdtree_t()                                  { clear(); }

        // friends -- note the extra <> telling the compiler to instantiate
        // a templated version of the operator<< -- <T> is also legal, i.e.,
        // friend std::ostream& operator<< <T>(std::ostream& s, const Tree&);
friend  std::ostream& operator<< <>(std::ostream& s, const kdtree_t& rhs);
friend  std::ostream& operator<<(std::ostream& s, kdtree_t *rhs)
    { return(s << (*rhs)); }

  // assignment operator
  const kdtree_t&        operator=(const kdtree_t&);

  // operators

  // members
  bool   empty() const	{ return root == NULL ? true : false;}
  void   insert(std::vector<P>& x,const T& min,const T& max)
                        { root = insert(root,x,min,max,0); }
  void   nn(T& q,P& p,double& r)
                        { r = INFINITY;  nn(root,q,p,r); }
  void   knn(T& q,std::vector<P>& p,double& r,int k)
                        { r = INFINITY;  knn(root,q,p,r,k); }
  void   range(const T& min, const T& max,std::vector<P>& p)
                        { range(root,min,max,p); }
  void   clear()	{ clear(root); }

  // private: only available to this class
  private:
  kdnode_t	*root;

  int		median(std::vector<P>&,int);
  kdnode_t*	insert(kdnode_t* &,std::vector<P>&,const T&,const T&,int);
  void		nn(kdnode_t* &,T&,P&,double&);
  void		knn(kdnode_t* &,T&,std::vector<P>&,double&,int);
  void		range(kdnode_t* &,const T&,const T&,std::vector<P>&);

  void		clear(kdnode_t* &);
  kdnode_t*	clone(kdnode_t* ) const;

};

#endif
