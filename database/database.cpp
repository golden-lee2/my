#include "database.h"
#include "include/mysql.h"
#include <iostream>
#include <string>

bool connectToDatabase(MYSQL& my_sql, const char* host, const char* user, const char* pswd, const char* table, unsigned int port) {
    mysql_init(&my_sql);
    if (!mysql_real_connect(&my_sql, host, user, pswd, table, port, NULL, 0)) {
        std::cerr << "connect fault: " << mysql_error(&my_sql) << std::endl;
        return false;
    } else {
        std::cout << "connect to " << table << std::endl;
        return true;
    }
}
std::string getUsername(unsigned int user_id, MYSQL& my_sql) {
    std::string username;

    // Construct the SQL query to retrieve username
    std::string query = "SELECT username FROM users WHERE uid = '" + std::to_string(user_id) + "'";

    // Execute the query
    if (mysql_query(&my_sql, query.c_str()) == 0) {
        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                username = row[0]; // Assuming username is in the first column
            }
            mysql_free_result(result);
        } else {
           
            username = "Unknown"; // Example default value
        }
    } else {
       
        username = "Error"; // Example default value
    }

    return username;
}

std::string getUserwallet(unsigned int user_id, MYSQL& my_sql) {
    std::string a;

    std::string query = "SELECT wallet FROM users WHERE uid = '" + std::to_string(user_id) + "'";

    
    if (mysql_query(&my_sql, query.c_str()) == 0) {
        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                a = row[0]; // Assuming username is in the first column
            }
            mysql_free_result(result);
        } else {
           
            a = "0.00"; // Example default value
        }
    } else {
       
        a = "Error"; // Example default value
    }

    return a;
}

std::string getUsercontact(unsigned int user_id, MYSQL& my_sql) {
    std::string a;

    std::string query = "SELECT contact FROM users WHERE uid = '" + std::to_string(user_id) + "'";

    
    if (mysql_query(&my_sql, query.c_str()) == 0) {
        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                a = row[0]; // Assuming username is in the first column
            }
            mysql_free_result(result);
        } else {
           
            a = "Unknown"; // Example default value
        }
    } else {
       
        a = "Error"; // Example default value
    }

    return a;
}

std::string getUseraddress(unsigned int user_id, MYSQL& my_sql) {
    std::string a;

    std::string query = "SELECT address FROM users WHERE uid = '" + std::to_string(user_id) + "'";

    
    if (mysql_query(&my_sql, query.c_str()) == 0) {
        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                a = row[0]; // Assuming username is in the first column
            }
            mysql_free_result(result);
        } else {
           
            a = "Unknown"; // Example default value
        }
    } else {
       
        a = "Error"; // Example default value
    }

    return a;
}

std::string getUserschool(unsigned int user_id, MYSQL& my_sql) {
    std::string a;

    std::string query = "SELECT school FROM users WHERE uid = '" + std::to_string(user_id) + "'";

    
    if (mysql_query(&my_sql, query.c_str()) == 0) {
        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                a = row[0]; // Assuming username is in the first column
            }
            mysql_free_result(result);
        } else {
           
            a = "Unknown"; // Example default value
        }
    } else {
       
        a = "Error"; // Example default value
    }

    return a;
}

std::string getProductsseller_id(unsigned int pid, MYSQL& my_sql){
     std::string a;

    std::string query = "SELECT seller_id FROM products WHERE pid = '" + std::to_string(pid) + "'";

    
    if (mysql_query(&my_sql, query.c_str()) == 0) {
        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                a = row[0]; // Assuming username is in the first column
            }
            mysql_free_result(result);
        } else {
           
            a = "Unknown"; // Example default value
        }
    } else {
       
        a = "Error"; // Example default value
    }

    return a;

}

std::string getProductsprice(unsigned int pid, MYSQL& my_sql){
     std::string a;

    std::string query = "SELECT price FROM products WHERE pid = '" + std::to_string(pid) + "'";

    
    if (mysql_query(&my_sql, query.c_str()) == 0) {
        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                a = row[0]; // Assuming username is in the first column
            }
            mysql_free_result(result);
        } else {
           
            a = "Unknown"; // Example default value
        }
    } else {
       
        a = "Error"; // Example default value
    }

    return a;

}

