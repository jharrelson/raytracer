#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>

#include "vector.h"
#include "pixel.h"
#include "camera.h"
#include "light.h"
#include "material.h"
#include "object.h"
#include "list.h"
#include "plane.h"
#include "model.h"
#include "ray.h"
#include "photon.h"
#include "point.h"
#include "kdtree.h"

template <typename T, typename P, typename C>
std::ostream& operator<<(std::ostream& s, const kdtree_t<T,P,C>& rhs)
{
  s.setf(ios::fixed,ios::floatfield);
  s.precision(2);
  if(rhs.empty()) s << "empty" << std::endl;
  else s << rhs.root;
  return s;
}

template <typename T, typename P, typename C>
const kdtree_t<T,P,C>& kdtree_t<T,P,C>::operator=(const kdtree_t<T,P,C>& rhs)
{
  if(this != &rhs) {       // standard alias test
    clear();
    root = clone(rhs.root);
  }
  return *this;
}

template <typename T, typename P, typename C>
typename kdtree_t<T,P,C>::kdnode_t* kdtree_t<T,P,C>::clone(kdnode_t* t) const
{
  if(t == NULL) return NULL;

  return new kdnode_t(t->data,t->min,t->max,
                clone(t->left),clone(t->right),t->axis);
}

template <typename T, typename P, typename C>
void kdtree_t<T,P,C>::clear(kdnode_t* &t)
{
  if(t != NULL) {
    clear(t->left);
    clear(t->right);
    delete t;
  }
  t = NULL;
}

template <typename T, typename P, typename C>
int
kdtree_t<T,P,C>::median(std::vector<P>& x,int axis)
{
        int     left=0,right=x.size()-1;
        int     i,j,k=left+(right-left)/2;
        P       key,tmp;
        C       comparator(axis);

  // find kth element in linear time (see Wirth, p.84);
  // if k=(left+right)/2 then find the median
  while(left < right) {
    key = x[k];
    i = left; j = right;
    do {
     while(comparator(x[i],key)) i++;
     while(comparator(key,x[j])) j--;
     if(i <= j) {
       // swap large i with small j
       tmp = x[i];
       x[i] = x[j];
       x[j] = tmp;
       i++;
       j--;
     }
   } while(i <= j);
   if(j < k) left = i;
   if(k < i) right = j;
  }
  return(k);
}

template <typename T, typename P, typename C>
typename kdtree_t<T,P,C>::kdnode_t*
kdtree_t<T,P,C>::insert(kdnode_t* &t,std::vector<P>& x,const T& min,const T& max,int d)
{
                                    // select axis based on depth so that
                                    // axis cycles through all valid values
           int                      axis = x.empty() ? 0 : d % x[0]->dim();
           int                      m=0; // median index
           P                        med;
           T                        _min, _max;
           C                        comparator(axis);
           std::vector<P>           left, right;
  typename std::vector<P>::iterator it;

  if(x.empty()) return NULL;

  // debugging
  std::cerr << "depth: " << d << std::endl;
  std::cerr << "size: " << x.size() << std::endl;

  // sort vector and choose median as pivot element
//sort(x.begin(),x.end(),C(axis));
  // create node and construct subtrees
//m = x.size()/2;

  m = median(x,axis);

  // debugging
  std::cerr << "median = x[" << m << "] = " << x[m] << std::endl;
  for(it = x.begin(); it<x.end(); it++) std::cerr << *it << std::endl;

  // create left and right sublists
//for(int i=0; i<(int)x.size(); i++)
//  if(i < m)       left.push_back(x[i]);
//  else if(i > m) right.push_back(x[i]);
//  else           med = x[m];
  med = x[m];
  for(int i=0; i<(int)x.size(); i++)
    if(i==m) continue;
    else
      if(comparator(x[i],x[m]))       left.push_back(x[i]);
      else                            right.push_back(x[i]);

  // create new node
  kdnode_t* node = new kdnode_t(med,min,max,NULL,NULL,axis);

  // recursively add left subtree
  _min = min;
  _max = max;
  _max[axis] = (*med)[axis];
  node->left = insert(node,left,_min,_max,d+1);

  // recursively add right subtree
  _min = min;
  _max = max;
  _min[axis] = (*med)[axis];
  node->right = insert(node,right,_min,_max,d+1);

  return node;
}

