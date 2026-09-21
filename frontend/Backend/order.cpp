#include <iostream>
#include "order.h"

using namespace std;

// Save purchased items into orders table
void saveOrder(PGconn* conn, string username)
{
    const char* params[1] = { username.c_str() };

    PGresult* res = PQexecParams(
        conn,
        "INSERT INTO orders(username,product_id,quantity,price) "
        "SELECT c.username,c.product_id,c.quantity,p.price "
        "FROM cart c JOIN products p ON c.product_id=p.product_id "
        "WHERE c.username=$1;",
        1,
        NULL,
        params,
        NULL,
        NULL,
        0
    );

    PQclear(res);
}

// View previous orders
void viewOrders(PGconn* conn, string username)
{
    const char* params[1] = { username.c_str() };

    PGresult* res = PQexecParams(
        conn,
        "SELECT o.order_id,p.product_name,o.quantity,o.price,o.order_date "
        "FROM orders o JOIN products p ON o.product_id=p.product_id "
        "WHERE o.username=$1 "
        "ORDER BY o.order_date DESC;",
        1,
        NULL,
        params,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        cout << "Error: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return;
    }

    cout << "\n=========================================\n";
    cout << "            MY ORDERS\n";
    cout << "=========================================\n";

    if (PQntuples(res) == 0)
    {
        cout << "No orders found.\n";
        PQclear(res);
        return;
    }

    for (int i = 0; i < PQntuples(res); i++)
    {
        cout << "Order ID : " << PQgetvalue(res, i, 0) << endl;
        cout << "Product  : " << PQgetvalue(res, i, 1) << endl;
        cout << "Quantity : " << PQgetvalue(res, i, 2) << endl;
        cout << "Price    : Rs." << PQgetvalue(res, i, 3) << endl;
        cout << "Date     : " << PQgetvalue(res, i, 4) << endl;
        cout << "-----------------------------------------\n";
    }

    PQclear(res);
}