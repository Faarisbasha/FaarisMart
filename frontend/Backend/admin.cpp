#include <iostream>
#include "admin.h"

using namespace std;

// View all users
void viewUsers(PGconn* conn)
{
    PGresult* res = PQexec(conn,
        "SELECT id,name,username,role FROM users ORDER BY id;");

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        cout << "Error: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return;
    }

    cout << "\n--------------- USERS ---------------\n";

    for (int i = 0; i < PQntuples(res); i++)
    {
        cout << "ID: " << PQgetvalue(res,i,0)
             << " | Name: " << PQgetvalue(res,i,1)
             << " | Username: " << PQgetvalue(res,i,2)
             << " | Role: " << PQgetvalue(res,i,3)
             << endl;
    }

    PQclear(res);
}

// View all orders
void viewAllOrders(PGconn* conn)
{
    PGresult* res = PQexec(conn,
        "SELECT o.order_id,o.username,p.product_name,o.quantity,o.price,o.order_date "
        "FROM orders o JOIN products p ON o.product_id=p.product_id "
        "ORDER BY o.order_date DESC;");

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        cout << "Error: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return;
    }

    cout << "\n============== ALL ORDERS ==============\n";

    if (PQntuples(res) == 0)
    {
        cout << "No orders found.\n";
        PQclear(res);
        return;
    }

    for (int i = 0; i < PQntuples(res); i++)
    {
        cout << "Order ID : " << PQgetvalue(res,i,0) << endl;
        cout << "Buyer    : " << PQgetvalue(res,i,1) << endl;
        cout << "Product  : " << PQgetvalue(res,i,2) << endl;
        cout << "Quantity : " << PQgetvalue(res,i,3) << endl;
        cout << "Price    : Rs." << PQgetvalue(res,i,4) << endl;
        cout << "Date     : " << PQgetvalue(res,i,5) << endl;
        cout << "----------------------------------------\n";
    }

    PQclear(res);
}

// Remove product
void removeProductByAdmin(PGconn* conn)
{
    int productID;

    cout << "\nEnter Product ID to remove: ";
    cin >> productID;

    string id = to_string(productID);

    const char* params[1] = { id.c_str() };

    PGresult* res = PQexecParams(
        conn,
        "DELETE FROM products WHERE product_id=$1;",
        1,
        NULL,
        params,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        cout << "\nProduct Removed Successfully!\n";
    else
        cout << "\nError: " << PQerrorMessage(conn) << endl;

    PQclear(res);
}

// Admin Dashboard
void adminDashboard(PGconn* conn)
{
    int choice;

    while (true)
    {
        cout << "\n=================================\n";
        cout << "        ADMIN DASHBOARD\n";
        cout << "=================================\n";
        cout << "1. View Users\n";
        cout << "2. View All Orders\n";
        cout << "3. Remove Products\n";
        cout << "4. Logout\n";
        cout << "=================================\n";
        cout << "Enter choice: ";

        cin >> choice;

        switch (choice)
        {
            case 1:
                viewUsers(conn);
                break;

            case 2:
                viewAllOrders(conn);
                break;

            case 3:
                removeProductByAdmin(conn);
                break;

            case 4:
                cout << "\nAdmin Logged Out!\n";
                return;

            default:
                cout << "\nInvalid Choice!\n";
        }
    }
}