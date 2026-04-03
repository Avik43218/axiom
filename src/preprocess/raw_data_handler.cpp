#include <string>
#include <vector>
#include <fstream>
#include <mysql-cppconn/mysqlx/xdevapi.h>

class RawDataHandler {
private:
    const std::string& filename;

    struct ResultRow {
        std::string firstColumn;
        std::vector<double> rawScores;
    };

public:
    struct DBConfig {
        const char *db_host, *db_user, *db_name;
        const int port;
    };

    RawDataHandler(const std::string& filePath) : filename(filePath) {}

    std::vector<ResultRow> extractRows(const std::vector<std::string>& selectedHeaders) {

        std::vector<ResultRow> results;
        std::ifstream file(filename);
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
                result.rawScores.push_back(obtained);
            }

            results.push_back(std::move(result));
        }

        return results;
    }
};
