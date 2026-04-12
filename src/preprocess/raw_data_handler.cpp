#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <mysql-cppconn/mysqlx/xdevapi.h>

class RawDataHandler {
private:
    const std::string& filename;

    struct ResultRow {
        std::string firstColumn;
        std::vector<double> S_rawScores;
        std::vector<uint32_t> S_maxScores;
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
        ResultRow result;

        while (std::getline(maxScoresStream, maxScoreString, ',')) {
            metaRow.push_back(maxScoreString);
        }

        for (size_t j : selectedIndices) {
            if (j == 0 || j >= metaRow.size()) continue;

            uint32_t individualMaxScore = static_cast<uint32_t>(std::stoul(metaRow[j]));
            result.S_maxScores.push_back(individualMaxScore);
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

                double obtained = static_cast<double>(std::stod(row[i]));
                result.S_rawScores.push_back(obtained);
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

    bool createRecordsTable(mysqlx::Session& sess, const std::vector<std::string>& selectedHeaders, const std::string tableName, const std::string parentTableName) {

        try {
            std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + "(";
            query += "student_id VARCHAR(7) PRIMARY KEY, ";
            for (size_t i = 0; i < selectedHeaders.size(); ++i) {
                query += selectedHeaders[i] + " INT UNSIGNED";
                query += ",";
            }
            query += "CONSTRAINT FK_student_id ";
            query += "FOREIGN KEY (student_id) REFERENCES " + parentTableName + "(student_id)";
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
        std::vector<ResultRow> rawData = extractRows(selectedHeaders);

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

                for (const auto& row : rawData) {

                    std::vector<mysqlx::Value> values;
                    values.push_back(row.firstColumn);

                    for (double score : row.S_rawScores) {
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

