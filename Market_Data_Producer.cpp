#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include "json.hpp"


using namespace std;
using json = nlohmann::json;

struct Tick {
    string timestamp;
    string symbol;
    double price;
};

int main() {

    ifstream csvInput("data/sample_ticks.csv");
    ofstream jsonOutput("data/ticks.jsonl");

    if (!csvInput.is_open() || !jsonOutput.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    string line;
    while (getline(csvInput, line)) {
        stringstream parser(line);

        Tick tickRecord;
        string priceStr;

        getline(parser, tickRecord.timestamp, ',');
        getline(parser, tickRecord.symbol, ',');
        getline(parser, priceStr, ',');

        tickRecord.price = stod(priceStr);

        json tickJson;
        tickJson["timestamp"] = tickRecord.timestamp;
        tickJson["symbol"] = tickRecord.symbol;
        tickJson["price"] = tickRecord.price;

        jsonOutput << tickJson.dump() << endl;
    }

    csvInput.close();
    jsonOutput.close();

    cout << "Conversion complete. Output written to ticks.jsonl" << endl;
    return 0;
}