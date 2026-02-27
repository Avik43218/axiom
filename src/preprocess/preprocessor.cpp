#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <cstdlib>
#include <mysql-cppconn/mysqlx/xdevapi.h>

class PreprocessRawData {
private:
    const std::string& filename;
    uint32_t scalingFactor = 1000000;

    struct ResultRow {
        std::string firstColumn;
        std::vector<uint32_t> normalizedScores;
    };

    struct DBConfig {
        const char *db_host, *db_user, *db_name;
        const int port;
    };

public:

    PreprocessRawData(std::string filePath) : filename(filePath) {}

    uint32_t normalizeScalarScore(double obtainedScore, double maxScore) {
        if (obtainedScore != obtainedScore || maxScore != maxScore) {
            return 0;
        }
        if (maxScore == 0 || obtainedScore > maxScore) {
            std::cerr << "Invalid scores: obtained = " << obtainedScore << ", max = " << maxScore << std::endl;
            return 0;
        }

        double normalized = (obtainedScore / maxScore) * scalingFactor;
        return static_cast<uint32_t>(normalized);
    }

    std::vector<ResultRow> extractRows(const std::string& filename, const std::vector<std::string>& selectedHeaders) {

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
                uint32_t maximum = maximumScores[i];
                result.normalizedScores.push_back(normalizeScalarScore(obtained, maximum));
            }

            results.push_back(std::move(result));
        }

        return results;
    }

    DBConfig parseIni(const std::string& filename) {

        static std::string host, user, passwd, name;
        int port = 3306; // default fallback

        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            std::string key, value;

            if (std::getline(iss, key, '=') && std::getline(iss, value)) {

                if (key == "DB_HOST") host = value;
                else if (key == "DB_USER") user = value;
                else if (key == "DB_NAME") name = value;
                else if (key == "DB_PORT") port = std::stoi(value);
            }
        }

        return DBConfig{
            host.c_str(),
            user.c_str(),
            name.c_str(),
            port
        };
    }

    void createRecordsTable(mysqlx::Session& sess, const std::vector<std::string>& selectedHeaders, std::string tableName) {

        std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + "(";
        query += "student_id VARCHAR(7) PRIMARY KEY";
        for (size_t i = 0; i < selectedHeaders.size(); ++i) {
            query += selectedHeaders[i] + " INT UNSIGNED";
            if (i != selectedHeaders.size() - 1) {
                query += ", ";
            }
        }
        query += ")";

        sess.sql(query).execute();
        std::cout << "Created table: " << tableName << std::endl;
    }

    int insertData(mysqlx::Session sess, const std::string& filename, const std::vector<std::string>& selectedHeaders, std::string tableName) {

        DBConfig config = parseIni(filename);
        std::vector<ResultRow> cleanData = extractRows(filename, selectedHeaders);

        try {
            mysqlx::Schema schema = sess.getSchema(config.db_name);

            // Check if table exists or not before inserting data
            std::vector<mysqlx::Table> tables = schema.getTables();
            bool exists = false;

            for (auto &t : tables) {
                if (t.getName() == tableName) {
                    exists = true;
                    break;
                }
            }

            if (exists) {
                // Data entry code goes here
            }
            else {
                std::cerr << "Error: Table " << tableName << " does not exist" << std::endl;
                return 1;
            }
        }
        catch (const mysqlx::Error &err) {
            std::cerr << "Error: " << err.what() << std::endl;
            return 1;
        }

        return 0;
    }

};
