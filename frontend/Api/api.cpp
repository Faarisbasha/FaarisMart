#include "httplib.h"
#include <libpq-fe.h>
#include <iostream>
#include <sstream>
#include <regex>
#include <string>

using namespace httplib;

// -------------------- JSON Value Extractor --------------------
std::string getValue(const std::string& body, const std::string& key) {
    std::regex rg("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (std::regex_search(body, match, rg))
        return match[1];
    return "";
}

// -------------------- Escape JSON --------------------
std::string escapeJSON(const std::string& text) {
    std::string out;

    for (char c : text) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c;
        }
    }

    return out;
}

int main() {

    // ================= DATABASE CONNECTION =================

    PGconn* conn = PQconnectdb(
        "host=127.0.0.1 port=5432 dbname=faarismart user=postgres password=faaris2705"
    );

    if (PQstatus(conn) != CONNECTION_OK) {

        std::cout << "Database Connection Failed!\n";
        std::cout << PQerrorMessage(conn);

        PQfinish(conn);
        return 1;
    }

    std::cout << "Database Connected Successfully!\n";

    Server server;

    // ================= CORS =================

    server.set_pre_routing_handler([](const Request&, Response& res) {

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        return Server::HandlerResponse::Unhandled;

    });

    server.Options(R"(.*)", [](const Request&, Response& res) {

        res.status = 200;

    });

    // ================= HOME =================

    server.Get("/", [](const Request&, Response& res) {

        res.set_content("FaarisMart Backend Running!", "text/plain");

    });

    // ==========================================================
    // REGISTER
    // ==========================================================

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

    // ==========================================================
    // LOGIN
    // ==========================================================

    server.Post("/login", [&](const Request& req, Response& res) {

        std::string username = getValue(req.body, "username");
        std::string password = getValue(req.body, "password");

        std::string query =
            "SELECT user_id, username FROM users WHERE username='" +
            username + "' AND password='" + password + "';";

        PGresult* result = PQexec(conn, query.c_str());

        if (PQntuples(result) == 1) {

            std::stringstream json;

            json << "{";
            json << "\"success\":true,";
            json << "\"user_id\":" << PQgetvalue(result,0,0) << ",";
            json << "\"username\":\""
                 << escapeJSON(PQgetvalue(result,0,1))
                 << "\"";
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

    // ==========================================================
    // PRODUCTS
    // ==========================================================

    server.Get("/products", [&](const Request&, Response& res) {

        PGresult* result = PQexec(
            conn,
            "SELECT product_id, product_name, price, image, description, category, stock, rating FROM products ORDER BY product_id;"
        );

        if (PQresultStatus(result) != PGRES_TUPLES_OK) {

            std::string err = PQerrorMessage(conn);

            std::cout << "Products Query Error: " << err << std::endl;

            res.status = 500;
            res.set_content(
                "{\"error\":\"" + escapeJSON(err) + "\"}",
                "application/json"
            );

            PQclear(result);
            return;
        }

        std::stringstream json;
        json << "[";

        int rows = PQntuples(result);

        for (int i = 0; i < rows; i++) {

            json << "{";
            json << "\"id\":" << PQgetvalue(result,i,0) << ",";
            json << "\"name\":\"" << escapeJSON(PQgetvalue(result,i,1)) << "\",";
            json << "\"price\":" << PQgetvalue(result,i,2) << ",";
            json << "\"image\":\"" << escapeJSON(PQgetvalue(result,i,3)) << "\",";
            json << "\"description\":\"" << escapeJSON(PQgetvalue(result,i,4)) << "\",";
            json << "\"category\":\"" << escapeJSON(PQgetvalue(result,i,5)) << "\",";
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

    // ==========================================================
    // SAVE ORDER
    // ==========================================================

    server.Post("/orders", [&](const Request& req, Response& res) {

        std::string user_id = getValue(req.body, "user_id");
        std::string customer = getValue(req.body, "customer_name");
        std::string phone = getValue(req.body, "phone");
        std::string address = getValue(req.body, "address");
        std::string payment = getValue(req.body, "payment_method");
        std::string total = getValue(req.body, "total");

        if (user_id.empty()) user_id = "NULL";
        if (total.empty()) total = "0";

        std::string query =
            "INSERT INTO orders(user_id, customer_name, phone, address, payment_method, total) "
            "VALUES(" + user_id + ", '" + customer + "', '" + phone + "', '" +
            address + "', '" + payment + "', " + total + ") RETURNING order_id;";

        PGresult* result = PQexec(conn, query.c_str());

        if (PQresultStatus(result) == PGRES_TUPLES_OK) {

            std::string orderId = PQgetvalue(result,0,0);

            res.set_content(
                "{\"success\":true,\"order_id\":" + orderId + "}",
                "application/json"
            );

        } else {

            std::string err = PQerrorMessage(conn);

            std::cout << "Save Order Error: " << err << std::endl;

            res.status = 500;

            res.set_content(
                "{\"success\":false,\"error\":\"" + escapeJSON(err) + "\"}",
                "application/json"
            );
        }

        PQclear(result);
    });

    // ==========================================================
    // GET ORDERS
    // ==========================================================

    server.Get("/orders", [&](const Request&, Response& res) {

        PGresult* result = PQexec(
            conn,
            "SELECT order_id, customer_name, phone, address, payment_method, total, order_date "
            "FROM orders ORDER BY order_date DESC;"
        );

        if (PQresultStatus(result) != PGRES_TUPLES_OK) {

            std::string err = PQerrorMessage(conn);

            std::cout << "Orders Query Error: " << err << std::endl;

            res.status = 500;

            res.set_content(
                "{\"error\":\"" + escapeJSON(err) + "\"}",
                "application/json"
            );

            PQclear(result);
            return;
        }

        std::stringstream json;
        json << "[";

        int rows = PQntuples(result);

        for (int i = 0; i < rows; i++) {

            json << "{";
            json << "\"order_id\":" << PQgetvalue(result,i,0) << ",";
            json << "\"customer_name\":\"" << escapeJSON(PQgetvalue(result,i,1)) << "\",";
            json << "\"phone\":\"" << escapeJSON(PQgetvalue(result,i,2)) << "\",";
            json << "\"address\":\"" << escapeJSON(PQgetvalue(result,i,3)) << "\",";
            json << "\"payment_method\":\"" << escapeJSON(PQgetvalue(result,i,4)) << "\",";
            json << "\"total\":" << PQgetvalue(result,i,5) << ",";
            json << "\"order_date\":\"" << escapeJSON(PQgetvalue(result,i,6)) << "\"";
            json << "}";

            if (i != rows - 1)
                json << ",";
        }

        json << "]";

        PQclear(result);

        res.set_content(json.str(), "application/json");
    });

    // ================= START SERVER =================

    std::cout << "Starting FaarisMart Server at http://127.0.0.1:8080\n";

    if (!server.listen("0.0.0.0", 8080)) {

        std::cout << "Failed to start server on port 8080.\n";

    }

    PQfinish(conn);

    return 0;
}