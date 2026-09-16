#include <iostream>
#include <cstdlib>
#include "checkout.h"
#include "order.h"

using namespace std;

void checkout(PGconn* conn, string username)
{
    const char* params[1] = { username.c_str() };

    PGresult* res = PQexecParams(
        conn,
        "SELECT c.product_id, p.product_name, c.quantity, p.price, (c.quantity*p.price) "
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

    if (PQntuples(res) == 0)
    {
        cout << "\nYour cart is empty!\n";
        PQclear(res);
        return;
    }

    cout << "\n=========================================\n";
    cout << "           FAARISMART BILL\n";
    cout << "=========================================\n";
    cout << "Customer : " << username << "\n\n";

    double subtotal = 0;

    for (int i = 0; i < PQntuples(res); i++)
    {
        cout << PQgetvalue(res, i, 1)
             << " x" << PQgetvalue(res, i, 2)
             << "  Rs." << PQgetvalue(res, i, 4) << endl;

        subtotal += atof(PQgetvalue(res, i, 4));
    }

    double gst = subtotal * 0.18;
    double total = subtotal + gst;

    cout << "\n-----------------------------------------\n";
    cout << "Subtotal : Rs." << subtotal << endl;
    cout << "GST (18%): Rs." << gst << endl;
    cout << "-----------------------------------------\n";
    cout << "Grand Total : Rs." << total << endl;

    PQclear(res);

    char confirm;
    cout << "\nConfirm Purchase? (Y/N): ";
    cin >> confirm;

    if (confirm != 'Y' && confirm != 'y')
    {
        cout << "\nCheckout Cancelled.\n";
        return;
    }

    // Reduce stock
    PGresult* updateStock = PQexecParams(
        conn,
        "UPDATE products "
        "SET stock = stock - c.quantity "
        "FROM cart c "
        "WHERE products.product_id = c.product_id "
        "AND c.username = $1;",
        1,
        NULL,
        params,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(updateStock) != PGRES_COMMAND_OK)
    {
        cout << "Stock Update Error: " << PQerrorMessage(conn) << endl;
        PQclear(updateStock);
        return;
    }

    PQclear(updateStock);
// Save order before clearing cart
saveOrder(conn, username);
    // Clear cart
    PGresult* clearCart = PQexecParams(
        conn,
        "DELETE FROM cart WHERE username=$1;",
        1,
        NULL,
        params,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(clearCart) != PGRES_COMMAND_OK)
    {
        cout << "Cart Clear Error: " << PQerrorMessage(conn) << endl;
        PQclear(clearCart);
        return;
    }

    PQclear(clearCart);

    cout << "\n=========================================\n";
    cout << "      PAYMENT SUCCESSFUL\n";
    cout << "=========================================\n";
    cout << "Thank you for shopping with FaarisMart!\n";
}