#pragma once
#include "PayOff.h"
#include <vector>

// Solves the Heston PDE using Explicit Finite Difference for an American Option
double HestonFDM_Explicit(
    double S0, const PayOff& thePayOff, double r, double v0,
    double kappa, double theta, double sigma, double rho, double T,
    int Ns, int Nv, int Nt,    // Number of grid nodes (Stock, Variance, Time)
    double Smax, double vmax   // Maximum boundaries for the grid
);