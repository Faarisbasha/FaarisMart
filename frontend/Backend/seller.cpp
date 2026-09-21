#include <iostream>
#include <string>
#include "seller.h"

using namespace std;

// View Orders Received
void viewSellerOrders(PGconn* conn, string username)
{
    const char* params[1] = { username.c_str() };

    PGresult* res = PQexecParams(
        conn,
        "SELECT o.username,p.product_name,o.quantity,(o.quantity*o.price),o.order_date "
        "FROM orders o "
        "JOIN products p ON o.product_id=p.product_id "
        "WHERE p.seller_username=$1 "
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
    cout << "        ORDERS RECEIVED\n";
    cout << "=========================================\n";

    if (PQntuples(res) == 0)
    {
        cout << "No orders received yet.\n";
        PQclear(res);
        return;
    }

    for (int i = 0; i < PQntuples(res); i++)
    {
        cout << "Buyer    : " << PQgetvalue(res, i, 0) << endl;
        cout << "Product  : " << PQgetvalue(res, i, 1) << endl;
        cout << "Quantity : " << PQgetvalue(res, i, 2) << endl;
        cout << "Amount   : Rs." << PQgetvalue(res, i, 3) << endl;
        cout << "Date     : " << PQgetvalue(res, i, 4) << endl;
        cout << "-----------------------------------------\n";
    }

    PQclear(res);
}

// Seller Dashboard
void sellerDashboard(PGconn* conn, string username)
{
    int choice;

    while (true)
    {
        cout << "\n=================================\n";
        cout << "       SELLER DASHBOARD\n";
        cout << "=================================\n";
        cout << "1. View My Products\n";
        cout << "2. Add Product\n";
        cout << "3. Edit Product Price\n";
        cout << "4. Delete Product\n";
        cout << "5. Orders Received\n";
        cout << "6. Logout\n";
        cout << "=================================\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice)
        {
            // View My Products
            case 1:
            {
                const char* params[1] = { username.c_str() };

                PGresult* res = PQexecParams(
                    conn,
                    "SELECT product_id, product_name, price, stock "
                    "FROM products WHERE seller_username=$1 "
                    "ORDER BY product_id;",
                    1,
                    NULL,
                    params,
                    NULL,
                    NULL,
                    0
                );

                if (PQresultStatus(res) == PGRES_TUPLES_OK)
                {
                    cout << "\n========== MY PRODUCTS ==========\n";

                    if (PQntuples(res) == 0)
                    {
                        cout << "No products found.\n";
                    }

                    for (int i = 0; i < PQntuples(res); i++)
                    {
                        cout << "ID: " << PQgetvalue(res, i, 0)
                             << " | Name: " << PQgetvalue(res, i, 1)
                             << " | Price: Rs." << PQgetvalue(res, i, 2)
                             << " | Stock: " << PQgetvalue(res, i, 3)
                             << endl;
                    }
                }
                else
                {
                    cout << "Error: " << PQerrorMessage(conn) << endl;
                }

                PQclear(res);
                break;
            }

            // Add Product
            case 2:
            {
                string name, description;
                double price;
                int stock;

                cin.ignore();

                cout << "\nProduct Name: ";
                getline(cin, name);

                cout << "Description: ";
                getline(cin, description);

                cout << "Price: ";
                cin >> price;

                cout << "Stock: ";
                cin >> stock;

                string priceStr = to_string(price);
                string stockStr = to_string(stock);

                const char* params[5] =
                {
                    name.c_str(),
                    description.c_str(),
                    priceStr.c_str(),
                    stockStr.c_str(),
                    username.c_str()
                };

                PGresult* res = PQexecParams(
                    conn,
                    "INSERT INTO products(product_name,description,price,stock,seller_username)"
                    "VALUES($1,$2,$3,$4,$5);",
                    5,
                    NULL,
                    params,
                    NULL,
                    NULL,
                    0
                );

                if (PQresultStatus(res) == PGRES_COMMAND_OK)
                    cout << "\nProduct Added Successfully!\n";
                else
                    cout << "\nError: " << PQerrorMessage(conn) << endl;

                PQclear(res);
                break;
            }

            // Edit Product Price
            case 3:
            {
                int productID;
                double newPrice;

                cout << "\nEnter Product ID: ";
                cin >> productID;

                cout << "New Price: ";
                cin >> newPrice;

                string idStr = to_string(productID);
                string priceStr = to_string(newPrice);

                const char* params[3] =
                {
                    priceStr.c_str(),
                    idStr.c_str(),
                    username.c_str()
                };

                PGresult* res = PQexecParams(
                    conn,
                    "UPDATE products SET price=$1 "
                    "WHERE product_id=$2 AND seller_username=$3;",
                    3,
                    NULL,
                    params,
                    NULL,
                    NULL,
                    0
                );

                if (PQresultStatus(res) == PGRES_COMMAND_OK)
                    cout << "\nPrice Updated Successfully!\n";
                else
                    cout << "\nError: " << PQerrorMessage(conn) << endl;

                PQclear(res);
                break;
            }

            // Delete Product
            case 4:
            {
                int productID;

                cout << "\nEnter Product ID to Delete: ";
                cin >> productID;

                string idStr = to_string(productID);

                const char* params[2] =
                {
                    idStr.c_str(),
                    username.c_str()
                };

                PGresult* res = PQexecParams(
                    conn,
                    "DELETE FROM products "
                    "WHERE product_id=$1 AND seller_username=$2;",
                    2,
                    NULL,
                    params,
                    NULL,
                    NULL,
                    0
                );

                if (PQresultStatus(res) == PGRES_COMMAND_OK)
                    cout << "\nProduct Deleted Successfully!\n";
                else
                    cout << "\nError: " << PQerrorMessage(conn) << endl;

                PQclear(res);
                break;
            }

            // Orders Received
            case 5:
            {
                viewSellerOrders(conn, username);
                break;
            }

            // Logout
            case 6:
            {
                cout << "\nSeller Logged Out Successfully!\n";
                return;
            }

            default:
            {
                cout << "\nInvalid Choice!\n";
            }
        }
    }
}