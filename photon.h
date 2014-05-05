#ifndef PHOTON_H
#define PHOTON_H
#include <cstdlib>

#define MAX_BOUNCES 10

#include "ray.h"

class photon_t : public ray_t {
    public:
        // constructors
        photon_t() : \
            ray_t(), \
            power(50.0, 50.0, 50.0) \
            { };

        photon_t(const vec_t& o) : \
            ray_t(o, genrand_hemisphere(), 0.0), \
            power(50.0, 50.0, 50.0) \
            { };

        photon_t(const vec_t& o, const vec_t& d, double r=0.0) : \
            ray_t(o, d, r), \
            power(50.0, 50.0, 50.0) \
            { };

        photon_t(const vec_t& o, const vec_t& d, const vec_t& p, double r=0.0) : \
            ray_t(o, d, r), \
            power(p) \
            { };
        
        // destructor
        ~photon_t() {
        }

        // operator overloads
        friend bool operator>(photon_t& lhs, photon_t& rhs) {
            for(int i = 0; i < 3; i++) {
                if (lhs[i] < rhs[i])
                    return false;
            }
            return true;
        }

        friend bool operator<(photon_t& lhs, photon_t& rhs) {
            for (int i = 0; i < 3; i++) {
                if (lhs[i] > rhs[i])
                    return false;
            }
            return true;
        }
    
        const double& operator[](int i) const   { return pos[i]; }
        double& operator[](int i)               { return pos[i]; }

        // ostream overload
        friend std::ostream& operator<<(std::ostream& s, photon_t& rhs) {
            s << rhs.pos[0] << " " << rhs.pos[1] << " " << rhs.pos[2] << " ";
            s << rhs.power[0] << " " << rhs.power[1] << " " << rhs.power[2];
            s << std::endl;
            return s;
        }

        friend std::ostream& operator<<(std::ostream& s, photon_t *rhs) {
            return (s << *rhs);
        }

        // istream overload
        friend std::istream& operator>>(std::istream& s, photon_t& rhs) {
            s >> rhs.pos[0] >> rhs.pos[1] >> rhs.pos[2] >> rhs.power[0] >> rhs.power[1] >> rhs.power[2];
            return s;
        }
        friend std::istream& operator>>(std::istream& s, photon_t *rhs) {
            return (s >> *rhs);
        }

        // member functions
        vec_t genrand_hemisphere();
        bool caustic(model_t& model, int bounces);
        bool global(model_t& model, int bounces);
        
        vec_t getpower()           { return power; }
        void setpower(vec_t pwr)   { power = pwr; }

        double genrand(double lo, double hi) {
            return ((double)(((double)rand() / (double)RAND_MAX) * hi + lo));
        }

        double distance(const photon_t& rhs) {
            vec_t diff = pos - rhs.pos;
            return sqrt(diff.dot(diff));
        }
        
        double distance(photon_t*& rhs) {
            return distance(rhs);
        }

        double distance(const vec_t& rhs) {
            vec_t diff = pos - rhs;
            return sqrt(diff.dot(diff));
        }

        int dim()   { return 3; }
        
    protected:
        vec_t   power;
};

// photon functor
class photon_c {
    public:
        photon_c(int inaxis=0) : axis(inaxis) 
        { };

        bool operator()(const photon_t& p1, photon_t& p2) const {
            return (p1[axis] < p2[axis]); 
        }
    
        bool operator()(const photon_t*& p1, photon_t*& p2) const {
            return ((*p1)[axis] < (*p2)[axis]);
        }

        bool operator()(photon_t * const & p1, photon_t * const & p2) const {
            return ((*p1)[axis] < (*p2)[axis]);
        }

    private:
        int axis;
};
#endif
