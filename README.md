# Market Data Pipeline

## Quickstart (Run the Full Pipeline)

Run these commands from the project root. All compiled files go into `bin/`, and all pipeline outputs go into `output/`.

```bash

# Step 1: Compile & Run C++ (produce JSONL)
g++ -std=c++17 C++/Market_Data_Producer.cpp -o bin/Market_Data_Producer.exe

./bin/Market_Data_Producer.exe intraday_1min_AAPL.csv

# Step 2: Compile & Run Java (process JSONL -> CSV with signals)
javac -cp ".;Java/gson-2.13.1.jar" -d bin Java/Tick.java Java/MarketDataProcessor.java

java -cp ".;bin;Java/gson-2.13.1.jar" MarketDataProcessor output/output.jsonl output/tradingSignals.csv 20

# Step 3: Run Python (visualise signals -> charts)
python Python/data_viewer.py --in output/tradingSignals.csv --out output

```

## About this Project
Through this project, I demonstrate my ability to apply **C++**, **Java**, and **Python** fundamentals in building a simplified quantitative trading pipeline.

It shows that I can:

• Work across multiple languages in a single system.  

• Handle **data ingestion (C++)**, **signal processing (Java)**, and **visualisation (Python)**.  

• Write clean and well-structured, documented code that is easy to run and extend.  

I built this to showcase my interest and development skills relevant to **internships, year-in-industry placements, and graduate roles** in software engineering, quantitative data science, and fintech.

---

## Dependencies

• **C++17 or newer** 

• **Java 11+** (with `javac` and `java`)  

• **Python 3.9+** with:
  - `pandas`
  - `numpy`
  - `matplotlib`

• **nlohmann/json** single-header library (`json.hpp`) → included in `C++/` 
• **gson-2.13.1.jar** → included in `Java/`

All required third-party libraries are already included in this repo (`json.hpp` and `gson-2.13.1.jar`).

---

## Overview
This project is a simplistic quant trading pipeline spanning three languages:

1. **C++ (Producer)**  
   Reads Alpha Vantage–style OHLCV CSV bar data, infers the symbol from the filename, and outputs JSON Lines.

2. **Java (Processor)**  
   Reads JSONL, computes rolling statistics, generates z-scores, and writes trading signals into a CSV.

3. **Python (Viewer)**  
   Reads the CSV, computes summary statistics, and produces visualisations of prices and signals.

---

## Data Format

Input CSVs must look like:

```
timestamp,open,high,low,close,volume
2025-02-28 09:30:00,175.12,176.00,174.95,175.90,123456
```

File names should follow the convention:  
`anything_SYMBOL.csv` → e.g. `intraday_1min_AAPL.csv` gives symbol `AAPL`.


---

## Folder Structure

```
project-root/
├── C++/                 # C++ source (producer)
├── Java/                # Java source (processor)
├── Python/              # Python source (viewer)
├── bin/                 # Compiled executables & class files
├── data/                # Input CSVs
├── output/              # All pipeline outputs (JSONL, CSV, charts)
├── README.md
├── .gitignore
└── .gitattributes
```

---

### Input/Output Flow

• **C++ (Producer)**  
  - Input: `data/<filename>.csv` (Alpha Vantage–style OHLCV CSV)  
  - Output: `output/output.jsonl` (or a custom name if passed as a second argument)

• **Java (Processor)**  
  - Input: `output/output.jsonl`  
  - Output: `output/tradingSignals.csv` (or a custom name if passed as the second argument)

• **Python (Viewer)**  
  - Input: `output/tradingSignals.csv`  
  - Output: charts saved in `output/`:
    - `price.png`  
    - `price_vs_mean.png`  
    - `zscore.png`

---


## Usage

### 1. Compile & Run C++ Producer

**Compile (into bin/):**
```bash
g++ -std=c++17 C++/Market_Data_Producer.cpp -o bin/Market_Data_Producer.exe
```

**Run (from project root):**
```bash
./bin/Market_Data_Producer.exe intraday_1min_AAPL.csv
```

- Reads: `data/intraday_1min_AAPL.csv`  
- Writes: `output/output.jsonl` (or a custom filename if you pass one)

---

### 2. Compile & Run Java Processor

**Compile (into bin/):**
```bash
javac -cp ".;Java/gson-2.13.1.jar" -d bin Java/Tick.java Java/MarketDataProcessor.java
```

**Run (from project root):**
```bash
java -cp ".;bin;Java/gson-2.13.1.jar" MarketDataProcessor output/output.jsonl output/tradingSignals.csv 20
```

- Input: `output/output.jsonl`  
- Output: `output/tradingSignals.csv` (custom name allowed if you pass your own as the second argument)  
- `20` = optional rolling window size (default = 20 if omitted)

---

### 3. Run Python Viewer

**Run (from project root):**
```bash
python Python/data_viewer.py --in output/tradingSignals.csv --out output
```

- Reads: `output/tradingSignals.csv`  
- Writes charts:
  - `output/price.png`  
  - `output/price_vs_mean.png`  
  - `output/zscore.png`

---

## Notes & Constraints

- Input CSVs must have header row: `timestamp,open,high,low,close,volume`.  
- Filename convention (`*_SYMBOL.csv`) is required for the symbol field to be populated.  
- Java output CSV schema is fixed:
  ```
  timestamp,symbol,close,mean,std,zscore,signal
  ```
- Python viewer validates the CSV schema and will stop with a clear error if any required column is missing.

---

## License
MIT License (see LICENSE file).







