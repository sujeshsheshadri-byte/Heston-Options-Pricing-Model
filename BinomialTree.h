#pragma once
#include "PayOff.h"

// Prices a European Option using the CRR Binomial Tree
double SimpleBinomialTree(double Spot, const PayOff& thePayOff, double r, double sigma, double T, int N);

// Prices an American Option using the CRR Binomial Tree with Early Exercise
double AmericanBinomialTree(double Spot, const PayOff& thePayOff, double r, double sigma, double T, int N);