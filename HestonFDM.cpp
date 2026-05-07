#include "HestonFDM.h"
#include <cmath>
#include <algorithm>
#include <iostream>

double HestonFDM_Explicit(
    double S0, const PayOff& thePayOff, double r, double v0,
    double kappa, double theta, double sigma, double rho, double T,
    int Ns, int Nv, int Nt, double Smax, double vmax) {

    // 1. Setup Grid Steps (dx, dy, dt)
    double dS = Smax / (Ns - 1);
    double dv = vmax / (Nv - 1);
    double dt = T / Nt;

    // 2. Allocate 2D memory arrays (grid = tomorrow, new_grid = today)
    std::vector<std::vector<double>> grid(Ns, std::vector<double>(Nv, 0.0));
    std::vector<std::vector<double>> new_grid(Ns, std::vector<double>(Nv, 0.0));

    // 3. Initialize Terminal Boundary Condition (At Expiration t=T)
    for (int s = 0; s < Ns; ++s) {
        for (int v = 0; v < Nv; ++v) {
            double S_val = s * dS;
            grid[s][v] = thePayOff(S_val); 
        }
    }

    // 4. Sweep Backwards in Time
    for (int t = 0; t < Nt; ++t) {
        
        // Loop over the internal nodes (ignoring the absolute edges to avoid array out-of-bounds)
        for (int s = 1; s < Ns - 1; ++s) {
            for (int v = 1; v < Nv - 1; ++v) {
                
                // State variables at this specific node
                double S = s * dS;
                double var = v * dv;
                double U = grid[s][v];

                // Calculate Discrete Derivatives (Central Difference)
                double dU_dS = (grid[s + 1][v] - grid[s - 1][v]) / (2.0 * dS);
                double dU_dv = (grid[s][v + 1] - grid[s][v - 1]) / (2.0 * dv);
                
                double d2U_dS2 = (grid[s + 1][v] - 2.0 * U + grid[s - 1][v]) / (dS * dS);
                double d2U_dv2 = (grid[s][v + 1] - 2.0 * U + grid[s][v - 1]) / (dv * dv);
                
                double d2U_dSdv = (grid[s + 1][v + 1] - grid[s + 1][v - 1] 
                                 - grid[s - 1][v + 1] + grid[s - 1][v - 1]) / (4.0 * dS * dv);

                // The Heston 2D PDE Operator
                double pde_operator = 0.5 * var * S * S * d2U_dS2 
                                    + rho * sigma * var * S * d2U_dSdv 
                                    + 0.5 * sigma * sigma * var * d2U_dv2 
                                    + r * S * dU_dS 
                                    + kappa * (theta - var) * dU_dv 
                                    - r * U;

                // Explicit Euler Time Step (Moving backwards means we ADD the operator)
                new_grid[s][v] = U + (dt * pde_operator);

                // THE AMERICAN OPTION CHECK (Early Exercise)
                double intrinsic = thePayOff(S);
                new_grid[s][v] = std::max(new_grid[s][v], intrinsic);
            }
        }

        // Apply simplistic boundary conditions for the edges
        for (int v = 0; v < Nv; ++v) {
            new_grid[0][v] = thePayOff(0.0); // S = 0 boundary
            new_grid[Ns - 1][v] = new_grid[Ns - 2][v] + (grid[Ns-1][v] - grid[Ns-2][v]); // Top boundary
        }
        for (int s = 0; s < Ns; ++s) {
            new_grid[s][0] = std::max(new_grid[s][1], thePayOff(s * dS)); // v = 0 boundary
            new_grid[s][Nv - 1] = new_grid[s][Nv - 2]; // v = max boundary
        }

        // Overwrite tomorrow's grid with today's newly calculated grid
        grid = new_grid;
    }

    // 5. Bilinear Interpolation
    // The exact S0 and v0 from the market probably don't sit perfectly on our grid nodes.
    // We find the 4 closest nodes and take a weighted average.
    int s_idx = S0 / dS;
    int v_idx = v0 / dv;

    // Safety check to ensure we are inside the grid
    if (s_idx >= Ns - 1) s_idx = Ns - 2;
    if (v_idx >= Nv - 1) v_idx = Nv - 2;

    double s_weight = (S0 - (s_idx * dS)) / dS;
    double v_weight = (v0 - (v_idx * dv)) / dv;

    double price = (1.0 - s_weight) * (1.0 - v_weight) * grid[s_idx][v_idx]
                 + s_weight * (1.0 - v_weight) * grid[s_idx + 1][v_idx]
                 + (1.0 - s_weight) * v_weight * grid[s_idx][v_idx + 1]
                 + s_weight * v_weight * grid[s_idx + 1][v_idx + 1];

    return price;
}