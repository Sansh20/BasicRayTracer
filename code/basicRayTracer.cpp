#include <fstream>
#include <iostream>
#include <limits>
#include <cmath>

#define AMBIENT  0
#define POINT 1
#define DIRECTIONAL 2

using namespace std;

float h = 500.0;
float w = 500.0;

class Vec{
    public:
        float x; float y; float z;
        Vec(){
            x = 0; y = 0; z = 0;
        };
        Vec(float a, float b, float c){
            x = a; y = b; z = c;
        }
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
        int r; int g; int b;
        Color(){
            r = 255; g = 255; b = 255;
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
        Sphere() {};
        Sphere(float radius, Color color, Vec center){
            rad = radius; col = color; cen = center;
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
                        Sphere(1.0, Color(255, 0, 0), Vec(0, -2.0, 4)), 
                        Sphere(1.0, Color(0, 0, 255), Vec(2, 1, 4)),
                        Sphere(500.0, Color(0, 255, 0), Vec(0, -510, 0))
                    };

Light lights[] = {
                    Light(AMBIENT, 0.2),
                    Light(POINT, 0.6, Vec(2, 1, 0)),
                    Light(DIRECTIONAL, 0.2, Vec(1, 4, 4))
                 };

auto computeLighting(Vec P, Vec N){
    auto intensity = 0.0;
    for(int i = 0; i<3; i++){
        if(lights[i].type==AMBIENT){
            intensity += lights[i].intensity;
        }
        else{
            Vec L;
            if(lights[i].type==POINT){
                L = lights[i].position;
            }
            else{
                L = lights[i].position;
            }
            auto nl = L*N;
            if(nl>0){
                intensity += lights[i].intensity*nl/(N.length()*L.length());
            }
        }
    }
    return intensity;
}

Color traceRay(Vec o, Vec d, float min_t, float max_t){
    float closest_t = numeric_limits<float>::infinity();
    bool sph = false;
    Vec sphCen;
    Color sphereCol;

    for(int i = 0; i<3; i++){
        float* ts = spheres[i].interesectRay(o, d);
        if(ts[0]<closest_t && ts[0]>min_t && ts[0]<max_t){
            closest_t = ts[0];
            sph = true;
            sphereCol = spheres[i].col;
            sphCen = spheres[i].cen;
        }
        if(ts[1]<closest_t && ts[1]>min_t && ts[1]<max_t){
            closest_t = ts[1];
            sph = true;
            sphereCol = spheres[i].col;
            sphCen = spheres[i].cen;
        }
    }

    if(!sph){
        float t =  0.5*(d.y + 1.0);
        return Color(255, 255, 255)*(1.0-t) + Color(127, 178, 255)*t;
    }
    else{
        Vec P = o + (d*closest_t);
        Vec N = P - sphCen;
        N = N*(1.0/N.length());
        return sphereCol*computeLighting(P,N);
    }
};

int main(){
    ofstream render("./out.ppm", ios::out | ios::binary);
    render<<"P3"<<endl;
    render<<w<<" "<<h<<endl;
    render<<"255"<<endl;
    for(int y = h/2; y>-h/2; y--){
        for(int x = -w/2; x<w/2; x++){
            Vec dir = cameraToViewport(x, y);
            Color color = traceRay(cameraPos, dir, 1.0, numeric_limits<float>::infinity());
            render<< int(color.r)<< ' ' <<int(color.g)<< ' ' << int(color.b)<<endl;
        }
    }
    return 0;
}
