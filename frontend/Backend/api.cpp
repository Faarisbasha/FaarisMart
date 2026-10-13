#include "httplib.h"
#include <libpq-fe.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>
#include <cstdlib>
#include <cstdint>
#include <iomanip>
#include <random>
#include <algorithm>
#include <csignal>

using namespace httplib;

// =====================================================================
// CRYPTOGRAPHY & PASSWORD HASHING (Standard FIPS 180-4 SHA-256 + Salt)
// =====================================================================
namespace crypto {

inline uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

inline uint32_t choose(uint32_t e, uint32_t f, uint32_t g) {
    return (e & f) ^ (~e & g);
}

inline uint32_t majority(uint32_t a, uint32_t b, uint32_t c) {
    return (a & b) ^ (a & c) ^ (b & c);
}

inline uint32_t sig0(uint32_t x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

inline uint32_t sig1(uint32_t x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

inline uint32_t theta0(uint32_t x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

inline uint32_t theta1(uint32_t x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

std::string sha256(const std::string& input) {
    uint32_t H[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    std::vector<uint8_t> msg(input.begin(), input.end());
    uint64_t bitLength = static_cast<uint64_t>(msg.size()) * 8;

    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) {
        msg.push_back(0x00);
    }

    for (int i = 7; i >= 0; --i) {
        msg.push_back(static_cast<uint8_t>((bitLength >> (i * 8)) & 0xFF));
    }

    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        uint32_t W[64];
        for (int t = 0; t < 16; ++t) {
            W[t] = (static_cast<uint32_t>(msg[chunk + t * 4]) << 24) |
                   (static_cast<uint32_t>(msg[chunk + t * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(msg[chunk + t * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(msg[chunk + t * 4 + 3]));
        }
        for (int t = 16; t < 64; ++t) {
            W[t] = theta1(W[t - 2]) + W[t - 7] + theta0(W[t - 15]) + W[t - 16];
        }

        uint32_t a = H[0];
        uint32_t b = H[1];
        uint32_t c = H[2];
        uint32_t d = H[3];
        uint32_t e = H[4];
        uint32_t f = H[5];
        uint32_t g = H[6];
        uint32_t h = H[7];

        for (int t = 0; t < 64; ++t) {
            uint32_t T1 = h + sig1(e) + choose(e, f, g) + K[t] + W[t];
            uint32_t T2 = sig0(a) + majority(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        H[0] += a;
        H[1] += b;
        H[2] += c;
        H[3] += d;
        H[4] += e;
        H[5] += f;
        H[6] += g;
        H[7] += h;
    }

    std::ostringstream ss;
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << std::setw(8) << std::setfill('0') << H[i];
    }
    return ss.str();
}

std::string generateSalt(size_t len = 16) {
    static const char hexChars[] = "0123456789abcdef";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::string salt;
    for (size_t i = 0; i < len; ++i) {
        salt += hexChars[dis(gen)];
    }
    return salt;
}

std::string hashPassword(const std::string& password) {
    std::string salt = generateSalt(16);
    std::string hash = sha256(salt + ":" + password);
    return "sha256$" + salt + "$" + hash;
}

bool verifyPassword(const std::string& password, const std::string& stored) {
    if (stored.rfind("sha256$", 0) == 0) {
        size_t first = 7;
        size_t second = stored.find('$', first);
        if (second == std::string::npos) return false;
        std::string salt = stored.substr(first, second - first);
        std::string expectedHash = stored.substr(second + 1);
        std::string actualHash = sha256(salt + ":" + password);
        return (actualHash == expectedHash);
    }
    // Backward compatibility with legacy plaintext passwords
    return (stored == password);
}

} // namespace crypto

// =====================================================================
// JSON HELPERS
// =====================================================================
std::string getJsonValue(const std::string& body, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = body.find(needle);
    if (pos == std::string::npos) return "";
    pos += needle.length();

    while (pos < body.length() && (body[pos] == ' ' || body[pos] == '\t' || body[pos] == '\n' || body[pos] == '\r')) {
        pos++;
    }
    if (pos >= body.length() || body[pos] != ':') return "";
    pos++;

    while (pos < body.length() && (body[pos] == ' ' || body[pos] == '\t' || body[pos] == '\n' || body[pos] == '\r')) {
        pos++;
    }
    if (pos >= body.length()) return "";

    if (body[pos] == '"') {
        pos++;
        std::string val;
        while (pos < body.length()) {
            if (body[pos] == '\\' && pos + 1 < body.length()) {
                val += body[pos + 1];
                pos += 2;
            } else if (body[pos] == '"') {
                break;
            } else {
                val += body[pos];
                pos++;
            }
        }
        return val;
    } else {
        size_t end = pos;
        while (end < body.length() && body[end] != ',' && body[end] != '}' && body[end] != ']' &&
               body[end] != ' ' && body[end] != '\t' && body[end] != '\n' && body[end] != '\r') {
            end++;
        }
        std::string val = body.substr(pos, end - pos);
        if (val == "null") return "";
        return val;
    }
}

std::string escapeJSON(const std::string& text) {
    std::string out;
    for (char c : text) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 32) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    out += buf;
                } else {
                    out += c;
                }
                break;
        }
    }
    return out;
}

// RAII Guard for libpq PGresult
struct PGResultGuard {
    PGresult* res = nullptr;
    PGResultGuard(PGresult* r) : res(r) {}
    ~PGResultGuard() {
        if (res) {
            PQclear(res);
            res = nullptr;
        }
    }
    PGresult* get() const { return res; }
    operator PGresult*() const { return res; }
};

// =====================================================================
// THREAD-SAFE DATABASE MANAGER
// =====================================================================
class DatabaseManager {
private:
    std::string conninfo;
    PGconn* conn = nullptr;
    std::mutex mtx;

public:
    DatabaseManager(const std::string& info) : conninfo(info) {}

    ~DatabaseManager() {
        std::lock_guard<std::mutex> lock(mtx);
        if (conn) {
            PQfinish(conn);
            conn = nullptr;
        }
    }

    bool init() {
        std::lock_guard<std::mutex> lock(mtx);
        conn = PQconnectdb(conninfo.c_str());
        if (conn == nullptr) {
            std::cerr << "[FATAL] Could not allocate PostgreSQL connection object.\n";
            return false;
        }
        if (PQstatus(conn) != CONNECTION_OK) {
            std::cerr << "[FATAL] Database connection failed:\n"
                      << PQerrorMessage(conn) << "\n";
            PQfinish(conn);
            conn = nullptr;
            return false;
        }
        std::cout << "[INFO] Database connected successfully!\n";
        return true;
    }

    bool isHealthy() {
        std::lock_guard<std::mutex> lock(mtx);
        return ensureConnectedInternal();
    }

    bool ensureConnectedInternal() {
        if (!conn || PQstatus(conn) != CONNECTION_OK) {
            std::cout << "[WARN] Reconnecting to PostgreSQL...\n";
            if (conn) {
                PQreset(conn);
            } else {
                conn = PQconnectdb(conninfo.c_str());
            }
            if (!conn || PQstatus(conn) != CONNECTION_OK) {
                std::cerr << "[ERROR] Database reconnection failed: "
                          << (conn ? PQerrorMessage(conn) : "connection null") << "\n";
                return false;
            }
            std::cout << "[INFO] Database reconnected successfully.\n";
        }
        return true;
    }

    std::mutex& getMutex() {
        return mtx;
    }

    PGconn* getRawConn() {
        if (!ensureConnectedInternal()) {
            return nullptr;
        }
        return conn;
    }

    // Run safe repeatable DDL initialization
    void runSchemaInit() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!ensureConnectedInternal()) return;

        const char* ddlStatements[] = {
            // Users table
            "CREATE TABLE IF NOT EXISTS users ("
            "  user_id SERIAL PRIMARY KEY,"
            "  name VARCHAR(100),"
            "  username VARCHAR(50) UNIQUE NOT NULL,"
            "  email VARCHAR(100) UNIQUE NOT NULL,"
            "  password VARCHAR(255) NOT NULL,"
            "  role VARCHAR(20) DEFAULT 'buyer',"
            "  created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP"
            ");",

            // Backward compatibility column 'id' if console app accesses it
            "DO $$ BEGIN "
            "IF NOT EXISTS (SELECT 1 FROM information_schema.columns WHERE table_name='users' AND column_name='id') THEN "
            "  ALTER TABLE users ADD COLUMN id INT GENERATED ALWAYS AS (user_id) STORED; "
            "END IF; END $$;",

            // Products table
            "CREATE TABLE IF NOT EXISTS products ("
            "  product_id SERIAL PRIMARY KEY,"
            "  product_name VARCHAR(255) NOT NULL,"
            "  price NUMERIC(10, 2) NOT NULL,"
            "  image VARCHAR(255) DEFAULT 'placeholder.png',"
            "  description TEXT,"
            "  category VARCHAR(100),"
            "  stock INT DEFAULT 0,"
            "  rating NUMERIC(2, 1) DEFAULT 4.5,"
            "  seller_username VARCHAR(50) DEFAULT 'admin',"
            "  created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP"
            ");",

            // Orders table
            "CREATE TABLE IF NOT EXISTS orders ("
            "  order_id SERIAL PRIMARY KEY,"
            "  user_id INT REFERENCES users(user_id) ON DELETE SET NULL,"
            "  customer_name VARCHAR(100),"
            "  username VARCHAR(50),"
            "  phone VARCHAR(30),"
            "  address TEXT,"
            "  payment_method VARCHAR(50),"
            "  total NUMERIC(10, 2) DEFAULT 0,"
            "  product_id INT REFERENCES products(product_id) ON DELETE SET NULL,"
            "  quantity INT DEFAULT 1,"
            "  price NUMERIC(10, 2) DEFAULT 0,"
            "  order_date TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP"
            ");",

            // Cart table
            "CREATE TABLE IF NOT EXISTS cart ("
            "  cart_id SERIAL PRIMARY KEY,"
            "  username VARCHAR(50) NOT NULL,"
            "  product_id INT NOT NULL REFERENCES products(product_id) ON DELETE CASCADE,"
            "  quantity INT NOT NULL DEFAULT 1,"
            "  created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,"
            "  UNIQUE(username, product_id)"
            ");",

            // Reviews table
            "CREATE TABLE IF NOT EXISTS reviews ("
            "  review_id SERIAL PRIMARY KEY,"
            "  username VARCHAR(50) NOT NULL,"
            "  product_id INT NOT NULL REFERENCES products(product_id) ON DELETE CASCADE,"
            "  rating INT CHECK (rating >= 1 AND rating <= 5),"
            "  review_text TEXT,"
            "  review_date TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP"
            ");"
        };

        for (const char* sql : ddlStatements) {
            PGresult* res = PQexec(conn, sql);
            PQclear(res);
        }

        // Check if products table is empty; if so, populate initial catalogue
        PGresult* countRes = PQexec(conn, "SELECT COUNT(*) FROM products;");
        if (PQresultStatus(countRes) == PGRES_TUPLES_OK && PQntuples(countRes) > 0) {
            int count = std::atoi(PQgetvalue(countRes, 0, 0));
            if (count == 0) {
                std::cout << "[INFO] Products table is empty. Seeding initial catalogue...\n";
                const char* seedSql =
                    "INSERT INTO products (product_id, product_name, price, image, description, category, stock, rating, seller_username) VALUES "
                    "(1, 'ASUS TUF Gaming F15', 52990.00, 'asusutuf15.jpg', 'Intel Core i5 11th Gen, 16GB RAM, 512GB SSD, RTX 2050 Graphics, 144Hz FHD Display', 'Electronics', 15, 4.6, 'admin'), "
                    "(2, 'HP Victus Gaming Laptop', 62990.00, 'hpvictus.jpg', 'AMD Ryzen 5 5600H, 16GB RAM, 512GB SSD, AMD Radeon RX 6500M, 15.6 inch FHD', 'Electronics', 12, 4.5, 'admin'), "
                    "(3, 'Lenovo IdeaPad Slim 3', 38990.00, 'lenovo.jpg', 'Intel Core i3 12th Gen, 8GB RAM, 512GB SSD, Windows 11 Home, Thin & Light', 'Electronics', 20, 4.3, 'admin'), "
                    "(4, 'Apple iPhone 15 (128 GB)', 69900.00, 'iphone15.jpg', 'Dynamic Island, 48MP Main Camera, USB-C, A16 Bionic chip, All-Day Battery Life', 'Electronics', 25, 4.8, 'admin'), "
                    "(5, 'Samsung Galaxy S24 5G', 74999.00, 'galaxys24.jpg', 'Galaxy AI, 8GB RAM, 128GB Storage, 50MP Camera, Dynamic AMOLED 2X Display', 'Electronics', 18, 4.7, 'admin'), "
                    "(6, 'OnePlus 13 5G', 54999.00, 'oneplus13.jpg', 'Snapdragon 8 Elite, 16GB RAM, 256GB Storage, Hasselblad Camera, 100W SUPERVOOC', 'Electronics', 22, 4.7, 'admin'), "
                    "(7, 'Realme P1 Speed 5G', 17999.00, 'realmep1speed.jpg', 'Dimensity 7300 Energy 5G, 8GB RAM, 128GB ROM, 120Hz OLED, 45W Fast Charging', 'Electronics', 30, 4.4, 'admin'), "
                    "(8, 'Sony WH-CH520 Wireless Headphones', 3990.00, 'sonywhch520.jpg', 'Up to 50 Hours Battery Life, DSEE Upscaling, Multipoint Connection, Built-in Mic', 'Electronics', 40, 4.5, 'admin'), "
                    "(9, 'boAt Airdopes 141 ANC', 1499.00, 'boatearbuds.jpg', 'Active Noise Cancellation (up to 32dB), 42H Playtime, ENx Tech, ASAP Charge', 'Electronics', 50, 4.2, 'admin'), "
                    "(10, 'JBL Flip 6 Bluetooth Speaker', 9999.00, 'jblflip6.jpg', 'Powerful 2-Way Speaker System, IP67 Waterproof & Dustproof, 12 Hours Playtime', 'Electronics', 15, 4.7, 'admin'), "
                    "(11, 'Logitech G102 Lightsync Mouse', 1495.00, 'logitechg102.jpg', '8000 DPI Sensor, Customizable RGB Lighting, 6 Programmable Buttons', 'Electronics', 35, 4.6, 'admin'), "
                    "(12, 'HP Wireless Optical Mouse', 699.00, 'hpwirelessmouse..jpg', '2.4GHz Wireless Connectivity, 1600 DPI, Long Battery Life, Ergonomic Design', 'Electronics', 45, 4.3, 'admin'), "
                    "(13, 'Redgear Mechanical Keyboard', 2299.00, 'redgearkeyboard.jpg', 'Blue Mechanical Switches, Rainbow LED Backlit, Control Knob, Wrist Rest', 'Electronics', 20, 4.4, 'admin'), "
                    "(14, 'Xiaomi Smart Band 9', 2999.00, 'miband9.jpg', '1.62 inch AMOLED Display, 150+ Sports Modes, 21-Day Battery Life, Heart Rate & SpO2', 'Electronics', 28, 4.5, 'admin'), "
                    "(15, 'SanDisk Ultra 128GB MicroSD', 899.00, 'sandisk128gb.jpg', 'Up to 140MB/s Read Speed, Class 10, A1 App Performance, Full HD Video Support', 'Electronics', 60, 4.7, 'admin'), "
                    "(16, 'Nike Air Max SC Running Shoes', 5995.00, 'nikeshoes.jpg', 'Lightweight cushioning, breathable mesh and leather upper, heritage running look', 'Footwear', 14, 4.6, 'admin'), "
                    "(17, 'Adidas Ultraboost Light Shoes', 11999.00, 'adidasultraboost.jpg', 'Light BOOST midsole, Primeknit+ upper, Continental Rubber outsole for superior grip', 'Footwear', 10, 4.8, 'admin'), "
                    "(18, 'Puma Smash V2 Casual Sneakers', 2499.00, 'pumasneakers.jpg', 'Classic tennis-inspired silhouette, soft suede upper, durable rubber outsole', 'Footwear', 22, 4.3, 'admin'), "
                    "(19, 'Bata Formal Leather Shoes', 1999.00, 'batashoes.jpg', 'Genuine leather formal lace-up derby shoes, cushioned insole for all-day comfort', 'Footwear', 18, 4.2, 'admin'), "
                    "(20, 'Levis 511 Slim Fit Stretch Jeans', 2599.00, 'levisjeans.jpg', 'Slim through the thigh with a straight leg, premium stretch denim comfort', 'Fashion', 25, 4.5, 'admin'), "
                    "(21, 'Allen Solly Slim Fit Shirt', 1499.00, 'allensollyshirt.jpg', '100% Cotton, spread collar, long sleeves, versatile formal and casual wear', 'Fashion', 30, 4.4, 'admin'), "
                    "(22, 'Roadster Pure Cotton T-Shirt', 499.00, 'roadstertshirt.jpg', '100% Breathable cotton round neck casual t-shirt, relaxed regular fit', 'Fashion', 40, 4.2, 'admin'), "
                    "(23, 'Puma Fleece Pullover Hoodie', 2199.00, 'pumahoodie..jpg', 'Warm cotton-poly fleece, adjustable drawstring hood, kangaroo front pocket', 'Fashion', 16, 4.5, 'admin'), "
                    "(24, 'Fossil Leather Bifold Wallet', 2995.00, 'fossilwallet.jpg', '100% Genuine leather, RFID blocking technology, 8 card slots, 2 bill compartments', 'Accessories', 20, 4.6, 'admin'), "
                    "(25, 'Amazon Basics Laptop Sleeve', 699.00, 'amazonsleeve.jpg', 'Padded neoprene protective case, slim and compact, top zipper opening', 'Accessories', 35, 4.5, 'admin') "
                    "ON CONFLICT (product_id) DO NOTHING;";
                PGresult* seedRes = PQexec(conn, seedSql);
                PQclear(seedRes);

                PGresult* seqRes = PQexec(conn, "SELECT setval('products_product_id_seq', COALESCE((SELECT MAX(product_id) FROM products), 1));");
                PQclear(seqRes);
                std::cout << "[INFO] Initial catalogue seeded successfully.\n";
            }
        }
        PQclear(countRes);
    }
};

static Server* g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\n[INFO] Signal " << signum << " received, shutting down gracefully...\n";
    if (g_server) {
        g_server->stop();
    }
}

// =====================================================================
// MAIN ENTRY POINT
// =====================================================================
int main() {
    // 1. Validate DATABASE_URL environment variable
    const char* databaseUrl = std::getenv("DATABASE_URL");
    if (databaseUrl == nullptr || databaseUrl[0] == '\0') {
        std::cerr << "[FATAL] DATABASE_URL environment variable is missing or empty.\n"
                  << "[FATAL] The API requires a valid PostgreSQL connection string.\n"
                  << "[FATAL] On Render: Go to Web Service -> Environment -> Add 'DATABASE_URL' "
                  << "with the Internal Database URL from your 'faarismart-db' PostgreSQL service.\n";
        return 1;
    }

    // 2. Initialize PostgreSQL connection
    DatabaseManager db(databaseUrl);
    if (!db.init()) {
        std::cerr << "[FATAL] Could not connect to PostgreSQL. Halting startup.\n";
        return 1;
    }

    // 3. Run safe schema initialization (CREATE TABLE IF NOT EXISTS)
    db.runSchemaInit();

    // 4. Initialize HTTP Server
    Server server;
    g_server = &server;
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // ================= CORS & PRE-ROUTING =================
    server.set_pre_routing_handler(
        [](const Request& req, Response& res) {
            const char* allowedOriginsEnv = std::getenv("ALLOWED_ORIGINS");
            std::string origin = "*";
            if (allowedOriginsEnv != nullptr && allowedOriginsEnv[0] != '\0') {
                origin = allowedOriginsEnv;
            }
            res.set_header("Access-Control-Allow-Origin", origin);
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
            res.set_header("Access-Control-Max-Age", "86400");

            if (req.method == "OPTIONS") {
                res.status = 204;
                return Server::HandlerResponse::Handled;
            }
            return Server::HandlerResponse::Unhandled;
        }
    );

    // ================= HEALTH CHECK ROUTES =================
    server.Get("/", [](const Request&, Response& res) {
        res.set_content(
            "{\"status\":\"ok\",\"service\":\"FaarisMart API\",\"version\":\"1.0.0\"}",
            "application/json"
        );
    });

    server.Get("/health", [&db](const Request&, Response& res) {
        if (db.isHealthy()) {
            res.status = 200;
            res.set_content("{\"status\":\"ok\",\"database\":\"connected\"}", "application/json");
        } else {
            res.status = 503;
            res.set_content("{\"status\":\"degraded\",\"database\":\"disconnected\"}", "application/json");
        }
    });

    // ================= REGISTER =================
    server.Post("/register", [&db](const Request& req, Response& res) {
        std::string username = getJsonValue(req.body, "username");
        std::string email = getJsonValue(req.body, "email");
        std::string password = getJsonValue(req.body, "password");

        if (username.empty() || email.empty() || password.empty()) {
            res.status = 400;
            res.set_content("{\"success\":false,\"message\":\"Username, email, and password are required.\"}", "application/json");
            return;
        }

        if (password.length() < 6) {
            res.status = 400;
            res.set_content("{\"success\":false,\"message\":\"Password must be at least 6 characters long.\"}", "application/json");
            return;
        }

        // Hash password securely with random salt
        std::string hashedPassword = crypto::hashPassword(password);

        std::lock_guard<std::mutex> lock(db.getMutex());
        PGconn* conn = db.getRawConn();
        if (!conn) {
            res.status = 503;
            res.set_content("{\"success\":false,\"message\":\"Database temporarily unavailable.\"}", "application/json");
            return;
        }

        const char* values[] = {
            username.c_str(),
            email.c_str(),
            hashedPassword.c_str()
        };

        PGResultGuard result(PQexecParams(
            conn,
            "INSERT INTO users(username, email, password) VALUES($1, $2, $3)",
            3,
            nullptr,
            values,
            nullptr,
            nullptr,
            0
        ));

        if (PQresultStatus(result) == PGRES_COMMAND_OK) {
            res.status = 201;
            res.set_content("{\"success\":true,\"message\":\"Account created successfully.\"}", "application/json");
        } else {
            res.status = 400;
            res.set_content(
                "{\"success\":false,\"message\":\"Registration failed. Username or email may already exist.\"}",
                "application/json"
            );
        }
    });

    // ================= LOGIN =================
    server.Post("/login", [&db](const Request& req, Response& res) {
        std::string username = getJsonValue(req.body, "username");
        std::string password = getJsonValue(req.body, "password");

        if (username.empty() || password.empty()) {
            res.status = 400;
            res.set_content("{\"success\":false,\"message\":\"Username and password are required.\"}", "application/json");
            return;
        }

        std::lock_guard<std::mutex> lock(db.getMutex());
        PGconn* conn = db.getRawConn();
        if (!conn) {
            res.status = 503;
            res.set_content("{\"success\":false,\"message\":\"Database temporarily unavailable.\"}", "application/json");
            return;
        }

        const char* values[] = { username.c_str() };

        PGResultGuard result(PQexecParams(
            conn,
            "SELECT user_id, username, password FROM users WHERE username = $1",
            1,
            nullptr,
            values,
            nullptr,
            nullptr,
            0
        ));

        if (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) == 1) {
            std::string userId = PQgetvalue(result, 0, 0);
            std::string dbUsername = PQgetvalue(result, 0, 1);
            std::string storedPassword = PQgetvalue(result, 0, 2);

            if (crypto::verifyPassword(password, storedPassword)) {
                // If legacy plaintext password, upgrade in place
                if (storedPassword.rfind("sha256$", 0) != 0) {
                    std::string newHash = crypto::hashPassword(password);
                    const char* upValues[] = { newHash.c_str(), userId.c_str() };
                    PGResultGuard upRes(PQexecParams(
                        conn,
                        "UPDATE users SET password = $1 WHERE user_id = $2",
                        2,
                        nullptr,
                        upValues,
                        nullptr,
                        nullptr,
                        0
                    ));
                }

                std::stringstream json;
                json << "{";
                json << "\"success\":true,";
                json << "\"user_id\":" << userId << ",";
                json << "\"username\":\"" << escapeJSON(dbUsername) << "\",";
                json << "\"message\":\"Login successful\"";
                json << "}";

                res.status = 200;
                res.set_content(json.str(), "application/json");
                return;
            }
        }

        res.status = 401;
        res.set_content(
            "{\"success\":false,\"message\":\"Invalid username or password.\"}",
            "application/json"
        );
    });

