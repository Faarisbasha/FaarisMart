#include <iostream>
#include <string>
#include <conio.h>
#include "database.h"
#include "product.h"
#include "cart.h"
#include "checkout.h"
#include "admin.h"
#include "seller.h"
#include "order.h"
#include "review.h"

using namespace std;

// Hide password with *
string getPassword()
{
    string password = "";
    char ch;

    while ((ch = _getch()) != 13)
    {
        if (ch == 8)
        {
            if (!password.empty())
            {
                password.pop_back();
                cout << "\b \b";
            }
        }
        else
        {
            password += ch;
            cout << "*";
        }
    }

    cout << endl;
    return password;
}

// Login using PostgreSQL
bool loginUser(PGconn* conn, string username, string password)
{
    const char* params[2] =
    {
        username.c_str(),
        password.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        "SELECT * FROM users WHERE username=$1 AND password=$2;",
        2,
        NULL,
        params,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        cout << "Query Error: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return false;
    }

    bool success = (PQntuples(res) > 0);

    PQclear(res);
    return success;
}

// Get user role
string getUserRole(PGconn* conn, string username)
{
    const char* params[1] = { username.c_str() };

    PGresult* res = PQexecParams(
        conn,
        "SELECT role FROM users WHERE username=$1;",
        1,
        NULL,
        params,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0)
    {
        PQclear(res);
        return "buyer";
    }

    string role = PQgetvalue(res, 0, 0);
    PQclear(res);
    return role;
}

// Register
bool registerUser(PGconn* conn)
{
    string name, username, password;

    cout << "\n========== REGISTER ==========\n";

    cout << "Enter Name: ";
    cin.ignore();
    getline(cin, name);

    cout << "Enter Username: ";
    cin >> username;

    cout << "Enter Password: ";
    password = getPassword();

    const char* params[3] =
    {
        name.c_str(),
        username.c_str(),
        password.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        "INSERT INTO users(name, username, password) VALUES($1,$2,$3);",
        3,
        NULL,
        params,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) == PGRES_COMMAND_OK)
    {
        cout << "\nAccount Created Successfully!\n";
        PQclear(res);
        return true;
    }

    cout << "\nRegistration Failed!\n";
    cout << PQerrorMessage(conn) << endl;
    PQclear(res);

    return false;
}

int main()
{
    PGconn* conn = connectDB();

    if (PQstatus(conn) != CONNECTION_OK)
        return 1;

    int choice;

    while (true)
    {
        cout << "\n=================================\n";
        cout << "         FAARISMART\n";
        cout << "=================================\n";
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "3. Exit\n";
        cout << "=================================\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
            case 1:
            {
                string username, password;
                int attempts = 3;

                while (attempts > 0)
                {
                    cout << "\nUsername: ";
                    cin >> username;

                    cout << "Password: ";
                    password = getPassword();

                    if (loginUser(conn, username, password))
                    {
                        cout << "\n=================================\n";
                        cout << "      LOGIN SUCCESSFUL\n";
                        cout << "=================================\n";
                        cout << "Welcome to FaarisMart, " << username << "!\n";

                        string role = getUserRole(conn, username);

                        // ADMIN
                        if (role == "admin")
                        {
                            adminDashboard(conn);
                            break;
                        }

                        // SELLER
                        if (role == "seller")
                        {
                            sellerDashboard(conn, username);
                            break;
                        }

                        // BUYER
                        int dashboardChoice;
                        bool logout = false;

                        while (!logout)
                        {
                            cout << "\n=================================\n";
                            cout << "      CUSTOMER DASHBOARD\n";
                            cout << "=================================\n";
                            cout << "1. View Products\n";
                            cout << "2. Search Products\n";
                            cout << "3. Filter by Category\n";
                            cout << "4. Add To Cart\n";
                            cout << "5. View Cart\n";
                            cout << "6. Remove From Cart\n";
                            cout << "7. Checkout\n";
                            cout << "8. My Orders\n";
                            cout << "9. Write Review\n";
                            cout << "10. View Product Reviews\n";
                            cout << "11. Logout\n";
                            cout << "=================================\n";
                            cout << "Enter choice: ";
                            cin >> dashboardChoice;

                            switch (dashboardChoice)
                            {
                                case 1:
                                    viewProducts(conn);
                                    break;

                                case 2:
                                    searchProducts(conn);
                                    break;

                                case 3:
                                    filterProductsByCategory(conn);
                                    break;

                                case 4:
                                    viewProducts(conn);
                                    addToCart(conn, username);
                                    break;

                                case 5:
                                    viewCart(conn, username);
                                    break;

                                case 6:
                                    removeFromCart(conn, username);
                                    break;

                                case 7:
                                    checkout(conn, username);
                                    break;

                                case 8:
                                    viewOrders(conn, username);
                                    break;

                                case 9:
                                    addReview(conn, username);
                                    break;

                                case 10:
                                {
                                    int productID;
                                    cout << "\nEnter Product ID: ";
                                    cin >> productID;
                                    viewProductReviews(conn, productID);
                                    break;
                                }

                                case 11:
                                    cout << "\nLogged Out Successfully!\n";
                                    logout = true;
                                    break;

                                default:
                                    cout << "\nInvalid Choice!\n";
                            }
                        }

                        break;
                    }

                    attempts--;

                    cout << "\nInvalid Username or Password.\n";

                    if (attempts > 0)
                        cout << "Attempts left: " << attempts << "\n";
                    else
                        cout << "Account Locked!\n";
                }

                break;
            }

            case 2:
            {
                registerUser(conn);
                break;
            }

            case 3:
            {
                cout << "\nThank you for using FaarisMart!\n";
                PQfinish(conn);
                return 0;
            }

            default:
            {
                cout << "\nInvalid Choice! Please try again.\n";
            }
        }
    }

    PQfinish(conn);
    return 0;
}