
#include "database.h"
#include <iostream>

using namespace std;

PGconn* connectDB()
{
    PGconn* conn = PQconnectdb(
        "host=localhost port=5432 dbname=faarismart user=postgres password=YOUR_POSTGRES_PASSWORD"
    );

    if (PQstatus(conn) != CONNECTION_OK)
    {
        cout << "Database Connection Failed!\n";
        cout << PQerrorMessage(conn) << endl;
    }
    else
    {
        cout << "Database Connected Successfully!\n";
    }

    return conn;
}