template <typename T, typename P, typename C>
void kdtree_t<T,P,C>::nn(kdnode_t* &t,T& q,P& p,double& r)
{
        int     axis;
        double  dist;

  if(!t) return;

  // compute node's distance to query point
  dist = q.distance(t->data);

  // as we descend the tree to find the closest leaf (the first
  // approximation is initially found at the leaf node which contains
  // the target point), but along the way test against each node that we
  // touch -- it may be that one of the nodes on the way down is closer
  if(dist < r) {
    r = dist;
    p = t->data;
  }

  // traverse down the "closer" side of the tree -- note that
  // these recursive calls may (will) override the p,r arguments
  // if a closer node than the current node is found
  //
  // the first approximation (e.g., leaf node) is not necessarily the
  // closest; but from this descent-only search we know that any potential
  // nearer neighbor must lie closer and so must lie within the circle
  // defined by center q and radius r
  //
  // as we return we need to check to see whether the current closest
  // circle (defined by center q and radius r) intersects the "farther"
  // side of the tree -- if it does, we must search that subtree
  axis = t->axis;
  if(q[axis] <= (*t->data)[axis]) {
    // check the circle (q,r) intersection with t->left (extraneous if)
  //if(q[axis] - r <= (*t->data)[axis]) nn(t->left,q,p,r);
    nn(t->left,q,p,r);
    // check the circle (q,r) intersection with t->right
    if(q[axis] + r > (*t->data)[axis]) nn(t->right,q,p,r);
  } else {
    // check the circle (q,r) intersection with t->right (extraneous if)
  //if(q[axis] + r > (*t->data)[axis]) nn(t->right,q,p,r);
    nn(t->right,q,p,r);
    // check the circle (q,r) intersection with t->left
    if(q[axis] - r <= (*t->data)[axis]) nn(t->left,q,p,r);
  }
}