    // ================= PRODUCTS =================
    server.Get("/products", [&db](const Request&, Response& res) {
        std::lock_guard<std::mutex> lock(db.getMutex());
        PGconn* conn = db.getRawConn();
        if (!conn) {
            res.status = 503;
            res.set_content("{\"error\":\"Database temporarily unavailable\"}", "application/json");
            return;
        }

        PGResultGuard result(PQexec(
            conn,
            "SELECT product_id, product_name, price, image, description, category, stock, rating "
            "FROM products ORDER BY product_id"
        ));

        if (PQresultStatus(result) != PGRES_TUPLES_OK) {
            std::cerr << "[ERROR] Products query failed: " << PQerrorMessage(conn) << "\n";
            res.status = 500;
            res.set_content("{\"error\":\"Unable to load products\"}", "application/json");
            return;
        }

        std::stringstream json;
        json << "[";
        int rows = PQntuples(result);

        for (int i = 0; i < rows; i++) {
            json << "{";
            json << "\"id\":" << PQgetvalue(result, i, 0) << ",";
            json << "\"name\":\"" << escapeJSON(PQgetvalue(result, i, 1)) << "\",";
            json << "\"price\":" << PQgetvalue(result, i, 2) << ",";
            json << "\"image\":\"" << escapeJSON(PQgetvalue(result, i, 3)) << "\",";
            json << "\"description\":\"" << escapeJSON(PQgetvalue(result, i, 4)) << "\",";
            json << "\"category\":\"" << escapeJSON(PQgetvalue(result, i, 5)) << "\",";
            json << "\"stock\":" << (PQgetvalue(result, i, 6)[0] ? PQgetvalue(result, i, 6) : "0") << ",";
            json << "\"rating\":" << (PQgetvalue(result, i, 7)[0] ? PQgetvalue(result, i, 7) : "0");
            json << "}";

            if (i != rows - 1) json << ",";
        }
        json << "]";

        res.status = 200;
        res.set_content(json.str(), "application/json");
    });

