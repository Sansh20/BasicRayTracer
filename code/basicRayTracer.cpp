#include <fstream>
#include <iostream>
#include <limits>
#include <cmath>

#define AMBIENT  0
#define POINT 1
#define DIRECTIONAL 2
#define recursion_depth 3

using namespace std;

float h = 500.0;
float w = 500.0;

class Vec{
    public:
        float x, y, z;
        Vec(){
            x = 0; y = 0; z = 0;
        };
        Vec(float a, float b, float c){
            x = a; y = b; z = c;
        }
        Vec operator - (){return Vec(-x, -y, -z);};
        Vec operator - (Vec const obj1){
            Vec obj2;
            obj2.x = x - obj1.x;
            obj2.y = y - obj1.y;
            obj2.z = z - obj1.z;
            return obj2;
        };
        Vec operator + (Vec const obj1){
            Vec obj2;
            obj2.x = x + obj1.x;
            obj2.y = y + obj1.y;
            obj2.z = z + obj1.z;
            return obj2;
        };
        float operator * (Vec const obj1){
            Vec obj2;
            float res;
            obj2.x = x * obj1.x;
            obj2.y = y * obj1.y;
            obj2.z = z * obj1.z;
            res = obj2.x + obj2.y + obj2.z;
            return res;
        }  
        Vec operator * (float const obj1){
            Vec obj2;
            obj2.x = obj1 * x;
            obj2.y = obj1 * y;
            obj2.z = obj1 * z;
            return obj2;
        }
        auto length(){
            return sqrt(x*x+y*y+z*z);
        }
};

class Light{
    public:
        float type;
        float intensity;
        Vec position;
        Light(float lightType, float intens){
            type = lightType;
            intensity = intens;
        }
        Light(float lightType, float intens, Vec pos){
            type = lightType;
            intensity = intens;
            position = pos;
        }
};

class Color{
    public:
        float r; float g; float b;
        Color(){
            r = 1.0; g = 1.0; b = 1.0;
        };
        Color(int red, int green, int blue){
            r = red; g = green; b = blue; 
        };
        Color operator * (float const obj1){
            Color obj2;
            obj2.r = obj1 * r;
            obj2.g = obj1 * g;
            obj2.b = obj1 * b;
            return obj2;
        }
        Color operator + (Color const obj1){
            Color obj2;
            obj2.r = r + obj1.r;
            obj2.g = g + obj1.g;
            obj2.b = b + obj1.b;
            return obj2;
        };
};

class Sphere{
    public: 
        float rad;
        Color col;
        Vec cen;
        int spec;
        float ref;
        Sphere() {};
        Sphere(float radius, Color color, Vec center, int specular, float reflective){
            rad = radius; col = color; cen = center; spec = specular; ref = reflective;
        };

        float* interesectRay(Vec origin, Vec dir){
            Vec OC = origin - cen;
            float k1 = dir * dir;
            float k2 = 2*(OC*dir);
            float k3 = (OC*OC) - (rad*rad);
            static float t[2];
            float discr = (k2*k2) - 4*k1*k3;
            if(discr<0.0){
                t[0] = 0.0;
                t[1] = 0.0;
                return t;
            }
            
            t[0] = (-k2 + sqrt(discr)) / (2*k1);
            t[1] = (-k2 - sqrt(discr)) / (2*k1);
            return t;
        }

        Sphere operator = (Sphere obj1){
            rad = obj1.rad;
            col = obj1.col;
            cen = obj1.cen;
            spec = obj1.spec;
            return *this;
        }
        bool operator==(Sphere obj1){
            return (rad==0);
        }
        
};

Vec cameraPos = Vec(0, 0, 0);
int viewport_size = 1.0;
float vW = viewport_size/w;
float vH = viewport_size/h;
int projection_plane_z = 1.0;

Vec cameraToViewport(int x , int y){
    return Vec(x*vW, y*vH, projection_plane_z);
};


