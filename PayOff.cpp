#include "PayOff.h"
#include <algorithm> // This gives us std::max()

// --- PayOffCall Implementation ---

// Constructor: Initializes the private variable K
PayOffCall::PayOffCall(double Strike) : K(Strike) {
}

// The Functor: Calculates max(S - K, 0)
double PayOffCall::operator()(double Spot) const {
    return std::max(Spot - K, 0.0); 
}

// --- PayOffPut Implementation ---

PayOffPut::PayOffPut(double Strike) : K(Strike) {
}

// The Functor: Calculates max(K - S, 0)
double PayOffPut::operator()(double Spot) const {
    return std::max(K - Spot, 0.0);
}