int performTransaction(int buyerUid, int productId, MYSQL& my_sql) {
    
    double buyerWallet=std::stod (getUserwallet(buyerUid,my_sql));
    int seller_id = std::stoi( getProductsseller_id(productId,my_sql));
    double sellerWallet=std::stod( getUserwallet(seller_id,my_sql));
    double price=std::stod( getProductsprice(productId, my_sql));
   
    std::string query;
    if (buyerWallet >= price) {
        // 更新购买者的钱包
        query = "UPDATE users SET wallet = wallet - " + std::to_string(price) + " WHERE uid = " + std::to_string(buyerUid);
        if (mysql_query(&my_sql, query.c_str())) {
            std::cerr << "Error updating buyer's wallet: " << mysql_error(&my_sql) << std::endl;
            return 0;
        }
        mysql_commit(&my_sql); // 确保提交更改

        // 更新卖家的钱包
        query = "UPDATE users SET wallet = wallet + " + std::to_string(price) + " WHERE uid = " + std::to_string(seller_id);
        if (mysql_query(&my_sql, query.c_str())) {
            std::cerr << "Error updating seller's wallet: " << mysql_error(&my_sql) << std::endl;
            return 0;
        }
        mysql_commit(&my_sql);
        // 更新商品状态
        query = "UPDATE products SET status = 'sold' WHERE pid = " + std::to_string(productId);
        if (mysql_query(&my_sql, query.c_str())) {
            std::cerr << "Error updating products's status: " << mysql_error(&my_sql) << std::endl;
            return 0;
        }
        mysql_commit(&my_sql);

        // 插入新订单
       
        query = "INSERT INTO OrderClass ( Buyer_id, Seller_id, Product_id, Status) VALUES (" +
                 std::to_string(buyerUid) + ", " + std::to_string(seller_id) +
                ", " + std::to_string(productId) + ", 'Incomplete')";
        if (mysql_query(&my_sql, query.c_str())) {
            std::cerr << "Error inserting new order: " << mysql_error(&my_sql) << std::endl;
            return 0;
        }
        mysql_commit(&my_sql);
       
    } else {
        std::cerr << "Transaction failed! Buyer does not have enough funds." << std::endl;
        return 0;
    }



    return 1;
       
    
}


std::string getProductname(unsigned int pid, MYSQL& my_sql){
     std::string a;

    std::string query = "SELECT productName FROM products WHERE pid = '" + std::to_string(pid) + "'";

    
    if (mysql_query(&my_sql, query.c_str()) == 0) {
        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                a = row[0]; // Assuming username is in the first column
            }
            mysql_free_result(result);
        } else {
           
            a = "Unknown"; // Example default value
        }
    } else {
       
        a = "Error"; // Example default value
    }

    return a;

}

std::string getProductpicture(unsigned int pid, MYSQL& my_sql){
     std::string a;

    std::string query = "SELECT picture FROM products WHERE pid = '" + std::to_string(pid) + "'";

    
    if (mysql_query(&my_sql, query.c_str()) == 0) {
        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                a = row[0]; // Assuming username is in the first column
            }
            mysql_free_result(result);
        } else {
           
            a = "Unknown"; // Example default value
        }
    } else {
       
        a = "Error"; // Example default value
    }

    return a;

}

std::string getProductdescription(unsigned int pid, MYSQL& my_sql){
     std::string a;

    std::string query = "SELECT description FROM products WHERE pid = '" + std::to_string(pid) + "'";

    
    if (mysql_query(&my_sql, query.c_str()) == 0) {
        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                a = row[0]; // Assuming username is in the first column
            }
            mysql_free_result(result);
        } else {
           
            a = "Unknown"; // Example default value
        }
    } else {
       
        a = "Error"; // Example default value
    }

    return a;

}


std::string getProductstatus(unsigned int pid, MYSQL& my_sql){
     std::string a;

    std::string query = "SELECT status FROM products WHERE pid = '" + std::to_string(pid) + "'";

    
    if (mysql_query(&my_sql, query.c_str()) == 0) {
        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                a = row[0]; // Assuming username is in the first column
            }
            mysql_free_result(result);
        } else {
           
            a = "Unknown"; // Example default value
        }
    } else {
       
        a = "Error"; // Example default value
    }

    return a;

}