Sphere spheres[] =  {
                        Sphere(1.0, Color(1.0, 0, 0), Vec(-1.0, 0.0, 5), -1, 0.0), 
                        Sphere(1.0, Color(0, 0, 1.0), Vec(1.0, 0.0, 5), 1000, 0.2),
                        Sphere(500.0, Color(0, 1.0, 0), Vec(0, -501, 0), -1, 0.3) 
                    };

Light lights[] = {
                    Light(AMBIENT, 0.2),
                    Light(POINT, 0.6, Vec(1, 1, 0)),
                    Light(DIRECTIONAL, 0.2, Vec(1, 2, 2))
                 };

struct sphInfo{
    int index;
    float t;
};
auto closestSphereHit(Vec o, Vec d, float min_t, float max_t){
    float closest_t = numeric_limits<float>::infinity();
    int closestSphereIndex = -1;

    for(int i = 0; i<3; i++){
        float* ts = spheres[i].interesectRay(o, d);
        if(ts[0]<closest_t && ts[0]>min_t && ts[0]<max_t){
            closest_t = ts[0];
            closestSphereIndex = i;
        }
        if(ts[1]<closest_t && ts[1]>min_t && ts[1]<max_t){
            closest_t = ts[1];
            closestSphereIndex = i;
        }
    }
    sphInfo sph = {closestSphereIndex, closest_t};

    return sph;
};

auto computeLighting(Vec P, Vec N, Vec V, int s){
    auto intensity = 0.0;
    for(int i = 0; i<3; i++){
        if(lights[i].type==AMBIENT){
            intensity += lights[i].intensity;
        }
        else{
            Vec L;
            float t_max;
            if(lights[i].type==POINT){
                L = lights[i].position;
                t_max = 1;
            }
            else{
                L = lights[i].position;
                t_max = numeric_limits<float>::infinity();
            }

            sphInfo sph = closestSphereHit(P, L, 0.001, t_max);
            if(sph.index!=-1){
                continue;
            }
            
            auto nl = L*N;
            if(nl>0){
                intensity += lights[i].intensity*nl/(N.length()*L.length());
            }

            if(s!=-1){
                Vec R = (N*2)*nl -L; 
                auto rv = R*V;
                if(rv>0){
                    intensity += lights[i].intensity*pow(rv/(R.length()*V.length()), (float)(s));
                }
            }
        }
    }
    return intensity;
}


Color traceRay(Vec o, Vec d, float min_t, float max_t, int depth){
    sphInfo sph = closestSphereHit(o, d, min_t, max_t);
    if(sph.index<0){
        float t =  0.5*(d.y + 1.0);
        return Color(1.0, 1.0, 1.0)*(1.0-t) + Color(0.4, 0.7, 1.0)*t;
    }
    Sphere closestSphere = spheres[sph.index];
    Vec P = o + (d*sph.t);
    Vec N = P - closestSphere.cen;
    N = N*(1.0/N.length());
    Color local_color = closestSphere.col*computeLighting(P, N, -d, closestSphere.spec);
    float r = closestSphere.ref;
    if (depth<=0 || r<=0)
    {
        return local_color;
    }
    
    Vec R = ((N*(-d*N))*2)-(-d);
    
    Color reflected_color = traceRay(P, R, 0.001, numeric_limits<float>::infinity(), depth-1);

    return local_color*(1-r) + reflected_color*r;
};

int main(){
    ofstream render("./out.ppm", ios::out | ios::binary);
    render<<"P3"<<endl;
    render<<w<<" "<<h<<endl;
    render<<"255"<<endl;
    for(int y = h/2; y>-h/2; y--){
        for(int x = -w/2; x<w/2; x++){
            Vec dir = cameraToViewport(x, y);
            Color color = traceRay(cameraPos, dir, 0.001, numeric_limits<float>::infinity(), recursion_depth);
            render<< int(255.999*color.r)<< ' ' <<int(255.999*color.g)<< ' ' << int(255.999*color.b)<<endl;
        }
    }
    return 0;
}
