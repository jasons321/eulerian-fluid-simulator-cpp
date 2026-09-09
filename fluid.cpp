#include "fluid.h"
#include <algorithm>
#include <cmath>

Fluid::Fluid(float density_, int numX_, int numY_, float h_)
    : density(density_),
      numX(numX_ + 2),
      numY(numY_ + 2),
      numCells(numX * numY),
      h(h_),
      u(numCells, 0.0f),
      v(numCells, 0.0f),
      newU(numCells, 0.0f),
      newV(numCells, 0.0f),
      p(numCells, 0.0f),
      s(numCells, 0.0f),
      // 1.0 = no smoke (rendered white); newM mirrors it so the first swap is a no-op
      m(numCells, 1.0f),
      newM(numCells, 1.0f)
{
}

void Fluid::integrate(float dt, float gravity) {
    for (int i = 1; i < numX; ++i) {
        for (int j = 1; j < numY; ++j) {
            if (s[idx(i, j)] != 0.0f && s[idx(i, j - 1)] != 0.0f) {
                v[idx(i, j)] += gravity * dt; // gravity
            }
        }
    }
}

void Fluid::solveIncompressibility(int iterations, float dt, float overRelaxation) {
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

float Fluid::sampleField(float x, float y, FieldType field) const {
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

void Fluid::extrapolate() {
    for (int i = 0; i < numX; ++i) {
        u[idx(i, 0)] = u[idx(i, 1)];
        u[idx(i, numY - 1)] = u[idx(i, numY - 2)];
    }
    for (int j = 0; j < numY; ++j) {
        v[idx(0, j)] = v[idx(1, j)];
        v[idx(numX - 1, j)] = v[idx(numX - 2, j)];
    }
}

float Fluid::avgV(int i, int j) {
    float vSum = (v[idx(i - 1, j)] + v[idx(i, j)] +
               v[idx(i - 1, j + 1)] + v[idx(i, j + 1)]) * 0.25f;
    return vSum;
}

float Fluid::avgU(int i, int j) {
    float uSum   = (u[idx(i - 1, j)] + u[idx(i, j)] +
               u[idx(i - 1, j + 1)] + u[idx(i, j + 1)]) * 0.25f;
    return uSum;
}

void Fluid::advectVel(float dt) {
    float h2 = 0.5f * h;

    for (int i = 1; i < numX; ++i) {
        for (int j = 1; j < numY; ++j) {
            if (s[idx(i, j)] != 0.0f && s[idx(i - 1, j)] != 0.0f && j < numY - 1) {
                float x = i * h;
                float y = j * h + h2;

                float currU = u[idx(i, j)];
                float currV = avgV(i, j);

                x = x - dt * currU;
                y = y - dt * currV;

                float sampledU = sampleField(x, y, FieldType::U);
                newU[idx(i, j)] = sampledU;
            }
            if (s[idx(i, j)] != 0.0f && s[idx(i, j - 1)] != 0.0f && i < numX - 1) {
                float x = i * h + h2;
                float y = j * h;

                float currU = avgU(i, j);
                float currV = v[idx(i, j)];

                x = x - dt * currU;
                y = y - dt * currV;

                newV[idx(i, j)] = sampleField(x, y, FieldType::V);
            }
        }
    }

    std::swap(u, newU);
    std::swap(v, newV);
}

void Fluid::advectSmoke(float dt) {
    float h2 = 0.5f * h;

    for (int i = 1; i < numX; ++i) {
        for (int j = 1; j < numY; ++j) {
            if (s[idx(i, j)] != 0.0f) {
                float x = i * h + h2;
                float y = j * h + h2;

                float currU = avgU(i, j);
                float currV = avgV(i, j);

                x = x - dt * currU;
                y = y - dt * currV;

                newM[idx(i, j)] = sampleField(x, y, FieldType::SMOKE);
            }
        }
    }

    std::swap(m, newM);
}

void Fluid::simulate(float dt, float gravity, int numIters, float overRelaxation) {
    integrate(dt, gravity);

    std::fill(p.begin(), p.end(), 0.0f);
    solveIncompressibility(numIters, dt, overRelaxation);

    extrapolate();
    advectVel(dt);
    advectSmoke(dt);
}