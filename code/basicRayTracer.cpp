#include <fstream>
#include <iostream>
#include <limits>
#include <cmath>

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
        float operator * (Vec const obj1){
            Vec obj2;
            float res;
            obj2.x = x * obj1.x;
            obj2.y = y * obj1.y;
            obj2.z = z * obj1.z;
            res = obj2.x + obj2.y + obj2.z;
            return res;
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
         }
};

class Sphere{
    public: 
        float rad;
        Color col;
        Vec cen;
        
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

            if(discr<0){
                t[0] = numeric_limits<float>::infinity();
                t[1] = numeric_limits<float>::infinity();
                return t;
            }

            t[0] = (-(k2) + (sqrt(discr)))/(2*k1);
            t[1] = (-(k2) - (sqrt(discr)))/(2*k1);
            return t;
        }

};

Vec cameraPos = Vec(0, 0, 0);
int viewport_size = 1.0;
float vW = viewport_size/w;
float vH = viewport_size/h;
int projection_plane_z = 1.0;

Vec cameraToViewport(int x , int y){
    return Vec(x*vW, y*vH, projection_plane_z);
}

Sphere spheres[] =  {
                        Sphere(1.0, Color(255, 0, 0), Vec(0, -1, 3)), 
                        Sphere(1.0, Color(0, 0, 255), Vec(2, 0, 4)),
                    };

Color traceRay(Vec o, Vec d, float min_t, float max_t){
    float closest_t = numeric_limits<float>::infinity();
    Color sphereCol;
    bool sph = false;

    for(int i = 0; i<2; i++){
        float* ts = spheres[i].interesectRay(o, d);
        if(ts[0]<closest_t && ts[0]>min_t && ts[0]<max_t){
            closest_t = ts[0];
            sph = true;
            sphereCol = spheres[i].col;
        }
        if(ts[1]<closest_t && ts[1]>min_t && ts[1]<max_t){
            closest_t = ts[1];
            sph = true;
            sphereCol = spheres[i].col;
        }
    }

    return sphereCol;
}

int main(){
    ofstream render("./imgOut.ppm", ios::out | ios::binary);
    render<<"P3"<<endl;
    render<<w<<" "<<h<<endl;
    render<<"255"<<endl;
    for(int y = h/2; y>-h/2; y--){
        for(int x = -w/2; x<w/2; x++){
            Vec dir = cameraToViewport(x, y);
            Color color = traceRay(cameraPos, dir, 1.0, numeric_limits<float>::infinity());
            render<< color.r<< ' ' <<color.g<< ' ' << color.b<<endl;
        }
    }
    return 0;
}
