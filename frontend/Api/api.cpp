#include "httplib.h"
#include <libpq-fe.h>
#include <iostream>
#include <sstream>

int main() {

    // PostgreSQL Connection
    PGconn* conn = PQconnectdb(
        "host=localhost port=5432 dbname=faarismart user=postgres password=1234"
    );

    // Check connection
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cout << "Database Connection Failed!\n";
        std::cout << PQerrorMessage(conn);
        PQfinish(conn);
        return 1;
    }

    httplib::Server server;

    // Home Route
    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("FaarisMart Backend Running!", "text/plain");
    });

    // Products Route
    server.Get("/products", [&](const httplib::Request&, httplib::Response& res) {

        PGresult* result = PQexec(
            conn,
            "SELECT product_id, product_name, price, image FROM products ORDER BY product_id;"
        );

        if (PQresultStatus(result) != PGRES_TUPLES_OK) {
            res.status = 500;
            res.set_content("{\"error\":\"Database Query Failed\"}", "application/json");
            PQclear(result);
            return;
        }

        std::stringstream json;
        json << "[";

        int rows = PQntuples(result);

        for (int i = 0; i < rows; i++) {
            json << "{";
            json << "\"id\":" << PQgetvalue(result, i, 0) << ",";
            json << "\"name\":\"" << PQgetvalue(result, i, 1) << "\",";
            json << "\"price\":" << PQgetvalue(result, i, 2) << ",";
            json << "\"image\":\"" << PQgetvalue(result, i, 3) << "\"";
            json << "}";

            if (i != rows - 1)
                json << ",";
        }

        json << "]";

        PQclear(result);

        res.set_content(json.str(), "application/json");
    });

    std::cout << "Server started at http://localhost:8080\n";

    server.listen("0.0.0.0", 8080);

    PQfinish(conn);

    return 0;
}