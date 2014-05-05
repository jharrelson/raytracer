#include <iostream>
#include <cstdlib>
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
#include "ray.h"
#include "photon.h"

vec_t photon_t::genrand_hemisphere() {
    double azimuth = genrand(0.0, 2.0*M_PI);
    double elevation = genrand(0.0, 2.0*M_PI);

    double sinA = sin(azimuth), sinE = sin(elevation);
    double cosA = cos(azimuth), cosE = cos(elevation);

    vec_t dir, vup, out;

    dir[0] = -sinA * cosE;  vup[0] = sinA * sinE;
    dir[1] = sinE;          vup[1] = cosE;
    dir[2] = cosA * cosE;   vup[2] = -cosA * sinE;

    out = (dir + vup).norm();

    return out;
}

bool photon_t::caustic(model_t& model, int bounce) {
    double rdis;
    object_t *robj;
    double alpha, ior;
    double fp(0.0), fd(1.0);
    double fa;

    if (bounce > MAX_BOUNCES) return false;
    
    // get closest object
    if (!(obj = model.find_closest(pos, dir, obj, dis, hit, N)) || dis > 100) return false;

    if (dis > 0) {
        material_t *mat = model.getmaterial(obj->getmaterial());
        if (mat != NULL) {
            alpha = mat->getalpha();
            ior = mat->getior();
        }

        if (alpha > 0) {
            pos = hit;
            dir = dir.dot(N) < 0 ? dir.refract(N, ior) : 
                                 dir.refract(-N, 1.0/ior);

            if (fp < 1000.0) 
                fa = 1.0 / (1.0 + pow(dis / fd, fp));
            else
                fa = exp(-dis/fd);

            power = fa * power;

            return (caustic(model, bounce+1));
        } else {
            pos = hit;
            return true;
        }
    }
}

bool photon_t::global(model_t& model, int bounce) {

    object_t    *obj = NULL;
    material_t  *mat;
    rgb_t<double>   ambient, diffuse, specular;
    double          alpha=0.0, ior;
    double          Kd, Ks;
    vec_t           hit, N;

    double roulette = genrand(0.0, 1.0);

    if (bounce > MAX_BOUNCES) 
        return false;

    if (!(obj = model.find_closest(pos, dir, obj, dis, hit, N)) || dis > 100)
        return false;

    if (dis > 0) {
        mat = model.getmaterial(obj->getmaterial());
        if (mat != NULL) {
            ambient = mat->getamb();
            diffuse = mat->getdiff();
            specular = mat->getspec();
            alpha = mat->getalpha();
            ior = mat->getior();
        }

        Kd = (diffuse[0] + diffuse[1] + diffuse[2]) / 3.0;
        Ks = (specular[0] + specular[1] + specular[2]) / 3.0;

        if ((0.0 < roulette) && (roulette < Kd)) {
            pos = hit;
            dir = genrand_hemisphere();
            return (global(model, bounce+1));
        } else if ((Kd < roulette) && (roulette < Kd+Ks)) {
            pos = hit;
            dir = dir.reflect(N);
            return (global(model, bounce+1));
        } else if ((Kd+Ks < roulette) && (roulette < 1.0)) {
            pos = hit;
            return true;
        }
    } else {
        return false;
    }
}
/*
    double rdis;
    object_t *robj;

    rgb_t<double> diffuse;
    rgb_t<double> specular;
    double roulette = genrand(0.0, 1.0);
    
    if (bounce > MAX_BOUNCES) return false; // MAX BOUNCES

    // get closeset object
    if (!(obj = model.find_closest(pos, dir, obj, dis, hit, N)) || dis > 100)  return false;

    if (dis > 0) {
        material_t *mat = model.getmaterial(obj->getmaterial());
        if (mat != NULL) {
            diffuse = mat->getamb();
            specular = mat->getspec();
        }
        
        double Kd = (diffuse[0] + diffuse[1] + diffuse[2]) / 3.0;
        double Ks = (specular[0] + specular[1] + specular[2]) / 3.0;
        double roulette = genrand(0.0, 1.0);

        if ((0.0 < roulette) && (roulette < Kd)) {
            pos  = hit;
            dir = genrand_hemisphere().norm();
            return global(model, bounce+1);
        } else if ((Kd < roulette) && (roulette < Kd+Ks)) {
            std::cerr << "case 2" << std::endl;
            pos = hit;
            dir = dir.reflect(N);
            return global(model, bounce+1);
        } else if ((Kd+Ks < roulette) && (roulette < 1.0)) {
            std::cerr << "case 3" << std::endl;
            pos = hit;
            return true;
        }
    }
}
*/
