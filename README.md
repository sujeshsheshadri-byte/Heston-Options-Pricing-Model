# Quantitative Options Pricing Engine

A hybrid Python + C++ pipeline for pricing European and American options using stochastic volatility models and live market data.

---

## 🧠 Models Implemented

- Black-Scholes-Merton (BSM)
- Binomial Tree (CRR)
- Heston Monte Carlo Simulation
- Heston Finite Difference Method (FDM)

Also computes:
- Delta
- Gamma
- Vega
- Theta
- Rho

using Central Finite Difference methods.

---

## ⚙️ Architecture

- **Python:** Live options data scraping + Heston parameter calibration
- **C++:** High-performance numerical pricing engine

---

# 🚀 Quick Start

## Clone Repository

```bash
git clone https://github.com/sujeshsheshadri-byte/Heston-Options-Pricing-Model.git
cd Heston-Options-Pricing-Model
```

## Install Dependencies

```bash
pip install numpy scipy yfinance
```

## Compile C++ Engine

```bash
g++ main.cpp PayOff.cpp BlackScholes.cpp BinomialTree.cpp HestonMonteCarlo.cpp HestonFDM.cpp Greeks.cpp -o pricer
```

## Run Pipeline

```bash
python app.py
```

Enter a stock ticker like:

```text
AAPL
TSLA
NVDA
```

The pipeline automatically:
- Pulls live market data
- Calibrates Heston parameters
- Prices options
- Computes Greeks
