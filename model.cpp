#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <cassert>

#include "vector.h"
#include "pixel.h"
#include "material.h"
#include "object.h"
#include "list.h"
#include "plane.h"
#include "sphere.h"
#include "light.h"
#include "camera.h"
#include "model.h"
#include "ray.h"
#include "photon.h"

std::istream& operator>>(std::istream& s, model_t& rhs)
{
	light_t		*lgt;
	material_t	*mat;
	object_t	*obj;
	std::string	token,name;

  while(!s.eof()) {

    if((s >> token).good()) {

      if(token == "light") {
        s >> (lgt = new light_t());
        std::cerr << "loaded " << lgt->getname() << std::endl;
        rhs.lgts.push_back(lgt);
      }

      if(token == "camera") {
        s >> rhs.cam;
        std::cerr << "loaded " << rhs.cam.getname() << std::endl;
      }

      if(token == "material") {
        s >> (mat = new material_t());
        std::cerr << "loaded " << mat->getname() << std::endl;
        rhs.mats.push_back(mat);
      }

      if(token == "plane") {
        s >> (obj = new plane_t(token));
        std::cerr << "loaded " << obj->getname() << std::endl;
        rhs.objs.push_back(obj);
      }

      if(token == "sphere") {
        s >> (obj = new sphere_t(token));
        std::cerr << "loaded " << obj->getname() << std::endl;
        rhs.objs.push_back(obj);
      }
    }
  }

  return s;
}

std::ostream& operator<<(std::ostream& s, model_t& rhs)
{
	light_t				*lgt;
	object_t			*obj;
	material_t			*mat;

	list_t<light_t* >::iterator	litr;
	list_t<material_t* >::iterator	mitr;
	list_t<object_t* >::iterator	oitr;

  // print out camera 
  s << rhs.cam;

  // print out lights, materials, objects 
  for(litr = rhs.lgts.begin(); litr != rhs.lgts.end(); litr++) s << *litr;
  for(mitr = rhs.mats.begin(); mitr != rhs.mats.end(); mitr++) s << *mitr;
  for(oitr = rhs.objs.begin(); oitr != rhs.objs.end(); oitr++) s << *oitr;

  return s;
}

object_t* model_t::find_closest(vec_t& pos, vec_t& dir,
                                object_t *last_obj, double& retdist,
                                vec_t& rethit, vec_t& retN)
{
	double				dist,closest_dist=INFINITY;	
	object_t			*obj=NULL, *closest_obj=NULL;
	vec_t				hit,N;
	vec_t				closest_hit,closest_N;
	list_t<object_t* >::iterator	oitr;

  // check each object; basic list iteration
  for(oitr = objs.begin(); oitr != objs.end(); oitr++) {
    obj = (object_t *)*oitr;
    // if same object or no hit, keep going (side effect: dist calculated)
    dist = obj->hits(pos, dir, hit, N);
    if (dist < 0) {
        continue;
    } else {
    // if first hit or smaller dist, set closest
        if ((0.00001 < dist) && (dist < closest_dist)) {
            closest_dist = dist;
            closest_obj = obj;
            closest_hit = hit;
            closest_N = N;
        }
    }
  }

  rethit = closest_hit;
  retN = closest_N;
  retdist = closest_dist;
  return(closest_obj);
}

material_t* model_t::getmaterial(std::string name)
{
	material_t			*mat;
	list_t<material_t* >::iterator	mitr;

  for(mitr = mats.begin(); mitr != mats.end(); mitr++) {
    mat = (material_t *)*mitr;
    assert(mat->getcookie() == MAT_COOKIE);
    if(mat->getname() == name) return mat;
  }
  return NULL;
}

void model_t::shoot(std::vector<photon_t* >& photons) {
    list_t<light_t*>::iterator litr;

    for (litr = lgts.begin(); litr != lgts.end(); litr++) {
        for (int i=0; i < 5000; i++) {
            photon_t *photon = new photon_t((*litr)->getlocation());
            if (photon->caustic(*this, 0)) {
                photons.push_back(photon);
            } else {
                delete photon;
            }
        }

        for (int j=0; j < 20000; j++) {
            photon_t *photon = new photon_t((*litr)->getlocation());
            if (photon->global(*this, 0)) {
                photons.push_back(photon);
            } else {
                delete photon;
            }
        }
    }
}
