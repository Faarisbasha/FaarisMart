#include "database.h"
#include <iostream>
#include <cstdlib>

using namespace std;

PGconn* connectDB()
{
    const char* conninfo = std::getenv("DATABASE_URL");

    if (conninfo == nullptr || conninfo[0] == '\0')
    {
        cerr << "DATABASE_URL environment variable is not configured!\n";
        return nullptr;
    }

    PGconn* conn = PQconnectdb(conninfo);

    if (conn == nullptr)
    {
        cerr << "Could not create PostgreSQL connection object!\n";
        return nullptr;
    }

    if (PQstatus(conn) != CONNECTION_OK)
    {
        cerr << "Database Connection Failed!\n";
        cerr << PQerrorMessage(conn) << '\n';
        PQfinish(conn);
        return nullptr;
    }

    cout << "Database Connected Successfully!\n";
    return conn;
}