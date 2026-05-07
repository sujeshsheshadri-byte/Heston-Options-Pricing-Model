#pragma once

// 1. The Abstract Base Class
class PayOff {
public:
    PayOff() {} // Default constructor
    virtual ~PayOff() {} // Virtual destructors are mandatory for C++ base classes!

    // The core function: Overloading the "()" operator makes this class a "Functor"
    // The "= 0" means it's a "pure virtual" function. Any derived class MUST implement this.
    virtual double operator()(double Spot) const = 0; 
};

// 2. The Derived Class for Call Options
class PayOffCall : public PayOff {
private:
    double K; // The Strike Price
public:
    PayOffCall(double Strike); // Constructor
    virtual ~PayOffCall() {}
    virtual double operator()(double Spot) const; // Implementation of the payoff
};

// 3. The Derived Class for Put Options
class PayOffPut : public PayOff {
private:
    double K;
public:
    PayOffPut(double Strike);
    virtual ~PayOffPut() {}
    virtual double operator()(double Spot) const;
};
