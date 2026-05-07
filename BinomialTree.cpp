#include "BinomialTree.h"
#include <cmath>
#include <vector>

double SimpleBinomialTree(double Spot, const PayOff& thePayOff, double r, double sigma, double T, int N) {
    
    // Calculate the CRR parameters
    double dt = T / N;
    double u = std::exp(sigma * std::sqrt(dt));
    double d = 1.0 / u;
    double p = (std::exp(r * dt) - d) / (u - d);
    double discount = std::exp(-r * dt);

    // We use a std::vector to store the option values.
    // It dynamically sizes to N+1 (since 0 steps means 1 node, 1 step means 2 nodes, etc.)
    std::vector<double> values(N + 1);

    // Step 1: Calculate asset prices at expiration (time step N) and their payoffs
    for (int i = 0; i <= N; ++i) {
        // i represents the number of UP steps. (N - i) is the number of DOWN steps.
        double S_T = Spot * std::pow(u, i) * std::pow(d, N - i);
        
        // POLYMORPHISM IN ACTION! 
        // We don't care if this is a Call or Put. The object calculates it for us.
        values[i] = thePayOff(S_T); 
    }

    // Step 2: Backward Induction through the tree
    for (int step = N - 1; step >= 0; --step) {
        for (int i = 0; i <= step; ++i) {
            // Expected value discounted back one time step
            values[i] = discount * (p * values[i + 1] + (1.0 - p) * values[i]);
        }
    }

    // The value at the base of the tree is today's fair option price
    return values[0];
}

double AmericanBinomialTree(double Spot, const PayOff& thePayOff, double r, double sigma, double T, int N) {
    
    double dt = T / N;
    double u = std::exp(sigma * std::sqrt(dt));
    double d = 1.0 / u;
    double p = (std::exp(r * dt) - d) / (u - d);
    double discount = std::exp(-r * dt);

    std::vector<double> values(N + 1);

    // Step 1: Terminal Payoffs (Exactly the same as European)
    for (int i = 0; i <= N; ++i) {
        double S_T = Spot * std::pow(u, i) * std::pow(d, N - i);
        values[i] = thePayOff(S_T); 
    }

    // Step 2: Backward Induction (WITH EARLY EXERCISE CHECK)
    for (int step = N - 1; step >= 0; --step) {
        for (int i = 0; i <= step; ++i) {
            
            // Choice A: Hold the option (Discounted expected value)
            double holdValue = discount * (p * values[i + 1] + (1.0 - p) * values[i]);

            // Choice B: Exercise the option right now
            // To do this, we must calculate the stock price at this specific node
            // i = number of UP moves. (step - i) = number of DOWN moves so far.
            double S_t = Spot * std::pow(u, i) * std::pow(d, step - i);
            double exerciseValue = thePayOff(S_t);

            // The value of the node is the MAX of holding or exercising
            values[i] = std::max(holdValue, exerciseValue);
        }
    }

    return values[0];
}