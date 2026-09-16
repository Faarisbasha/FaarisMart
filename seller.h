#ifndef SELLER_H
#define SELLER_H

#include "database.h"
#include <string>

void sellerDashboard(PGconn* conn, std::string username);
void addProduct(PGconn* conn, std::string username);
void editProduct(PGconn* conn, std::string username);
void deleteProduct(PGconn* conn, std::string username);
void viewMyProducts(PGconn* conn, std::string username);
void viewSellerOrders(PGconn* conn, std::string username);

#endif