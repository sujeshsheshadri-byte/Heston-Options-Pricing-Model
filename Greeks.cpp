#include "Greeks.h"
#include "HestonFDM.h"
#include <cmath>

RiskMetrics CalculateHestonGreeks(
    double S0, const PayOff& thePayOff, double r, double v0,
    double kappa, double theta, double sigma, double rho, double T,
    int Ns, int Nv, int Nt, double Smax, double vmax) {

    RiskMetrics greeks;

    // The base price (no bumps)
    double basePrice = HestonFDM_Explicit(S0, thePayOff, r, v0, kappa, theta, sigma, rho, T, Ns, Nv, Nt, Smax, vmax);

    // 1. DELTA & GAMMA (Bump Stock Price by 1%)
    double dS = S0 * 0.01; 
    double priceUpS = HestonFDM_Explicit(S0 + dS, thePayOff, r, v0, kappa, theta, sigma, rho, T, Ns, Nv, Nt, Smax, vmax);
    double priceDownS = HestonFDM_Explicit(S0 - dS, thePayOff, r, v0, kappa, theta, sigma, rho, T, Ns, Nv, Nt, Smax, vmax);
    
    greeks.Delta = (priceUpS - priceDownS) / (2.0 * dS);
    greeks.Gamma = (priceUpS - 2.0 * basePrice + priceDownS) / (dS * dS);

    // 2. VEGA (Bump Initial Variance by 1% absolute)
    // Note: Heston Vega is sensitivity to initial variance (v0), not constant volatility.
    double dv = 0.01; 
    double priceUpV = HestonFDM_Explicit(S0, thePayOff, r, v0 + dv, kappa, theta, sigma, rho, T, Ns, Nv, Nt, Smax, vmax);
    double priceDownV = HestonFDM_Explicit(S0, thePayOff, r, std::max(v0 - dv, 0.0001), kappa, theta, sigma, rho, T, Ns, Nv, Nt, Smax, vmax);
    
    greeks.Vega = (priceUpV - priceDownV) / (2.0 * dv);

    // 3. THETA (Decay over 1 day)
    // Theta is usually expressed as the value lost over 1 day. So we subtract 1 day from Maturity.
    double oneDay = 1.0 / 365.0;
    if (T > oneDay) {
        double priceTomorrow = HestonFDM_Explicit(S0, thePayOff, r, v0, kappa, theta, sigma, rho, T - oneDay, Ns, Nv, Nt, Smax, vmax);
        // Theta is (Price Tomorrow - Price Today) / 1 Day
        greeks.Theta = (priceTomorrow - basePrice) / oneDay;
    } else {
        greeks.Theta = 0.0; // Option expires today
    }

    // 4. RHO (Bump Risk-Free Rate by 1% absolute, i.e., 100 basis points)
    double dr = 0.01;
    double priceUpR = HestonFDM_Explicit(S0, thePayOff, r + dr, v0, kappa, theta, sigma, rho, T, Ns, Nv, Nt, Smax, vmax);
    double priceDownR = HestonFDM_Explicit(S0, thePayOff, r - dr, v0, kappa, theta, sigma, rho, T, Ns, Nv, Nt, Smax, vmax);
    
    greeks.Rho = (priceUpR - priceDownR) / (2.0 * dr);

    return greeks;
}