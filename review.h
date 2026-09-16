#ifndef REVIEW_H
#define REVIEW_H

#include "database.h"
#include <string>

void addReview(PGconn* conn, std::string username);
void viewProductReviews(PGconn* conn, int productID);

#endif