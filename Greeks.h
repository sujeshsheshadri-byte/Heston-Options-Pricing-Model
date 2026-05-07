#pragma once
#include "PayOff.h"

// A simple struct to package our risk metrics together
struct RiskMetrics {
    double Delta;
    double Gamma;
    double Vega;
    double Theta;
    double Rho;
};

// Calculates the Greeks using Central Finite Difference Bumping
RiskMetrics CalculateHestonGreeks(
    double S0, const PayOff& thePayOff, double r, double v0,
    double kappa, double theta, double sigma, double rho, double T,
    int Ns, int Nv, int Nt, double Smax, double vmax
);