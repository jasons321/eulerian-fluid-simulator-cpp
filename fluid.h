#pragma once
#include <vector>

enum class FieldType { U, V, SMOKE };

class Fluid {
public:
    Fluid(float density, int numX, int numY, float h);

    void integrate(float dt, float gravity);
    void solveIncompressibility(int iterations, float dt, float overRelaxation);
    float sampleField(float x, float y, FieldType field) const; // bilinear interpolation
    float avgV(int i, int j); // average velocity magnitude at cell (i,j)
    float avgU(int i, int j); // average velocity magnitude at cell (i,j)
    void extrapolate();
    void advectVel(float dt);
    void advectSmoke(float dt);


    inline int idx(int i, int j) const {
        return i + j * numX;
    }

    float density;
    int numX, numY;   // remember: these already include the +2 border
    int numCells;
    float h;          // grid spacing

    std::vector<float> u, v, newU, newV;
    std::vector<float> p;
    std::vector<float> s;
    std::vector<float> m, newM;
};


