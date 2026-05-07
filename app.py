import subprocess
import time
import os
import sys  # REQUIRED: This lets us find the correct Python path

def print_header():
    print("\n" + "="*50)
    print("   QUANTITATIVE DERIVATIVES PRICING PLATFORM")
    print("   Architecture: Python (Data) + C++ (Compute)")
    print("="*50)

def run_pipeline():
    print_header()
    
    # 1. Get User Input
    ticker = input("\nEnter Ticker Symbol (e.g., AAPL, SPY, TSLA): ").upper().strip()
    if not ticker:
        ticker = "AAPL"
        
    print(f"\n>>> [1/3] Initializing Data Pipeline for {ticker}...")
    time.sleep(1)
    
    # 2. Run Python Calibrator
    print(">>> [2/3] Fetching live options chain and calibrating Heston SDE...")
    try:
        # FIX: We use sys.executable instead of the hardcoded "python"
        subprocess.run([sys.executable, "calibrate_heston.py", ticker], check=True)
    except subprocess.CalledProcessError:
        print("\n[ERROR] Calibration failed. Check your internet or ticker symbol.")
        return

    # 3. Run C++ Engine
    print("\n>>> [3/3] Passing calibrated reality to C++ Execution Engine...")
    time.sleep(1)
    
    executable = "./pricer" if os.name != 'nt' else "pricer.exe"
    
    try:
        subprocess.run([executable], check=True)
    except subprocess.CalledProcessError:
        print("\n[ERROR] C++ Engine failed to execute. Did you compile it?")
        return
        
    print("\n[SUCCESS] Pipeline Execution Complete.")

if __name__ == "__main__":
    run_pipeline()