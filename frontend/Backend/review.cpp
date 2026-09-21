#include <iostream>
#include <string>
#include "review.h"

using namespace std;

// Add Review
void addReview(PGconn* conn, string username)
{
    int productID, rating;
    string review;

    cout << "\n========== WRITE REVIEW ==========\n";

    cout << "Enter Product ID: ";
    cin >> productID;

    cout << "Rate this product (1-5): ";
    cin >> rating;

    if (rating < 1 || rating > 5)
    {
        cout << "Invalid rating! Enter between 1 and 5.\n";
        return;
    }

    cin.ignore();

    cout << "Write your review: ";
    getline(cin, review);

    string idStr = to_string(productID);
    string ratingStr = to_string(rating);

    const char* params[4] =
    {
        username.c_str(),
        idStr.c_str(),
        ratingStr.c_str(),
        review.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        "INSERT INTO reviews(username,product_id,rating,review_text)"
        "VALUES($1,$2,$3,$4);",
        4,
        NULL,
        params,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        cout << "\nReview Submitted Successfully!\n";
    else
        cout << "\nError: " << PQerrorMessage(conn) << endl;

    PQclear(res);
}

// View Product Reviews
void viewProductReviews(PGconn* conn, int productID)
{
    string idStr = to_string(productID);

    const char* params[1] =
    {
        idStr.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        "SELECT username,rating,review_text,review_date "
        "FROM reviews WHERE product_id=$1 "
        "ORDER BY review_date DESC;",
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

    cout << "\n========== PRODUCT REVIEWS ==========\n";

    if (PQntuples(res) == 0)
    {
        cout << "No reviews yet.\n";
        PQclear(res);
        return;
    }

    for (int i = 0; i < PQntuples(res); i++)
    {
        cout << "User   : " << PQgetvalue(res, i, 0) << endl;
        cout << "Rating : " << PQgetvalue(res, i, 1) << "/5" << endl;
        cout << "Review : " << PQgetvalue(res, i, 2) << endl;
        cout << "Date   : " << PQgetvalue(res, i, 3) << endl;
        cout << "-------------------------------------\n";
    }

    PQclear(res);
}