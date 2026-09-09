#pragma once
#include <vector>

enum class FieldType { U, V, SMOKE };

class Fluid {
public:
    Fluid(float density, int numX, int numY, float h);

    inline void integrate(float dt, float gravity) {
        for (int i = 1; i < numX; ++i) {
            for (int j = 1; j < numY; ++j) {
                if (s[idx(i, j)] != 0.0f && s[idx(i, j - 1)] != 0.0f) {
                    v[idx(i, j)] += gravity * dt; // gravity
                }
            }
        }
    }

    inline void solveIncompressibility(int iterations, float dt, float overRelaxation) {
        float cp = density * h / dt; 
        for (int k = 0; k < iterations; ++k) {
            for (int i = 1; i < numX - 1; ++i) {
                for (int j = 1; j < numY - 1; ++j) {
                    if (s[idx(i, j)] == 0.0f) {
                        continue; // skip empty cells
                    }

                    float d = (u[idx(i + 1, j)] - u[idx(i, j)]) +
                                        (v[idx(i, j + 1)] - v[idx(i, j)]) ;
                    float sx0 = s[idx(i - 1, j)]; 
                    float sx1 = s[idx(i + 1, j)];
                    float sy0 = s[idx(i, j - 1)];
                    float sy1 = s[idx(i, j + 1)];

                    float ssum = sx0 + sx1 + sy0 + sy1;
                    if (ssum == 0.0f) {
                        continue; // skip if all neighbors are empty
                    }

                    float p_new = d / ssum;
                    p_new *= overRelaxation;
                    p[idx(i, j)] += cp * p_new;

                    u[idx(i, j)] += p_new * sx0;
                    u[idx(i + 1, j)] -= p_new * sx1;
                    v[idx(i, j)] += p_new * sy0;
                    v[idx(i, j + 1)] -= p_new * sy1;
                }
            }
        }
    }

    //bilinear interpolation
    inline float Fluid::sampleField(float x, float y, FieldType field) const {
        float h1 = 1.0f / h;
        float h2 = 0.5f * h;

        // clamp the query point to stay inside the valid grid range
        x = std::max(std::min(x, numX * h), h);
        y = std::max(std::min(y, numY * h), h);

        float dx = 0.0f, dy = 0.0f;
        const std::vector<float>* f = nullptr;

        switch (field) {
            case FieldType::U:     f = &u; dy = h2; break;
            case FieldType::V:     f = &v; dx = h2; break;
            case FieldType::SMOKE: f = &m; dx = h2; dy = h2; break;
        }

        int x0 = std::min((int)std::floor((x - dx) * h1), numX - 1);
        float tx = ((x - dx) - x0 * h) * h1;
        int x1 = std::min(x0 + 1, numX - 1);

        int y0 = std::min((int)std::floor((y - dy) * h1), numY - 1);
        float ty = ((y - dy) - y0 * h) * h1;
        int y1 = std::min(y0 + 1, numY - 1);

        float sx = 1.0f - tx;
        float sy = 1.0f - ty;

        return sx * sy * (*f)[idx(x0, y0)] +
            tx * sy * (*f)[idx(x1, y0)] +
            tx * ty * (*f)[idx(x1, y1)] +
            sx * ty * (*f)[idx(x0, y1)];
    }

    inline void extrapolate() {
        for (int i = 0; i < numX; ++i) {
            u[idx(i, 0)] = u[idx(i, 1)];
            u[idx(i, numY - 1)] = u[idx(i, numY - 2)];
        }
        for (int j = 0; j < numY; ++j) {
            v[idx(0, j)] = v[idx(1, j)];
            v[idx(numX - 1, j)] = v[idx(numX - 2, j)];
        }
    }

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