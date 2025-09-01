/**
 * @file main.cpp
 * @brief CSV-to-JSONL converter for trading OHLCV bar data.
 *
 * This program reads a CSV file containing Alpha Vantage style market data
 * in the format: timestamp, open, high, low, close, volume and converts
 * it into JSON Lines format, where each line is a JSON object.
 * 
 * The program reads each line of the CSV, stores it in
 * a OHLCV_Bar struct, converts it into a JSON object, and then writes
 * each JSON object as a separate line in a .jsonl file so that 
 * other programs later in the pipeline can easily process the data.
 *
 * The trading symbol is inferred from the input filename (e.g.
 * "intraday_1min_AAPL.csv" → "AAPL").
 * 
 * Example:
 * Input CSV:  data/intraday_1min_AAPL.csv
 * Output JSONL: output.jsonl
 */

#include <filesystem>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "json.hpp"  ///< nlohmann/json library for JSON serialization

using namespace std;
using json = nlohmann::json;

/**
 * @struct OHLCV_Bar
 * @brief Represents a single OHLCV bar data point.
 *
 * Fields:
 * - timestamp: the event timestamp in a string format
 * - symbol: trading instrument identifier
 * - open: opening price
 * - high: highest price
 * - low: lowest price
 * - close: closing price
 */

struct OHLCV_Bar {
    string timestamp;
    string symbol;
    double open, high, low, close;
    long long volume;
};


/**
 * @brief The symbol is extracted from the filename by taking the substring
 *        between the last underscore '_' and the last dot '.'.
 *
 * Example: "intraday_1min_AAPL.csv" → "AAPL"
 *
 * @param inputPath The CSV filename (not including directory which is set).
 * @return Extracted symbol if found, otherwise an empty string is produced.
 */
static string symbolFromFilename(const string& inputPath) {
    size_t underscore = inputPath.rfind('_');
    size_t dot        = inputPath.rfind('.');

    // Ensure both characters are found and in the correct order
    if (underscore != string::npos && dot != string::npos && underscore < dot) {
        return inputPath.substr(underscore + 1, dot - underscore - 1);
    }
    return ""; // Fallback in case nothing was extracted
}

int main(int argc, char* argv[]) {

    // argv[1] = input CSV filename (required, in "data/")
    // argv[2] = output JSONL filename (optional, written into "output/")

    if (argc < 2) {
        cerr << "You must provide the input CSV file path as a command line argument\n";
        return 1;
    }

    const string dataFolder = "data/";
    filesystem::create_directories("output"); // Ensure output directory exists
    const string outputFolder = "output/";
    const string fileName   = argv[1];

    // input file must have .csv extension (guard clause)
    if (fileName.rfind(".csv") == string::npos) {
        cerr << "Error: first argument must be a .csv file\n";
        return 1;
    }

    // Build full input/output paths
    const string inputPath     = dataFolder + fileName;
    const string outputPath    = (argc > 2) ? (outputFolder + string(argv[2])) : (outputFolder + "output.jsonl");
    
    // Extract trading symbol from filename (may be empty if pattern not found)
    const string detectedSymbol = symbolFromFilename(fileName);


    
    // Open CSV input file and JSONL output file
    ifstream csvInput(inputPath);
    ofstream jsonOutput(outputPath);

    // Validating file streams before continuing
    if (!csvInput.is_open() || !jsonOutput.is_open()) {
        cerr << "Error opening file!\n";
        return 1; // stop if either file cannot be opened
    }

    string csvLine;
    bool isFirstLine = true;
    // Process CSV file line by line
    while (getline(csvInput, csvLine)) {
        if (csvLine.empty()) continue;   // skip empty lines

        // skip the header row (the very first row)
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }   
        
        // Parse CSV row into individual string fields
        stringstream parser(csvLine);
        OHLCV_Bar dataRecord;
        string timestampStr, openStr, highStr, lowStr, closeStr, volumeStr;  

        getline(parser, timestampStr, ',');
        getline(parser, openStr,      ',');
        getline(parser, highStr,      ',');
        getline(parser, lowStr,       ',');
        getline(parser, closeStr,     ',');
        getline(parser, volumeStr,    ',');
        
        // timestamp and close are required, thus the check
        if (timestampStr.empty() || closeStr.empty()) continue;

        dataRecord.timestamp = timestampStr;
        dataRecord.symbol    = detectedSymbol;  // symbol we extracted from filename

        try {
            // Convert strings → numbers (type:double); skip row if conversion fails
            dataRecord.open   = stod(openStr);             
            dataRecord.high   = stod(highStr);
            dataRecord.low    = stod(lowStr);
            dataRecord.close  = stod(closeStr);
            dataRecord.volume = volumeStr.empty() ? 0 : stoll(volumeStr); // string → long long
        } catch (...) {
            // If conversion fails, skip this row
            continue;
        }

        // Serialize OHLCV_Bar struct into JSON object
        json dataJson;
        dataJson["timestamp"] = dataRecord.timestamp;
        dataJson["open"]      = dataRecord.open;
        dataJson["high"]      = dataRecord.high;
        dataJson["low"]       = dataRecord.low;
        dataJson["close"]     = dataRecord.close;
        dataJson["volume"]    = dataRecord.volume;
        if (!dataRecord.symbol.empty()) {
            dataJson["symbol"] = dataRecord.symbol;
        }

        // Write JSON object as a single line in the output file
        jsonOutput << dataJson.dump() << '\n';
    }
    // Close file streams
    csvInput.close();
    jsonOutput.close();

    // Final console message summary
    cout << "Conversion complete.\n"
         << "Input : " << inputPath  << '\n'
         << "Output: " << outputPath << '\n'
         << "Symbol: " << (detectedSymbol.empty() ? "<none>" : detectedSymbol) << '\n';
    
    // End of program
    return 0; // Success
}