    // ================= SAVE ORDER =================
    server.Post("/orders", [&db](const Request& req, Response& res) {
        std::string userId = getJsonValue(req.body, "user_id");
        std::string customer = getJsonValue(req.body, "customer_name");
        std::string phone = getJsonValue(req.body, "phone");
        std::string address = getJsonValue(req.body, "address");
        std::string payment = getJsonValue(req.body, "payment_method");
        std::string total = getJsonValue(req.body, "total");

        if (customer.empty() || phone.empty() || address.empty()) {
            res.status = 400;
            res.set_content("{\"success\":false,\"message\":\"Customer name, phone, and delivery address are required.\"}", "application/json");
            return;
        }

        if (payment.empty()) payment = "COD";
        if (total.empty()) total = "0";

        const char* userIdParam = userId.empty() ? nullptr : userId.c_str();

        std::lock_guard<std::mutex> lock(db.getMutex());
        PGconn* conn = db.getRawConn();
        if (!conn) {
            res.status = 503;
            res.set_content("{\"success\":false,\"message\":\"Database temporarily unavailable.\"}", "application/json");
            return;
        }

        const char* values[] = {
            userIdParam,
            customer.c_str(),
            phone.c_str(),
            address.c_str(),
            payment.c_str(),
            total.c_str()
        };

        PGResultGuard result(PQexecParams(
            conn,
            "INSERT INTO orders(user_id, customer_name, phone, address, payment_method, total) "
            "VALUES($1, $2, $3, $4, $5, $6) RETURNING order_id",
            6,
            nullptr,
            values,
            nullptr,
            nullptr,
            0
        ));

        if (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0) {
            std::string orderId = PQgetvalue(result, 0, 0);
            res.status = 201;
            res.set_content(
                "{\"success\":true,\"order_id\":" + orderId + ",\"message\":\"Order placed successfully.\"}",
                "application/json"
            );
        } else {
            std::cerr << "[ERROR] Save order failed: " << PQerrorMessage(conn) << "\n";
            res.status = 500;
            res.set_content("{\"success\":false,\"error\":\"Unable to save order\"}", "application/json");
        }
    });

