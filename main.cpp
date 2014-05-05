/******************************************
 * Jacob Harrelson (tharrel@g.clemson.edu)
 * November 8, 2011
 * CPSC 212 - Section 1
 * Assignment 04
 ******************************************/
#include <omp.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
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
#include "ray.h"
#include "photon.h"
#include "timer.h"
#include "kdtree.h"

// prototypes
int  main(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	model_t		model;		// model (the world)
	std::ifstream	model_ifs;	// input file stream
	atd::timer_t	timer;

  if(argc != 2) {
    std::cerr << "Usage " << argv[0] << " <model_file>" << std::endl;
    return 1;
  }

  // open file, read in model
  model_ifs.open(argv[1],std::ifstream::in);
  model_ifs >> model;
  model_ifs.close();

  // debugging
  std::cerr << model;

	int		tid,nthreads=omp_get_num_threads();
	int		w=model.getpixel_w(), h=model.getpixel_h();
	int		chunk;
	double		wx,wy,wz=0.0;
	double		ww=model.getworld_w(), wh=model.getworld_h();
	vec_t		pos=model.getviewpoint();
	vec_t		pix,dir;
	ray_t		*ray=NULL;
    photon_t    *photon=NULL;
	rgb_t<double>	color;
	rgb_t<uchar>	*imgloc,*img=NULL;
    kdtree_t<photon_t, photon_t*, photon_c> kdtree;

  img = new rgb_t<uchar>[w*h];
  memset(img,0,3*w*h);

  // create photon vector
  std::vector<photon_t*> photons;
  // shoot photons
  model.shoot(photons);

  // output photons to photons.pts
  std::ofstream out("photons.pts");
  std::vector<photon_t*>::iterator pitr;
  for (pitr = photons.begin(); pitr != photons.end(); pitr++) {
    out << *pitr;
  }
  out.close();

  // initialize min and max for photon kdtree
  int photon_sz = photons.size();
  photon_t min = *photons[0];
  photon_t max = *photons[photon_sz - 1];

  // scale photons power
  for (int i = 0; i < photon_sz; i++) {
    vec_t pwr = photons[i]->getpower();

    pwr[0] = pwr[0] / photon_sz;
    pwr[1] = pwr[1] / photon_sz;
    pwr[2] = pwr[2] / photon_sz;

    photons[i]->setpower(pwr);

    if (*photons[i] > max) 
      max = *photons[i];
    if (*photons[i] < min) 
      min = *photons[i];
  }

  // insert photons into kdtree
  kdtree.insert(photons, min, max);

  // figure out how many threads we have available, then calculate chunk,
  // splitting up the work as evenly as possible
  #pragma omp parallel private(tid)
  {
    if((tid = omp_get_thread_num())==0)
      nthreads = omp_get_num_threads();
  }
  chunk = h/nthreads;

  timer.start();

// two statements...
//#pragma omp parallel \
//          shared(model,w,h,ww,wh,wz,pos,img) \
//          private(tid,wx,wy,pix,dir,ray,color,imgloc)
//#pragma omp for schedule(static,1) nowait

// ...or all-in-one
  #pragma omp parallel for \
            shared(model,w,h,ww,wh,wz,pos,img) \
            private(tid,wx,wy,pix,dir,ray,color,imgloc) \
            schedule(static,chunk)
  for(int y=h-1;y>=0;y--) {
    for(int x=0;x<w;x++) {
      wx = (double)x/(double)(w-1) * ww;
      wy = (double)y/(double)(h-1) * wh;

      // set pixel position vector (in world coordinates)
      pix = vec_t(wx,wy,wz);

      // compute the vector difference  v3 = v2 - v1
      dir = pix - pos;

      // our ray is now {pos, dir} (in world coordinates), normalize dir
      dir = dir.norm();

      // zero out color
      color.zero();

      // spawn new ray
      ray = new ray_t(pos,dir);

      // trace ray
      ray->trace(model,color, 1, kdtree);

      // nuke this ray, we don't need it anymore, prevent memory leak
      delete ray;

      // where are we in the image (using old i*c + j)
      imgloc = img + y*w + x;

      // scale pixel by maxval, store at dereferenced image location
//    if(tid) // fake banding
      for(int i=0;i<3;i++) (*imgloc)[i] = static_cast<uchar>(255.0 * color[i]);
//    else
//    if(y < 240)
//      *imgloc = rgb_t<uchar>(255,0,0);
//    else
//      *imgloc = rgb_t<uchar>(0,255,0);

      // instead of using image, just write pixel to stdout
    //std::cout.write((char *)imgloc,3);
    }
  }
  timer.end();

  std::cerr << "threads: " << nthreads << ", ";
  std::cerr << "time: " << timer.elapsed_ms() << " ms" << std::endl;

  // output image
  std::cout << "P6 " << w << " " << h << " " << 255 << std::endl;
  for(int y=h-1;y>=0;y--) {
    for(int x=0;x<w;x++) {
      imgloc = img + y*w + x;
      std::cout.write((char *)imgloc,3);
    }
  }

  return 0;
}
