#ifndef CART_H
#define CART_H

#include <string>
#include "database.h"

using namespace std;

void addToCart(PGconn* conn, string username);
void viewCart(PGconn* conn, string username);
void removeFromCart(PGconn* conn, string username);

#endif