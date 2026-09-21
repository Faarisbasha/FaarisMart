#ifndef PRODUCT_H
#define PRODUCT_H

#include "database.h"
#include <string>

void viewProducts(PGconn* conn);
void searchProducts(PGconn* conn);
void filterProductsByCategory(PGconn* conn);

#endif