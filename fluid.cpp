#include "fluid.h"

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
      m(numCells /* TODO: what should smoke default to? think about what "no smoke yet" should look like on screen */),
      newM(numCells, 1.0f)
{
}