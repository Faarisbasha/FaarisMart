
#ifndef CHECKOUT_H
#define CHECKOUT_H

#include <string>
#include "database.h"

using namespace std;

void checkout(PGconn* conn, string username);

#endif