template <typename T, typename P, typename C>
void kdtree_t<T,P,C>::knn(kdnode_t* &t,T& q,std::vector<P>& p,double& r,int k)
{
        int                                     axis;
        double                                  dist;
        typename std::vector<P>::iterator       pit;

  if(!t) return;

  // compute node's distance to query point
  dist = q.distance(t->data);

  // as we descend the tree to find the closest leaf (the first approximation
  // is initially found at the leaf node which contains the target point),
  // but along the way test against each node that we touch -- it may be that
  // one of the nodes on the way down is closer
  //    
  // for k-nearest neighbors, maintain a sorted list s.t. the point
  // that is furthest away can be deleted if the list contains k points
  // and a new closer point is found -- to do so, use a list that is
  // sorted as points are added (e.g., via insertion sort basically),
  // this way if we add a closer point than what is on the list, we can
  // safely remove the last point on the list
  //
  // for the actual search, instead of searching within a radius that is
  // the closest distance yet found, search within the area (volume) whose
  // radius is the k-th closest yet found--until k points have been found,
  // this distance is infinity
  if((int)p.size() < k) {
    // if we don't yet have the requested number of points, insert current
    // node's distance into (sorted) knn list without bothering to check
    // distance:
    // if list is empty or distance is larger than last node
    // (and there are fewer of them than required k)
    //   just add the node to the back
    if( p.empty() || (dist > q.distance(p.back())) )
      p.push_back(t->data);
    // otherwise
    //   iterate through the list to find proper place to insert --
    //   it won't be at the back since we already checked that
    else { 
      for(pit = p.begin(); pit != p.end(); pit++) {
        if(dist < q.distance(*pit)) {
          p.insert(pit,1,t->data);
          break;
        }
      }
    }
  } else {
    // insert current node into (sorted) knn list, checking its distance
    // against those points that are already on the list; do this only if
    // the distance is smaller than the last point on the list, otherwise
    // safe to ignore this point
    if(dist < q.distance(p.back())) {
      for(pit = p.begin(); pit != p.end(); pit++) {
        if(dist < q.distance(*pit)) {
          p.insert(pit,1,t->data);
          break;
        }
      }
      // since we know we have k nodes already and we just added one,
      // pop one off the end
      if((int)p.size() > k) p.pop_back();

    }
    // find largest distance in list
    r = q.distance(p.back());
  }

  // traverse down the "closer" side of the tree -- note that
  // these recursive calls may (will) override the p,r arguments
  // if a closer node than the current node is found
  //
  // the first approximation (e.g., leaf node) is not necessarily the
  // closest; but from this descent-only search we know that any potential
  // nearer neighbor must lie closer and so must lie within the circle
  // defined by center q and radius r
  //
  // as we return we need to check to see whether the current closest
  // circle (defined by center q and radius r) intersects the "farther"
  // side of the tree -- if it does, we must search that subtree
  axis = t->axis;
  if(q[axis] <= (*t->data)[axis])      {
    // check the circle (q,r) intersection with t->left (extraneous if)
  //if(q[axis] - r <= (*t->data)[axis]) knn(t->left,q,p,r,k);
    knn(t->left,q,p,r,k);
    // check the circle (q,r) intersection with t->right
    if(q[axis] + r > (*t->data)[axis]) knn(t->right,q,p,r,k);
  } else {
    // check the circle (q,r) intersection with t->right (extraneous if)
  //if(q[axis] + r > (*t->data)[axis]) knn(t->right,q,p,r,k);
    knn(t->right,q,p,r,k);
    // check the circle (q,r) intersection with t->left
    if(q[axis] - r <= (*t->data)[axis]) knn(t->left,q,p,r,k);
  }
}

template <typename T, typename P, typename C>
void kdtree_t<T,P,C>::range(kdnode_t* &t,const T& min,const T& max,std::vector<P>& p)
{
        bool    in_range=true;

  if(!t) return;

  // if data is within range query, add to list
//if( (min[0] <= (*t->data)[0]) && ((*t->data)[0] <= max[0])  &&
//    (min[1] <= (*t->data)[1]) && ((*t->data)[1] <= max[1])  &&
//    (min[2] <= (*t->data)[2]) && ((*t->data)[2] <= max[2]) ) {
//  (*p).push_back(t->data);
//}
  for(int i=0; i<t->data->dim(); i++)
    if( ((*t->data)[i] < min[i]) || (max[i] < (*t->data)[i]) )
      { in_range = false; break; }
  if(in_range) p.push_back(t->data);

  // figure out which side of tree to go into
  if( ((*t->data)[t->axis] >= min[t->axis]) &&
      ((*t->data)[t->axis] <= max[t->axis]) ) {
    // range box intersects split plane (straddles split axis)
    // need to traverse both sides of tree
    range(t->left,min,max,p);
    range(t->right,min,max,p);
  }
  else if((*t->data)[t->axis] >= max[t->axis])
    // range box fully contained on smaller side of space,
    // need only to traverse left side of tree
    range(t->left,min,max,p);
  else if((*t->data)[t->axis] <= min[t->axis])
    // range box fully contained on larger side of space,
    // need only to traverse left side of tree
    range(t->right,min,max,p);
}

/////////////////////////// specializations ////////////////////////////////////
template class kdtree_t<point_t,point_t*,point_c>;
template std::ostream& operator<<(std::ostream&, const kdtree_t<point_t,point_t*,point_c>&);
template class kdtree_t<photon_t,photon_t*,photon_c>;
template std::ostream& operator<<(std::ostream&, const kdtree_t<photon_t,photon_t*,photon_c>&);
