
#include <iostream>
#include <string>
#include <limits>
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

// Hide password input with *
string getPassword()
{
    string password;
    char ch;

    while ((ch = _getch()) != '\r' && ch != '\n')
    {
        if (ch == 8) // Backspace
        {
            if (!password.empty())
            {
                password.pop_back();
                cout << "\b \b";
            }
        }
        else if (ch >= 32 && ch <= 126)
        {
            password += ch;
            cout << '*';
        }
    }

    cout << '\n';
    return password;
}

// Check whether a database query succeeded
bool loginUser(PGconn* conn, const string& username,
               const string& password)
{
    if (conn == nullptr ||
        PQstatus(conn) != CONNECTION_OK)
    {
        cerr << "Database connection is unavailable.\n";
        return false;
    }

    const char* params[2] = {
        username.c_str(),
        password.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        "SELECT user_id FROM users "
        "WHERE username = $1 AND password = $2;",
        2,
        nullptr,
        params,
        nullptr,
        nullptr,
        0
    );

    if (res == nullptr ||
        PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        cerr << "Login query failed: "
             << PQerrorMessage(conn) << '\n';

        if (res != nullptr)
            PQclear(res);

        return false;
    }

    bool success = PQntuples(res) > 0;

    PQclear(res);
    return success;
}

// Get the user's role
string getUserRole(PGconn* conn, const string& username)
{
    if (conn == nullptr ||
        PQstatus(conn) != CONNECTION_OK)
    {
        return "buyer";
    }

    const char* params[1] = { username.c_str() };

    PGresult* res = PQexecParams(
        conn,
        "SELECT role FROM users WHERE username = $1;",
        1,
        nullptr,
        params,
        nullptr,
        nullptr,
        0
    );

    if (res == nullptr ||
        PQresultStatus(res) != PGRES_TUPLES_OK ||
        PQntuples(res) == 0)
    {
        if (res != nullptr)
            PQclear(res);

        return "buyer";
    }

    string role = PQgetvalue(res, 0, 0);

    PQclear(res);
    return role;
}

// Register a new user
bool registerUser(PGconn* conn)
{
    if (conn == nullptr ||
        PQstatus(conn) != CONNECTION_OK)
    {
        cerr << "Database connection is unavailable.\n";
        return false;
    }

    string name;
    string username;
    string password;

    cout << "\n========== REGISTER ==========\n";

    cout << "Enter Name: ";
    getline(cin >> ws, name);

    cout << "Enter Username: ";
    getline(cin >> ws, username);

    cout << "Enter Password: ";
    password = getPassword();

    if (name.empty() || username.empty() || password.empty())
    {
        cout << "\nAll fields are required.\n";
        return false;
    }

    const char* params[3] = {
        name.c_str(),
        username.c_str(),
        password.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        "INSERT INTO users(name, username, password) "
        "VALUES($1, $2, $3);",
        3,
        nullptr,
        params,
        nullptr,
        nullptr,
        0
    );

    if (res != nullptr &&
        PQresultStatus(res) == PGRES_COMMAND_OK)
    {
        cout << "\nAccount created successfully!\n";
        PQclear(res);
        return true;
    }

    cout << "\nRegistration failed: "
         << PQerrorMessage(conn) << '\n';

    if (res != nullptr)
        PQclear(res);

    return false;
}

int main()
{
    // Connect using DATABASE_URL through connectDB()
    PGconn* conn = connectDB();

    // IMPORTANT: check nullptr before calling PQstatus()
    if (conn == nullptr)
    {
        cerr << "Could not connect to the database. "
             << "Check DATABASE_URL and try again.\n";
        return 1;
    }

    if (PQstatus(conn) != CONNECTION_OK)
    {
        cerr << "Database connection is not ready.\n";
        PQfinish(conn);
        return 1;
    }

    int choice = 0;

    while (true)
    {
        cout << "\n=================================\n";
        cout << "           FAARISMART\n";
        cout << "=================================\n";
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "3. Exit\n";
        cout << "=================================\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(
                numeric_limits<streamsize>::max(), '\n'
            );

            cout << "\nPlease enter a valid number.\n";
            continue;
        }

        switch (choice)
        {
            case 1:
            {
                string username;
                string password;
                int attempts = 3;
                bool loggedIn = false;

                while (attempts > 0)
                {
                    cout << "\nUsername: ";
                    cin >> username;

                    cout << "Password: ";
                    password = getPassword();

                    if (loginUser(conn, username, password))
                    {
                        loggedIn = true;
                        break;
                    }

                    --attempts;
                    cout << "\nInvalid username or password.\n";

                    if (attempts > 0)
                        cout << "Attempts left: "
                             << attempts << '\n';
                    else
                        cout << "Login attempts exhausted.\n";
                }

                if (!loggedIn)
                    break;

                cout << "\n=================================\n";
                cout << "        LOGIN SUCCESSFUL\n";
                cout << "=================================\n";
                cout << "Welcome to FaarisMart, "
                     << username << "!\n";

                string role = getUserRole(conn, username);

                if (role == "admin")
                {
                    adminDashboard(conn);
                    break;
                }

                if (role == "seller")
                {
                    sellerDashboard(conn, username);
                    break;
                }

                int dashboardChoice = 0;
                bool logout = false;

                while (!logout)
                {
                    cout << "\n=================================\n";
                    cout << "       CUSTOMER DASHBOARD\n";
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

                    if (!(cin >> dashboardChoice))
                    {
                        cin.clear();
                        cin.ignore(
                            numeric_limits<streamsize>::max(),
                            '\n'
                        );

                        cout << "\nEnter a valid menu number.\n";
                        continue;
                    }

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

                            if (cin >> productID)
                            {
                                viewProductReviews(
                                    conn, productID
                                );
                            }
                            else
                            {
                                cin.clear();
                                cin.ignore(
                                    numeric_limits<streamsize>::max(),
                                    '\n'
                                );

                                cout << "Invalid product ID.\n";
                            }

                            break;
                        }

                        case 11:
                            cout << "\nLogged out successfully.\n";
                            logout = true;
                            break;

                        default:
                            cout << "\nInvalid choice. Try again.\n";
                    }
                }

                break;
            }

            case 2:
                registerUser(conn);
                break;

            case 3:
                cout << "\nThank you for using FaarisMart!\n";
                PQfinish(conn);
                return 0;

            default:
                cout << "\nInvalid choice. Please try again.\n";
        }
    }

    PQfinish(conn);
    return 0;
}