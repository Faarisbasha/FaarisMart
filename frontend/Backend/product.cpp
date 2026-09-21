#include <iostream>
#include <string>
#include "product.h"

using namespace std;

// Display all products
void viewProducts(PGconn* conn)
{
    PGresult* res = PQexec(
        conn,
        "SELECT product_id, product_name, description, price, stock FROM products ORDER BY product_id;"
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        cout << "Error: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return;
    }

    cout << "\n=========================================================\n";
    cout << "              FAARISMART PRODUCT LIST\n";
    cout << "=========================================================\n";

    for (int i = 0; i < PQntuples(res); i++)
    {
        cout << "Product ID : " << PQgetvalue(res, i, 0) << endl;
        cout << "Name       : " << PQgetvalue(res, i, 1) << endl;
        cout << "Description: " << PQgetvalue(res, i, 2) << endl;
        cout << "Price      : Rs. " << PQgetvalue(res, i, 3) << endl;
        cout << "Stock      : " << PQgetvalue(res, i, 4) << endl;
        cout << "---------------------------------------------------------\n";
    }

    PQclear(res);
}

// Search Products by Name
void searchProducts(PGconn* conn)
{
    string keyword;

    cout << "\nEnter Product Name: ";

    cin.ignore();
    getline(cin, keyword);

    string search = "%" + keyword + "%";

    const char* params[1] =
    {
        search.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        "SELECT product_id, product_name, description, price, stock "
        "FROM products WHERE LOWER(product_name) LIKE LOWER($1) "
        "ORDER BY product_id;",
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

    cout << "\n=========================================================\n";
    cout << "              SEARCH RESULTS\n";
    cout << "=========================================================\n";

    if (PQntuples(res) == 0)
    {
        cout << "No products found.\n";
        PQclear(res);
        return;
    }

    for (int i = 0; i < PQntuples(res); i++)
    {
        cout << "Product ID : " << PQgetvalue(res, i, 0) << endl;
        cout << "Name       : " << PQgetvalue(res, i, 1) << endl;
        cout << "Description: " << PQgetvalue(res, i, 2) << endl;
        cout << "Price      : Rs. " << PQgetvalue(res, i, 3) << endl;
        cout << "Stock      : " << PQgetvalue(res, i, 4) << endl;
        cout << "---------------------------------------------------------\n";
    }

    PQclear(res);
}

// Filter Products by Category
void filterProductsByCategory(PGconn* conn)
{
    int choice;
    string category;

    cout << "\n=================================\n";
    cout << "      FILTER BY CATEGORY\n";
    cout << "=================================\n";
    cout << "1. Electronics\n";
    cout << "2. Fashion\n";
    cout << "3. Footwear\n";
    cout << "4. Accessories\n";
    cout << "=================================\n";
    cout << "Enter choice: ";
    cin >> choice;

    switch (choice)
    {
        case 1:
            category = "Electronics";
            break;

        case 2:
            category = "Fashion";
            break;

        case 3:
            category = "Footwear";
            break;

        case 4:
            category = "Accessories";
            break;

        default:
            cout << "\nInvalid Category!\n";
            return;
    }

    const char* params[1] =
    {
        category.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        "SELECT product_id, product_name, description, price, stock "
        "FROM products WHERE category=$1 ORDER BY product_id;",
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

    cout << "\n===========================================\n";
    cout << "Category : " << category << endl;
    cout << "===========================================\n";

    if (PQntuples(res) == 0)
    {
        cout << "No products found.\n";
        PQclear(res);
        return;
    }

    for (int i = 0; i < PQntuples(res); i++)
    {
        cout << "Product ID : " << PQgetvalue(res, i, 0) << endl;
        cout << "Name       : " << PQgetvalue(res, i, 1) << endl;
        cout << "Description: " << PQgetvalue(res, i, 2) << endl;
        cout << "Price      : Rs. " << PQgetvalue(res, i, 3) << endl;
        cout << "Stock      : " << PQgetvalue(res, i, 4) << endl;
        cout << "-------------------------------------------\n";
    }

    PQclear(res);
}