#include <memory>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <exception>
#include <unordered_map>
#include <mysql-cppconn/mysqlx/xdevapi.h>

class PreprocessRawData {
private:
    const std::string& filename;
    const uint32_t scalingFactor = 1000000;

    struct ResultRow {
        std::string firstColumn;
        std::vector<uint32_t> normalizedScores;
    };

public:
    struct DBConfig {
        const char *db_host, *db_user, *db_name;
        const int port;
    };

    PreprocessRawData(const std::string& filePath) : filename(filePath) {}

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
                uint32_t maximum = maximumScores[i];
                result.normalizedScores.push_back(normalizeScalarScore(obtained, maximum));
            }

            results.push_back(std::move(result));
        }

        return results;
    }

    DBConfig parseIni(const std::string& configFile) {

        static std::string host, user, passwd, name;
        int port = 3306; // default fallback

        std::ifstream file(configFile);
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

    bool createRecordsTable(mysqlx::Session& sess, const std::vector<std::string>& selectedHeaders, const std::string tableName) {

        try {
            std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + "(";
            query += "student_id VARCHAR(7) PRIMARY KEY, ";
            for (size_t i = 0; i < selectedHeaders.size(); ++i) {
                query += selectedHeaders[i] + " INT UNSIGNED";
                if (i != selectedHeaders.size() - 1) {
                    query += ", ";
                }
            }
            query += ")";

            sess.sql(query).execute();
            std::cout << "Created table: " << tableName << std::endl;

            return true;
        }
        catch (const std::exception &e) {
            return false;
        }

    }

    int insertData(mysqlx::Session& sess, const std::string& configFile, const std::vector<std::string>& selectedHeaders, const std::string tableName) {

        DBConfig config = parseIni(configFile);
        std::vector<ResultRow> cleanedData = extractRows(selectedHeaders);

        try {
            mysqlx::Schema schema = sess.getSchema(config.db_name);

            // Check if table exists or not before inserting data
            bool exists = false;
            if (schema.getTable(tableName).existsInDatabase()) exists = true;

            if (exists) {
                std::vector<std::string> headers;
                headers.push_back("student_id");

                for (auto &h : selectedHeaders) {
                    headers.push_back(h);
                }

                auto table = schema.getTable(tableName);
                auto ins = table.insert(headers);

                for (const auto& row : cleanedData) {

                    std::vector<mysqlx::Value> values;
                    values.push_back(row.firstColumn);

                    for (uint32_t score : row.normalizedScores) {
                        values.push_back(score);
                    }

                    ins.values(values);
                }

                ins.execute();
                std::cout << "Data insertion successful into " << tableName << std::endl;

            }
            else {
                throw std::runtime_error("Table " + tableName + " does not exist");
            }
        }
        catch (const mysqlx::Error &err) {
            std::cerr << "Error: " << err.what() << std::endl;
            return 1;
        }
        catch (const std::runtime_error &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }

        return 0;
    }

};


std::vector<std::string> returnSelectedHeaders(const std::string& headersFile) {

    std::ifstream file(headersFile);
    std::string line;

    std::vector<std::string> selectedHeaders;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        selectedHeaders.push_back(line);
    }

    return selectedHeaders;
}

int main(int argc, char* argv[]) {

    if (argc != 6) {
        std::cerr << "Error: 6 arguments arequired, " << argc << " provided" << std::endl;
        return 1;
    }

    std::string dbPasswd = argv[1];
    std::string tableName = argv[2];
    std::string csvFilePath = argv[3];
    std::string configFilePath = argv[4];
    std::string headersFilePath = argv[5];

    PreprocessRawData engine(csvFilePath);

    PreprocessRawData::DBConfig config = engine.parseIni(configFilePath);

    std::unique_ptr<mysqlx::Session> session;

    try {
        session = std::make_unique<mysqlx::Session>(config.db_host, config.port, config.db_user, dbPasswd);
        mysqlx::Schema schema = session->getSchema(config.db_name);

        if (!schema.existsInDatabase()) {
            std::string errMsg = "Schema " + static_cast<std::string>(config.db_name) + " does not exist";
            throw std::runtime_error(errMsg);
        }
    }
    catch (const mysqlx::Error &err) {
        std::cerr << "Error: " << err.what() << std::endl;
        return 1;
    }
    catch (const std::runtime_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::vector<std::string> selectedHeaders = returnSelectedHeaders(headersFilePath);
    bool isTableCreated = engine.createRecordsTable(*session, selectedHeaders, tableName);

    if (isTableCreated) {

        int insertionResult = engine.insertData(*session, configFilePath, selectedHeaders, tableName);
        if (insertionResult == 0) {
            return 0;
        }

        else {
            std::cerr << "Data insertion failed" << std::endl;
            return 1;
        }
    }
}
