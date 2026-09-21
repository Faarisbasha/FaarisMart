
#ifndef ORDER_H
#define ORDER_H

#include <string>
#include "database.h"

using namespace std;

void saveOrder(PGconn* conn, string username);
void viewOrders(PGconn* conn, string username);

#endif