    // ================= GET ORDERS =================
    server.Get("/orders", [&db](const Request& req, Response& res) {
        std::lock_guard<std::mutex> lock(db.getMutex());
        PGconn* conn = db.getRawConn();
        if (!conn) {
            res.status = 503;
            res.set_content("{\"error\":\"Database temporarily unavailable\"}", "application/json");
            return;
        }

        std::string userIdFilter;
        if (req.has_param("user_id")) {
            userIdFilter = req.get_param_value("user_id");
        }

        PGresult* rawRes = nullptr;
        if (!userIdFilter.empty()) {
            const char* values[] = { userIdFilter.c_str() };
            rawRes = PQexecParams(
                conn,
                "SELECT order_id, customer_name, phone, address, payment_method, total, order_date "
                "FROM orders WHERE user_id = $1 ORDER BY order_date DESC",
                1,
                nullptr,
                values,
                nullptr,
                nullptr,
                0
            );
        } else {
            rawRes = PQexec(
                conn,
                "SELECT order_id, customer_name, phone, address, payment_method, total, order_date "
                "FROM orders ORDER BY order_date DESC"
            );
        }

        PGResultGuard result(rawRes);

        if (PQresultStatus(result) != PGRES_TUPLES_OK) {
            std::cerr << "[ERROR] Orders query failed: " << PQerrorMessage(conn) << "\n";
            res.status = 500;
            res.set_content("{\"error\":\"Unable to load orders\"}", "application/json");
            return;
        }

        std::stringstream json;
        json << "[";
        int rows = PQntuples(result);

        for (int i = 0; i < rows; i++) {
            json << "{";
            json << "\"order_id\":" << PQgetvalue(result, i, 0) << ",";
            json << "\"customer_name\":\"" << escapeJSON(PQgetvalue(result, i, 1)) << "\",";
            json << "\"phone\":\"" << escapeJSON(PQgetvalue(result, i, 2)) << "\",";
            json << "\"address\":\"" << escapeJSON(PQgetvalue(result, i, 3)) << "\",";
            json << "\"payment_method\":\"" << escapeJSON(PQgetvalue(result, i, 4)) << "\",";
            json << "\"total\":" << (PQgetvalue(result, i, 5)[0] ? PQgetvalue(result, i, 5) : "0") << ",";
            json << "\"order_date\":\"" << escapeJSON(PQgetvalue(result, i, 6)) << "\"";
            json << "}";

            if (i != rows - 1) json << ",";
        }
        json << "]";

        res.status = 200;
        res.set_content(json.str(), "application/json");
    });

    // ================= START SERVER =================
    const char* portEnv = std::getenv("PORT");
    int port = 8080;

    if (portEnv != nullptr && portEnv[0] != '\0') {
        try {
            size_t processed = 0;
            int parsedPort = std::stoi(portEnv, &processed);
            if (processed == std::string(portEnv).size() && parsedPort >= 1 && parsedPort <= 65535) {
                port = parsedPort;
            } else {
                std::cerr << "[WARN] Invalid PORT environment variable value '" << portEnv << "'. Using default 8080.\n";
            }
        } catch (...) {
            std::cerr << "[WARN] Could not parse PORT environment variable. Using default 8080.\n";
        }
    }

    std::cout << "[INFO] Starting FaarisMart API server on 0.0.0.0:" << port << "...\n";

    if (!server.listen("0.0.0.0", port)) {
        std::cerr << "[FATAL] Failed to start API server on port " << port << ".\n";
        return 1;
    }

    std::cout << "[INFO] Server stopped cleanly.\n";
    return 0;
}