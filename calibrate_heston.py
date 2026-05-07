import yfinance as yf
import numpy as np
import pandas as pd
from scipy.integrate import quad
from scipy.optimize import minimize
import warnings
import sys
warnings.filterwarnings("ignore")

# =====================================================================
# 1. THE HESTON MATH (Semi-Analytical Pricer for Calibration)
# =====================================================================
def heston_characteristic_function(phi, S0, K, T, r, kappa, theta, sigma, rho, v0):
    """Calculates the complex characteristic function for Heston."""
    # Complex number setup
    i = complex(0, 1)
    
    # Heston parameters for the ODE
    a = kappa * theta
    u = -0.5
    b = kappa + rho * sigma * i * phi
    
    d = np.sqrt(b**2 - sigma**2 * (2 * u * i * phi - phi**2))
    g = (b - d) / (b + d)
    
    # Calculate C and D
    C = r * i * phi * T + (a / sigma**2) * ((b - d) * T - 2 * np.log((1 - g * np.exp(-d * T)) / (1 - g)))
    D = ((b - d) / sigma**2) * ((1 - np.exp(-d * T)) / (1 - g * np.exp(-d * T)))
    
    return np.exp(C + D * v0 + i * phi * np.log(S0))

def heston_integrand(phi, S0, K, T, r, kappa, theta, sigma, rho, v0):
    """The integrand used to find the probability of finishing in the money."""
    i = complex(0, 1)
    cf = heston_characteristic_function(phi - i, S0, K, T, r, kappa, theta, sigma, rho, v0)
    cf_denom = heston_characteristic_function(phi, S0, K, T, r, kappa, theta, sigma, rho, v0)
    
    numerator = np.exp(-i * phi * np.log(K)) * cf
    denominator = i * phi * cf_denom
    
    return (numerator / denominator).real

def heston_call_price(S0, K, T, r, kappa, theta, sigma, rho, v0):
    """Prices a European Call using numerical integration."""
    # We integrate from 0 to 100 (infinity approximation)
    integral, error = quad(heston_integrand, 0, 100, args=(S0, K, T, r, kappa, theta, sigma, rho, v0))
    
    P1 = 0.5 + (1 / np.pi) * integral
    
    # Calculate P2 (using standard characteristic function)
    integral2, error2 = quad(lambda phi: (np.exp(-complex(0,1) * phi * np.log(K)) * 
                                          heston_characteristic_function(phi, S0, K, T, r, kappa, theta, sigma, rho, v0) / 
                                          (complex(0,1) * phi)).real, 0, 100)
    P2 = 0.5 + (1 / np.pi) * integral2
    
    return S0 * P1 - K * np.exp(-r * T) * P2

# =====================================================================
# 2. THE OPTIMIZATION ENGINE (Minimizing the Error)
# =====================================================================
def objective_function(params, market_data, S0, r):
    """Calculates the Mean Squared Error between Market Prices and Heston Prices."""
    kappa, theta, sigma, rho, v0 = params
    
    # Penalize the optimizer if it breaks the Feller Condition (2 * kappa * theta > sigma^2)
    # The Feller condition ensures variance never drops below zero.
    if 2 * kappa * theta <= sigma**2:
        return 1e6 
        
    error = 0.0
    for index, row in market_data.iterrows():
        K = row['Strike']
        T = row['Time_to_Maturity']
        market_price = row['Market_Price']
        
        # Calculate model price
        model_price = heston_call_price(S0, K, T, r, kappa, theta, sigma, rho, v0)
        
        # Sum of Squared Errors (SSE)
        error += (model_price - market_price)**2
        
    return error / len(market_data)

# =====================================================================
# 3. THE DATA PIPELINE (Fetching & Execution)
# =====================================================================
def calibrate_to_market(ticker_symbol):
    print(f"Fetching live market data for {ticker_symbol}...")
    ticker = yf.Ticker(ticker_symbol)
    S0 = ticker.history(period="1d")['Close'].iloc[-1]
    
    # Get the nearest expiration date
    expirations = ticker.options
    target_exp = expirations[2] # Pick the 3rd expiration date to ensure liquid data
    
    print(f"Target Expiration Date: {target_exp}")
    opt_chain = ticker.option_chain(target_exp)
    calls = opt_chain.calls
    
    # Clean the data: Filter for Out-of-the-Money and At-the-Money calls with volume
    calls = calls[(calls['strike'] >= S0 * 0.9) & (calls['strike'] <= S0 * 1.1) & (calls['volume'] > 0)]
    
    # Time to maturity in years
    days_to_expiry = (pd.to_datetime(target_exp) - pd.Timestamp.today()).days
    T = days_to_expiry / 365.0
    r = 0.04 # Assume 4% risk-free rate
    
    # Format the data for the optimizer
    market_data = pd.DataFrame({
        'Strike': calls['strike'],
        'Time_to_Maturity': T,
        'Market_Price': (calls['bid'] + calls['ask']) / 2.0 # Use mid-price
    })
    
    print(f"Found {len(market_data)} valid option contracts for calibration.")
    
    # Initial Guesses for Heston: [kappa, theta, sigma, rho, v0]
    initial_guess = [2.0, 0.04, 0.1, -0.5, 0.04]
    
    # Bounds for the parameters to keep them mathematically valid
    bounds = (
        (0.01, 10.0),   # kappa (mean reversion speed)
        (0.01, 1.0),    # theta (long term variance)
        (0.01, 1.0),    # sigma (vol of vol)
        (-0.99, 0.99),  # rho (correlation)
        (0.01, 1.0)     # v0 (initial variance)
    )
    
    print("Running optimization algorithm (this may take 10-30 seconds)...")
    result = minimize(objective_function, initial_guess, args=(market_data, S0, r), 
                      method='SLSQP', bounds=bounds, tol=1e-6)
    
    kappa, theta, sigma, rho, v0 = result.x
    
    print("\n--- HESTON CALIBRATION SUCCESSFUL ---")
    print(f"Kappa (Mean Reversion) : {kappa:.4f}")
    print(f"Theta (Long Term Var)  : {theta:.4f}")
    print(f"Sigma (Vol of Vol)     : {sigma:.4f}")
    print(f"Rho   (Correlation)    : {rho:.4f}")
    print(f"v0    (Initial Var)    : {v0:.4f}")
    
    # Export to C++
    with open("heston_params.txt", "w") as f:
        f.write(f"{kappa}\n{theta}\n{sigma}\n{rho}\n{v0}\n{S0}\n{r}\n")
    print("\nMarket reality exported to heston_params.txt for C++ Engine.")

if __name__ == "__main__":
    # If the user provides a ticker in the terminal, use it. Otherwise default to AAPL.
    ticker_symbol = sys.argv[1] if len(sys.argv) > 1 else "AAPL"
    calibrate_to_market(ticker_symbol)