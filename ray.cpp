#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cassert>
#include <cmath>

#include "vector.h"
#include "pixel.h"
#include "camera.h"
#include "light.h"
#include "material.h"
#include "object.h"
#include "list.h"
#include "plane.h"
#include "model.h"
#include "photon.h"
#include "ray.h"
#include "kdtree.h"

void ray_t::trace(model_t&      model,
    rgb_t<double>&  color,
    int         bounce,
    kdtree_t<photon_t,photon_t*,photon_c>& kdtree)
{
	// returned data from find_closest function, might not be usable
	double				rdis=0.0;
	vec_t				rhit,rN;
	object_t			*robj=NULL;

	// (static) object properties
	rgb_t<double>			ambient, diffuse, specular;
    double alpha, ior;
	material_t			*mat;

	// light and illumination model variables
	light_t				*lgt=NULL;
	vec_t				L,V,R,H;
	rgb_t<double>			I_d, I_s;
	double				r,ndotl=0.0,n=32.0;
	list_t<light_t* >::iterator	litr;

    if (bounce > 5) 
        return;

  // get closest object, if any
  if(!(robj = model.find_closest(pos,dir,obj,rdis,rhit,rN)) || dis > MAX_DIST)
    return;

#ifdef DEBUG
    std::cerr << "distance: " << dis << std::endl;
#endif

  // if hit distance valid, compute color at surface
  if(rdis > 0) {
#ifdef DEBUG
    std::cerr << "Hit: " << obj->getname();
    std::cerr << "Dist = " << rdis;
    std::cerr << "Loc = " << rhit << std::endl;
#endif

    // set this ray's last hit object, hit point and normal at hit point
    obj = robj;  hit = rhit;  N = rN;

    // accumulate distance travelled by ray
    dis += rdis;

    // get object material properties
    if((mat = model.getmaterial(obj->getmaterial())) != NULL) {
      ambient = mat->getamb();
      diffuse = mat->getdiff();
      specular = mat->getspec();
      alpha = mat->getalpha();
      ior = mat->getior();
    }

    // ambient color
    color += 1.0/dis * ambient;			// ambient scaled by ray dist

    // clamp resultant color
    color.clamp(0.0,1.0);

    // view direction (direction from hit point to camera)
    //V = model.getviewpoint() - hit;	// dir to camera
    //V = V.norm();
    V = -dir;				// dir to camera

    // diffuse component from each light...
    if(!diffuse.iszero()) {
      for(litr = model.lgts.begin(); litr != model.lgts.end(); litr++) {
        // pointer to light
        lgt = (light_t *)*litr;
  
        // light direction and distance
        L = lgt->getlocation() - hit;
        r = L.len();				// distance to light
        L = L.norm();				// dir to light
  
        // angle with light
        ndotl = N.dot(L);
  
        // check visibility wrt light
        if(0.0 < ndotl && ndotl < 1.0) {
          // light color scaled by N . L
          I_d = 1.0/r * ndotl * lgt->getcolor();
  
          // add in diffuse contribution from light scaled by material property
          color += (I_d * diffuse);
        }
      }
  
      // clamp resultant color
      color.clamp(0.0,1.0);
    }

    // reflection
    if(!specular.iszero()) {

//	vect_t		out = dir.reflect(N).norm();
	vec_t		out = (((2.0 * N.dot(-dir)) * N) - -dir).norm();
	rgb_t<double>	refcolor;

      // spawn new reflection ray
      ray_t *reflection = new ray_t(hit,out);

      // set reflection ray's last hit object to current obj
      reflection->obj = obj;

      // set reflection ray's distance to current dis
      reflection->dis = dis;

      // trace the reflection
      reflection->trace(model,refcolor, bounce+1, kdtree);
      delete reflection;

      // composite surface color: blending baesd on surface properties
      // (note that diffuse + specular should add up to 1)
      color  = (diffuse * color) + (specular * refcolor);

      // clamp resultant color
      color.clamp(0.0,1.0);
    }

    // refract if material is transparent
    if (alpha > 0) {
        double udotN = dir.dot(N);
        vec_t t;
        rgb_t<double>   transmitted_color;

        // determine if the ray is pointing in the circle or not
        if (udotN < 0) {
            // refract the ray about N
            t = dir.refract(N, ior).norm();
        } else {
            // refract the ray about -N with 1/ior
            t = dir.refract(-N, 1.0/ior).norm();
        }
    
        // create our transmission array
        ray_t *transmission = new ray_t(hit, t, dis);
        transmission->trace(model, transmitted_color, bounce+1,kdtree);
        delete transmission;

        color = ((1.0 - alpha) * color) + (alpha * transmitted_color);
        color.clamp(0.0, 1.0);
    }


    // specular highlights
    if(!specular.iszero()) {

      // add in specular highlight from each light...
      for(litr = model.lgts.begin(); litr != model.lgts.end(); litr++) {
        // pointer to light
        lgt = (light_t *)*litr;

        // light direction and distance
        L = lgt->getlocation() - hit;
        r = L.len();				// distance to light
        L = L.norm();				// dir to light

        // angle with light
        ndotl = N.dot(L);

        // check visibility wrt light
        if(0.0 < ndotl && ndotl < 1.0) {
          // specular reflection direction (and bisector)
        //R = (L.reflect(N)).norm();
          R = (((2.0 * ndotl) * N) - L).norm();
          H = (0.5 * (L + V)).norm();

          // light color scaled by (R . V)^n
          I_s = 1.0/r * pow(R.dot(V),n) * lgt->getcolor();
  
          // add in specular contribution from light scaled by material property
          color += (I_s * specular);
        }

        // clamp resultant color
        color.clamp(0.0,1.0);
      }
    }
  }

  std::vector<photon_t*> knearest;
  double radius;

  photon_t *query = new photon_t(hit, vec_t(0.0, 0.0, 0.0), vec_t(0.0, 0.0, 0.0));
  kdtree.knn(*query, knearest, radius, 50);
/*
  rgb_t<double> flux(0.0, 0.0, 0.0);
  rgb_t<double> tmp_color(0.0, 0.0, 0.0);
  vec_t tmp_pwr;
  vec_t tmp_dir;

  for (int i = 0; i <20; i++) {
    tmp_pwr = knearest[i]->getpower();
    tmp_dir = knearest[i]->getdir();
    tmp_dir = tmp_dir.norm();

    if (-tmp_dir.dot(N) > 0) {
        tmp_color[0] = tmp_pwr[0];
        tmp_color[1] = tmp_pwr[1];
        tmp_color[2] = tmp_pwr[2];
    } else {
        tmp_color[0] = 0;
        tmp_color[1] = 0;
        tmp_color[2] = 0;
    }
  }
  flux = flux + tmp_color;

    flux[0] = flux[0] * (1/(M_PI*radius*radius));
    flux[1] = flux[1] * (1/(M_PI*radius*radius));
    flux[2] = flux[2] * (1/(M_PI*radius*radius));
    color = color + flux;
    color.clamp(0.0, 1.0);
*/
}
