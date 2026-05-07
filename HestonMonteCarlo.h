#pragma once
#include "PayOff.h"

// Prices a European Option using the Heston Stochastic Volatility Model via Monte Carlo
double HestonMonteCarlo(
    double S0,               // Current stock price
    const PayOff& thePayOff, // The option payoff object (Call, Put, Exotic)
    double r,                // Risk-free rate
    double v0,               // Initial variance
    double kappa,            // Mean reversion speed
    double theta,            // Long-term average variance
    double sigma,            // Volatility of volatility
    double rho,              // Correlation between stock and variance
    double T,                // Time to maturity (in years)
    int numSteps,            // Number of time steps per path (e.g., 252 for daily)
    int numPaths             // Number of simulated paths (e.g., 100,000)
);