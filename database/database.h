#ifndef DATABASE_H
#define DATABASE_H

#include "include/mysql.h"
#include <iostream>
#include <string>

bool connectToDatabase(MYSQL& my_sql, const char* host, const char* user, const char* pswd, const char* table, unsigned int port);
std::string getUsername(unsigned int user_id, MYSQL& my_sql);
std::string getUserwallet(unsigned int user_id, MYSQL& my_sql);
std::string getUsercontact(unsigned int user_id, MYSQL& my_sql);
std::string getUseraddress(unsigned int user_id, MYSQL& my_sql);
std::string getUserschool(unsigned int user_id, MYSQL& my_sql);
std::string getProductsseller_id(unsigned int pid, MYSQL& my_sql);
std::string getProductsprice(unsigned int pid, MYSQL& my_sql);
int performTransaction(int buyerUid, int productId, MYSQL& my_sql);
std::string getProductname(unsigned int pid, MYSQL& my_sql);
std::string getProductpicture(unsigned int pid, MYSQL& my_sql);
std::string getProductdescription(unsigned int pid, MYSQL& my_sql);
std::string getProductstatus(unsigned int pid, MYSQL& my_sql);

#endif // DATABASE_H
