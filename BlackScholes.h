#pragma once

// Calculates the Cumulative Standard Normal Distribution N(x)
double norm_cdf(const double& x);

// Calculates the Black-Scholes European Call Price
double bs_call_price(const double& S, const double& K, const double& r, const double& sigma, const double& T);

// Calculates the Black-Scholes European Put Price
double bs_put_price(const double& S, const double& K, const double& r, const double& sigma, const double& T);