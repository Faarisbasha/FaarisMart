#ifndef DATABASE_H
#define DATABASE_H

#include <libpq-fe.h>

PGconn* connectDB();
PGconn* conn = PQconnectdb(
    "host=localhost port=5432 dbname=faarismart user=postgres password=faaris2705"
);

#endif
