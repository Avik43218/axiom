#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class PreprocessRawData {
private:
    std::string rawDataFilePath;
    uint32_t scalingFactor = 1000000;

    using Row = std::vector<std::string>;

public:

    PreprocessRawData(std::string filePath) : rawDataFilePath(filePath) {}

    uint32_t normalizeScalarScore(uint32_t obtainedScore, uint32_t maxScore) {
        uint32_t normalizedScore = (obtainedScore / maxScore) * scalingFactor;
        return normalizedScore;
    }

    std::vector<Row> extractRows(rawDataFilePath) {
        std::vector<Row> allData;
        std::ifstream file(&rawDataFilePath);
        std::string line;

        if (!file.is_open()) return allData;

        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();

            std::vector<std::string> row;
            std::stringstream ss(line);
            std::string cell;

            while (std::getline(ss, cell, ',')) {
                row.push_back(cell);
            }

            allData.push_back(std::move(row));
        }

        return allData;
    }
    
};