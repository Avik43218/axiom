#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>

class PreprocessRawData {
private:
    std::string rawDataFilePath;
    uint32_t scalingFactor = 1000000;

    struct ResultRow {
        std::string firstColumn;
        std::vector<uint32_t> normalizedScores;
    };

public:

    PreprocessRawData(std::string filePath) : rawDataFilePath(filePath) {}

    static uint32_t normalizeScalarScore(uint32_t obtainedScore, uint32_t maxScore) {
        if (obtainedScore != obtainedScore) {
            return 0;
        }
        if ((maxScore != 0 && maxScore == maxScore) && (obtainedScore <= maxScore)) {
            uint32_t normalizedScore = (static_cast<double>(obtainedScore) / maxScore) * scalingFactor;
        }
        else {
            std::cerr << "Obtained score <= Maximum score (!= 0)" << std::endl;
        }

        return static_cast<uint32_t>(normalizedScore);
    }

    std::vector<ResultRow> extractRows(
        rawDataFilePath, const std::vector<std::string>& selectedHeaders,
        std::function<uint32_t(uint32_t)> normalizationFn
    ) {
        std::vector<ResultRow> results;
        std::ifstream file(&rawDataFilePath);
        std::string line;

        if (!file.is_open()) return results;

        // Read header row
        std::getline(file, line);
        std::stringstream headerStream(line);
        std::string header;

        std::unordered_map<std::string, size_t> headerIndex;
        size_t index = 0;

        while (std::getline(headerStream, header, ',')) {
            headerIndex[header] = index++;
        }

        // Map selected headers to column indices
        std::vector<size_t> selectedIndices;
        for (const auto& h : selectedHeaders) {
            if (headerIndex.count(h)) {
                selectedIndices.push_back(headerIndex[h]);
            }
        }

        // Read second row for maximum scores
        std::getline(file, line);
        std::stringstream maxScoresStream(line);
        std::string maxScoreString;
        std::vector<std::string> metaRow;
        std::vector<uint32_t> maximumScores;

        while (std::getline(maxScoresStream, maxScoreString, ',')) {
            metaRow.push_back(maxScoreString);
        }

        for (size_t j : selectedIndices) {
            if (j == 0 || j >= metaRow.size()) continue;

            uint32_t individualMaxScore = static_cast<uint32_t>(std::stoul(metaRow[j]));
            maximumScores.push_back(individualMaxScore);
        }

        // Read data rows
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();

            std::stringstream ss(line);
            std::string cell;
            std::vector<std::string> row;

            while (std::getline(ss, cell, ',')) {
                row.push_back(cell);
            }

            if (row.empty()) continue;

            ResultRow result;
            result.firstColumn = row[0];

            for (size_t i : selectedIndices) {
                if (i == 0 || i >= row.size()) continue;

                uint32_t obtained = static_cast<uint32_t>(std::stoul(row[i]));
                uint32_t maximum = maximumScores[i];
                result.normalizedScores.push_back(normalizationFn(obtained, maximum));
            }

            results.push_back(std::move(result));
        }

        return results;
    }
};
