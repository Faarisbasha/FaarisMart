
#include "database.h"
#include <iostream>
#include <cstdlib>
#include <libpq-fe.h>

PGconn* connectDB()
{
    // Read the database connection URL from the environment
    const char* conninfo = std::getenv("DATABASE_URL");

    if (conninfo == nullptr || conninfo[0] == '\0')
    {
        std::cerr << "DATABASE_URL is not configured!\n";
        return nullptr;
    }

    // Connect to PostgreSQL
    PGconn* conn = PQconnectdb(conninfo);

    // Check whether the connection was created
    if (conn == nullptr)
    {
        std::cerr << "Could not create PostgreSQL connection!\n";
        return nullptr;
    }

    // Check whether the database connection succeeded
    if (PQstatus(conn) != CONNECTION_OK)
    {
        std::cerr << "Database connection failed: "
                  << PQerrorMessage(conn) << '\n';

        PQfinish(conn);
        return nullptr;
    }

    std::cout << "Database connected successfully!\n";

    return conn;
}