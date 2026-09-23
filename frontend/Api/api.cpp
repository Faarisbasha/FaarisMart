#include "httplib.h"
#include <libpq-fe.h>
#include <iostream>
#include <sstream>
#include <regex>

using namespace httplib;

// -------------------- JSON Value Extractor --------------------
std::string getValue(const std::string& body, const std::string& key) {
    std::regex rg("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (std::regex_search(body, match, rg))
        return match[1];
    return "";
}

int main() {

    // -------------------- Database Connection --------------------
    PGconn* conn = PQconnectdb(
        "host=localhost port=5432 dbname=faarismart user=postgres password=1234"
    );

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cout << "Database Connection Failed!\n";
        std::cout << PQerrorMessage(conn);
        return 1;
    }

    std::cout << "Database Connected Successfully!\n";

    Server server;

    // -------------------- CORS --------------------
    server.set_pre_routing_handler([](const Request&, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        return Server::HandlerResponse::Unhandled;
    });

    server.Options(R"(.*)", [](const Request&, Response& res) {
        res.status = 200;
    });

    // -------------------- Home --------------------
    server.Get("/", [](const Request&, Response& res) {
        res.set_content("FaarisMart Backend Running!", "text/plain");
    });

    // ================= REGISTER =================
    server.Post("/register", [&](const Request& req, Response& res) {

        std::string username = getValue(req.body, "username");
        std::string email = getValue(req.body, "email");
        std::string password = getValue(req.body, "password");

        std::string query =
            "INSERT INTO users(username,email,password) VALUES('" +
            username + "','" + email + "','" + password + "');";

        PGresult* result = PQexec(conn, query.c_str());

        if (PQresultStatus(result) == PGRES_COMMAND_OK) {
            res.set_content("{\"success\":true}", "application/json");
        } else {
            res.set_content(
                "{\"success\":false,\"message\":\"Username or Email already exists\"}",
                "application/json"
            );
        }

        PQclear(result);
    });

    // ================= LOGIN =================
    server.Post("/login", [&](const Request& req, Response& res) {

        std::string username = getValue(req.body, "username");
        std::string password = getValue(req.body, "password");

        std::string query =
            "SELECT user_id,username FROM users WHERE username='" +
            username + "' AND password='" + password + "';";

        PGresult* result = PQexec(conn, query.c_str());

        if (PQntuples(result) == 1) {

            std::stringstream json;
            json << "{";
            json << "\"success\":true,";
            json << "\"user_id\":" << PQgetvalue(result,0,0) << ",";
            json << "\"username\":\"" << PQgetvalue(result,0,1) << "\"";
            json << "}";

            res.set_content(json.str(), "application/json");

        } else {

            res.set_content(
                "{\"success\":false,\"message\":\"Invalid Username or Password\"}",
                "application/json"
            );
        }

        PQclear(result);
    });

    // ================= PRODUCTS =================
    server.Get("/products", [&](const Request&, Response& res) {

        PGresult* result = PQexec(
            conn,
            "SELECT product_id, product_name, price, image, description, category, stock, rating FROM products ORDER BY product_id;"
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
            json << "\"id\":" << PQgetvalue(result,i,0) << ",";
            json << "\"name\":\"" << PQgetvalue(result,i,1) << "\",";
            json << "\"price\":" << PQgetvalue(result,i,2) << ",";
            json << "\"image\":\"" << PQgetvalue(result,i,3) << "\",";
            json << "\"description\":\"" << PQgetvalue(result,i,4) << "\",";
            json << "\"category\":\"" << PQgetvalue(result,i,5) << "\",";
            json << "\"stock\":" << PQgetvalue(result,i,6) << ",";
            json << "\"rating\":" << PQgetvalue(result,i,7);
            json << "}";

            if (i != rows - 1)
                json << ",";
        }

        json << "]";
        PQclear(result);
        res.set_content(json.str(), "application/json");
    });

    // ================= START SERVER =================

    std::cout << "Starting FaarisMart server...\n";

    bool started = server.listen("0.0.0.0", 8080);

    if (!started) {
        std::cout << "Failed to start server on port 8080.\n";
    }

    PQfinish(conn);
    return 0;
}