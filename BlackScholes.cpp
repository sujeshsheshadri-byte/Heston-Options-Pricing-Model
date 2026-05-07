#include "BlackScholes.h"
#include <cmath> // This gives us std::log, std::sqrt, std::exp, and std::erf

// Standard Normal Cumulative Distribution Function
double norm_cdf(const double& x) {
    // We use the error function (erf) built into standard C++ math
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

double bs_call_price(const double& S, const double& K, const double& r, const double& sigma, const double& T) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    
    return S * norm_cdf(d1) - K * std::exp(-r * T) * norm_cdf(d2);
}

double bs_put_price(const double& S, const double& K, const double& r, const double& sigma, const double& T) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    
    return K * std::exp(-r * T) * norm_cdf(-d2) - S * norm_cdf(-d1);
}