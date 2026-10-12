
#include "httplib.h"
#include <libpq-fe.h>
#include <iostream>
#include <sstream>
#include <regex>
#include <string>
#include <cstdlib>

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
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c;
        }
    }

    return out;
}

int main() {

    // ================= DATABASE CONNECTION =================

    const char* databaseUrl = std::getenv("DATABASE_URL");

    if (databaseUrl == nullptr || databaseUrl[0] == '\0') {
        std::cerr << "DATABASE_URL environment variable is missing.\n";
        return 1;
    }

    PGconn* conn = PQconnectdb(databaseUrl);

    if (conn == nullptr) {
        std::cerr << "Could not create PostgreSQL connection.\n";
        return 1;
    }

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Database connection failed:\n"
                  << PQerrorMessage(conn);

        PQfinish(conn);
        return 1;
    }

    std::cout << "Database connected successfully!\n";

    Server server;

    // ================= CORS =================

    server.set_pre_routing_handler(
        [](const Request&, Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header(
                "Access-Control-Allow-Methods",
                "GET, POST, OPTIONS"
            );
            res.set_header(
                "Access-Control-Allow-Headers",
                "Content-Type"
            );

            return Server::HandlerResponse::Unhandled;
        }
    );

    server.Options(R"(.*)", [](const Request&, Response& res) {
        res.status = 200;
    });

    // ================= HOME =================

    server.Get("/", [](const Request&, Response& res) {
        res.set_content(
            "FaarisMart Backend Running!",
            "text/plain"
        );
    });

    // ================= REGISTER =================

    server.Post("/register", [&](const Request& req, Response& res) {
        std::string username = getValue(req.body, "username");
        std::string email = getValue(req.body, "email");
        std::string password = getValue(req.body, "password");

        const char* values[] = {
            username.c_str(),
            email.c_str(),
            password.c_str()
        };

        PGresult* result = PQexecParams(
            conn,
            "INSERT INTO users(username, email, password) "
            "VALUES($1, $2, $3)",
            3,
            nullptr,
            values,
            nullptr,
            nullptr,
            0
        );

        if (PQresultStatus(result) == PGRES_COMMAND_OK) {
            res.set_content(
                "{\"success\":true}",
                "application/json"
            );
        } else {
            res.status = 400;
            res.set_content(
                "{\"success\":false,"
                "\"message\":\"Registration failed. Username or email may already exist.\"}",
                "application/json"
            );
        }

        PQclear(result);
    });

    // ================= LOGIN =================

    server.Post("/login", [&](const Request& req, Response& res) {
        std::string username = getValue(req.body, "username");
        std::string password = getValue(req.body, "password");

        const char* values[] = {
            username.c_str(),
            password.c_str()
        };

        PGresult* result = PQexecParams(
            conn,
            "SELECT user_id, username FROM users "
            "WHERE username = $1 AND password = $2",
            2,
            nullptr,
            values,
            nullptr,
            nullptr,
            0
        );

        if (PQresultStatus(result) == PGRES_TUPLES_OK &&
            PQntuples(result) == 1) {

            std::stringstream json;

            json << "{";
            json << "\"success\":true,";
            json << "\"user_id\":" << PQgetvalue(result, 0, 0) << ",";
            json << "\"username\":\""
                 << escapeJSON(PQgetvalue(result, 0, 1))
                 << "\"";
            json << "}";

            res.set_content(json.str(), "application/json");
        } else {
            res.status = 401;
            res.set_content(
                "{\"success\":false,"
                "\"message\":\"Invalid Username or Password\"}",
                "application/json"
            );
        }

        PQclear(result);
    });

    // ================= PRODUCTS =================

    server.Get("/products", [&](const Request&, Response& res) {
        PGresult* result = PQexec(
            conn,
            "SELECT product_id, product_name, price, image, "
            "description, category, stock, rating "
            "FROM products ORDER BY product_id"
        );

        if (PQresultStatus(result) != PGRES_TUPLES_OK) {
            std::cerr << "Products query failed: "
                      << PQerrorMessage(conn) << '\n';

            res.status = 500;
            res.set_content(
                "{\"error\":\"Unable to load products\"}",
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
            json << "\"id\":" << PQgetvalue(result, i, 0) << ",";
            json << "\"name\":\""
                 << escapeJSON(PQgetvalue(result, i, 1)) << "\",";
            json << "\"price\":" << PQgetvalue(result, i, 2) << ",";
            json << "\"image\":\""
                 << escapeJSON(PQgetvalue(result, i, 3)) << "\",";
            json << "\"description\":\""
                 << escapeJSON(PQgetvalue(result, i, 4)) << "\",";
            json << "\"category\":\""
                 << escapeJSON(PQgetvalue(result, i, 5)) << "\",";
            json << "\"stock\":" << PQgetvalue(result, i, 6) << ",";
            json << "\"rating\":" << PQgetvalue(result, i, 7);
            json << "}";

            if (i != rows - 1)
                json << ",";
        }

        json << "]";

        PQclear(result);
        res.set_content(json.str(), "application/json");
    });

    // ================= SAVE ORDER =================

    server.Post("/orders", [&](const Request& req, Response& res) {
        std::string userId = getValue(req.body, "user_id");
        std::string customer = getValue(req.body, "customer_name");
        std::string phone = getValue(req.body, "phone");
        std::string address = getValue(req.body, "address");
        std::string payment = getValue(req.body, "payment_method");
        std::string total = getValue(req.body, "total");

        if (total.empty())
            total = "0";

        // An empty user_id is stored as SQL NULL.
        const char* userIdValue =
            userId.empty() ? nullptr : userId.c_str();

        const char* values[] = {
            userIdValue,
            customer.c_str(),
            phone.c_str(),
            address.c_str(),
            payment.c_str(),
            total.c_str()
        };

        const int lengths[] = {0, 0, 0, 0, 0, 0};
        const int formats[] = {0, 0, 0, 0, 0, 0};

        PGresult* result = PQexecParams(
            conn,
            "INSERT INTO orders "
            "(user_id, customer_name, phone, address, payment_method, total) "
            "VALUES($1, $2, $3, $4, $5, $6) RETURNING order_id",
            6,
            nullptr,
            values,
            lengths,
            formats,
            0
        );

        if (PQresultStatus(result) == PGRES_TUPLES_OK) {
            std::string orderId = PQgetvalue(result, 0, 0);

            res.set_content(
                "{\"success\":true,\"order_id\":" + orderId + "}",
                "application/json"
            );
        } else {
            std::cerr << "Save order failed: "
                      << PQerrorMessage(conn) << '\n';

            res.status = 500;
            res.set_content(
                "{\"success\":false,"
                "\"error\":\"Unable to save order\"}",
                "application/json"
            );
        }

        PQclear(result);
    });

    // ================= GET ORDERS =================

    server.Get("/orders", [&](const Request&, Response& res) {
        PGresult* result = PQexec(
            conn,
            "SELECT order_id, customer_name, phone, address, "
            "payment_method, total, order_date "
            "FROM orders ORDER BY order_date DESC"
        );

        if (PQresultStatus(result) != PGRES_TUPLES_OK) {
            std::cerr << "Orders query failed: "
                      << PQerrorMessage(conn) << '\n';

            res.status = 500;
            res.set_content(
                "{\"error\":\"Unable to load orders\"}",
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
            json << "\"order_id\":" << PQgetvalue(result, i, 0) << ",";
            json << "\"customer_name\":\""
                 << escapeJSON(PQgetvalue(result, i, 1)) << "\",";
            json << "\"phone\":\""
                 << escapeJSON(PQgetvalue(result, i, 2)) << "\",";
            json << "\"address\":\""
                 << escapeJSON(PQgetvalue(result, i, 3)) << "\",";
            json << "\"payment_method\":\""
                 << escapeJSON(PQgetvalue(result, i, 4)) << "\",";
            json << "\"total\":" << PQgetvalue(result, i, 5) << ",";
            json << "\"order_date\":\""
                 << escapeJSON(PQgetvalue(result, i, 6)) << "\"";
            json << "}";

            if (i != rows - 1)
                json << ",";
        }

        json << "]";

        PQclear(result);
        res.set_content(json.str(), "application/json");
    });

    // ================= START SERVER =================

    const char* portEnv = std::getenv("PORT");
    int port = 8080;

    if (portEnv != nullptr && portEnv[0] != '\0') {
        try {
            size_t processed = 0;
            int parsedPort = std::stoi(portEnv, &processed);

            if (processed != std::string(portEnv).size() ||
                parsedPort < 1 || parsedPort > 65535) {
                std::cerr << "Invalid PORT environment variable.\n";
                PQfinish(conn);
                return 1;
            }

            port = parsedPort;
        } catch (...) {
            std::cerr << "Invalid PORT environment variable.\n";
            PQfinish(conn);
            return 1;
        }
    }

    std::cout << "Starting FaarisMart API on port "
              << port << '\n';

    if (!server.listen("0.0.0.0", port)) {
        std::cerr << "Failed to start the API server.\n";
        PQfinish(conn);
        return 1;
    }

    PQfinish(conn);
    return 0;
}