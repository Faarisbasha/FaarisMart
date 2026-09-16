#include <iostream>
#include <cstdlib>
#include "cart.h"

using namespace std;

// Add Product to Cart
void addToCart(PGconn* conn, string username)
{
    int productID, quantity;

    cout << "\nEnter Product ID: ";
    cin >> productID;

    cout << "Enter Quantity: ";
    cin >> quantity;

    string productStr = to_string(productID);
    string quantityStr = to_string(quantity);

    // Check if product already exists in cart
    const char* checkParams[2] =
    {
        username.c_str(),
        productStr.c_str()
    };

    PGresult* checkRes = PQexecParams(
        conn,
        "SELECT quantity FROM cart WHERE username=$1 AND product_id=$2;",
        2,
        NULL,
        checkParams,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(checkRes) != PGRES_TUPLES_OK)
    {
        cout << "\nError: " << PQerrorMessage(conn) << endl;
        PQclear(checkRes);
        return;
    }

    if (PQntuples(checkRes) > 0)
    {
        // Product already exists → increase quantity
        const char* updateParams[3] =
        {
            quantityStr.c_str(),
            username.c_str(),
            productStr.c_str()
        };

        PGresult* updateRes = PQexecParams(
            conn,
            "UPDATE cart SET quantity = quantity + $1 WHERE username=$2 AND product_id=$3;",
            3,
            NULL,
            updateParams,
            NULL,
            NULL,
            0
        );

        if (PQresultStatus(updateRes) == PGRES_COMMAND_OK)
            cout << "\nCart Quantity Updated Successfully!\n";
        else
            cout << "\nError: " << PQerrorMessage(conn) << endl;

        PQclear(updateRes);
    }
    else
    {
        // Product doesn't exist → insert new row
        const char* insertParams[3] =
        {
            username.c_str(),
            productStr.c_str(),
            quantityStr.c_str()
        };

        PGresult* insertRes = PQexecParams(
            conn,
            "INSERT INTO cart(username, product_id, quantity) VALUES($1,$2,$3);",
            3,
            NULL,
            insertParams,
            NULL,
            NULL,
            0
        );

        if (PQresultStatus(insertRes) == PGRES_COMMAND_OK)
            cout << "\nProduct Added Successfully!\n";
        else
            cout << "\nError: " << PQerrorMessage(conn) << endl;

        PQclear(insertRes);
    }

    PQclear(checkRes);
}

// View Cart
void viewCart(PGconn* conn, string username)
{
    const char* params[1] = { username.c_str() };

    PGresult* res = PQexecParams(
        conn,
        "SELECT c.product_id,p.product_name,c.quantity,p.price,(c.quantity*p.price) "
        "FROM cart c JOIN products p ON c.product_id=p.product_id "
        "WHERE c.username=$1;",
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

    cout << "\n================== YOUR CART ==================\n";

    if (PQntuples(res) == 0)
    {
        cout << "Your cart is empty.\n";
        PQclear(res);
        return;
    }

    double total = 0;

    for (int i = 0; i < PQntuples(res); i++)
    {
        cout << "Product ID : " << PQgetvalue(res, i, 0) << endl;
        cout << "Name       : " << PQgetvalue(res, i, 1) << endl;
        cout << "Quantity   : " << PQgetvalue(res, i, 2) << endl;
        cout << "Price      : Rs." << PQgetvalue(res, i, 3) << endl;
        cout << "Subtotal   : Rs." << PQgetvalue(res, i, 4) << endl;
        cout << "-----------------------------------------------\n";

        total += atof(PQgetvalue(res, i, 4));
    }

    cout << "TOTAL BILL : Rs." << total << endl;

    PQclear(res);
}

// Remove Product from Cart
void removeFromCart(PGconn* conn, string username)
{
    viewCart(conn, username);

    int productID;

    cout << "\nEnter Product ID to remove: ";
    cin >> productID;

    string productStr = to_string(productID);

    const char* params[2] =
    {
        username.c_str(),
        productStr.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        "DELETE FROM cart WHERE username=$1 AND product_id=$2;",
        2,
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