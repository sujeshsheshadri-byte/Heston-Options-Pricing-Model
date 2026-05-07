#include <iostream>
#include <fstream>
#include "PayOff.h"
#include "HestonMonteCarlo.h"
#include "HestonFDM.h"
#include "Greeks.h"
#include "BlackScholes.h"
#include "BinomialTree.h"
#include <cmath>
int main() {
    // 1. Declare the variables to hold Python's calibrated data
    double kappa, theta, sigma, rho, v0, S0, r;

    // 2. Read the calibrated parameters from Python
    std::ifstream paramFile("heston_params.txt");
    if (!paramFile.is_open()) {
        std::cerr << "ERROR: Run python calibrate_heston.py first!" << std::endl;
        return 1;
    }

    paramFile >> kappa >> theta >> sigma >> rho >> v0 >> S0 >> r;
    paramFile.close();

    std::cout << "--- Heston Parameters Loaded ---" << std::endl;
    std::cout << "Spot Price : $" << S0 << std::endl;
    std::cout << "Risk-Free  : " << r * 100 << "%" << std::endl;

    // 3. Define the Custom Option Contract
    double K = S0;           // At-The-Money Strike
    double T = 30.0 / 365.0; // 30 Days to expiration
    
    // Let's price an American Put Option this time to utilize the FDM's strength!
    PayOffPut putPayOff(K);

    // --- MONTE CARLO CONFIG ---
    int mcSteps = 30;      
    int mcPaths = 100000;  
    
    // --- FDM GRID CONFIG ---
    int Ns = 100;           // 100 Stock price nodes
    int Nv = 50;            // 50 Variance nodes
    int Nt = 5000;          // 5000 Time steps (High number required for Explicit stability!)
    double Smax = S0 * 2.0; // Grid goes up to double the current stock price
    double vmax = 1.0;      // Grid goes up to 100% variance
// Calculate Volatility for BSM/Binomial (Roughly sqrt of variance)
    double vol = std::sqrt(v0); 

    std::cout << "\nCalculating Prices across all Models..." << std::endl;
    
    double bsmPrice = bs_put_price(S0, K, r, vol, T);
    double treePrice = SimpleBinomialTree(S0, putPayOff, r, vol, T, 1000);
    double mcPrice = HestonMonteCarlo(S0, putPayOff, r, v0, kappa, theta, sigma, rho, T, mcSteps, mcPaths);
    double fdmPrice = HestonFDM_Explicit(S0, putPayOff, r, v0, kappa, theta, sigma, rho, T, Ns, Nv, Nt, Smax, vmax);

    std::cout << "\n=========================================" << std::endl;
    std::cout << "         PRICING ENGINE DASHBOARD        " << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "1. Black-Scholes (Analytical)  : $" << bsmPrice << std::endl;
    std::cout << "2. Binomial Tree (1000 Steps)  : $" << treePrice << std::endl;
    std::cout << "3. Heston Monte Carlo (100k)   : $" << mcPrice << std::endl;
    std::cout << "4. Heston FDM (American PDE)   : $" << fdmPrice << std::endl;
    std::cout << "=========================================" << std::endl;
    
    // --- RISK MANAGEMENT (THE GREEKS) ---
    std::cout << "\nCalculating Risk Profile (Greeks) via Finite Difference Bumping..." << std::endl;
    
    RiskMetrics myRisk = CalculateHestonGreeks(S0, putPayOff, r, v0, kappa, theta, sigma, rho, T, Ns, Nv, Nt, Smax, vmax);

    std::cout << "\n--- OPTION RISK PROFILE ---" << std::endl;
    std::cout << "Delta : " << myRisk.Delta << " (Directional Risk)" << std::endl;
    std::cout << "Gamma : " << myRisk.Gamma << " (Convexity Risk)" << std::endl;
    std::cout << "Vega  : " << myRisk.Vega  << " (Volatility Risk)" << std::endl;
    std::cout << "Theta : " << myRisk.Theta << " (Time Decay per year)" << std::endl;
    std::cout << "Rho   : " << myRisk.Rho   << " (Interest Rate Risk)" << std::endl;
    return 0;
}