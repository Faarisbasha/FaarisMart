#ifndef ADMIN_H
#define ADMIN_H

#include "database.h"

void adminDashboard(PGconn* conn);
void viewUsers(PGconn* conn);
void viewAllOrders(PGconn* conn);
void removeProductByAdmin(PGconn* conn);

#endif