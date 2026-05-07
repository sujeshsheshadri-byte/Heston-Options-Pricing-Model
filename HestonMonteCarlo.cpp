#include "HestonMonteCarlo.h"
#include <cmath>
#include <random>
#include <algorithm>

double HestonMonteCarlo(double S0, const PayOff& thePayOff, double r,
                        double v0, double kappa, double theta,
                        double sigma, double rho, double T,
                        int numSteps, int numPaths) {
    
    double dt = T / numSteps;
    double sqrt_dt = std::sqrt(dt);

    // Set up the high-performance Mersenne Twister random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> standardNormal(0.0, 1.0);

    double runningSum = 0.0;

    // Loop 1: Simulate 100,000 parallel universes
    for (int i = 0; i < numPaths; ++i) {
        
        double S_t = S0;
        double v_t = v0;

        // Loop 2: Walk forward in time step-by-step
        for (int step = 0; step < numSteps; ++step) {
            
            // 1. Generate two INDEPENDENT random shocks
            double Z1 = standardNormal(gen);
            double Z2 = standardNormal(gen);

            // 2. Cholesky Decomposition: Create CORRELATED shocks
            double Z_S = Z1; 
            double Z_v = rho * Z1 + std::sqrt(1.0 - rho * rho) * Z2;

            // 3. Full Truncation: Prevent negative variance before taking square root
            double v_t_plus = std::max(v_t, 0.0);

            // 4. Euler-Maruyama Step for Stock Price (Using logarithmic form for stability)
            S_t = S_t * std::exp((r - 0.5 * v_t_plus) * dt + std::sqrt(v_t_plus) * sqrt_dt * Z_S);

            // 5. Euler-Maruyama Step for Variance (CIR Process)
            v_t = v_t + kappa * (theta - v_t_plus) * dt + sigma * std::sqrt(v_t_plus) * sqrt_dt * Z_v;
        }

        // Add the terminal payoff of this specific path to our total
        runningSum += thePayOff(S_t);
    }

    // Average the payoff and discount it back to today
    double expectedPayoff = runningSum / numPaths;
    double priceToday = expectedPayoff * std::exp(-r * T);

    return priceToday;
}