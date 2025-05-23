#include <crow.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <unordered_map>
#include "../database/database.h" // 包含数据库相关的头文件

using namespace std;
std::unordered_map<std::string, unsigned int> session_id;
// 读取html文件
std::string readFile(const std::string& filePath) ;
// 获取用户输入
std::unordered_map<std::string, std::string> parseUrlEncoded(const std::string& body);
// Function to replace placeholders in a string with given values
std::string replacePlaceholder(std::string str, const std::string& placeholder, const std::string& value) ;
std::string generateUniqueFilename() ;

//设置路由
void setupRoutes_init(crow::SimpleApp& app);
void setupRoutes_login(crow::SimpleApp& app,MYSQL &my_sql);
void setupRoutes_register(crow::SimpleApp& app,MYSQL &my_sql);
void setupRoutes_welcome(crow::SimpleApp& app,MYSQL &my_sql);
void setupRoutes_userinfo(crow::SimpleApp& app,MYSQL &my_sql);
void setupRoutes_image(crow::SimpleApp& app,MYSQL &my_sql);
void setupRoutes_post_product(crow::SimpleApp& app,MYSQL &my_sql);
void setupRoutes_uploads(crow::SimpleApp& app,MYSQL &my_sql);
void setupRoutes_product_details(crow::SimpleApp& app, MYSQL& my_sql);
void setupRoutes_product_payment(crow::SimpleApp& app, MYSQL& my_sql);
void setupRoutes_recharge(crow::SimpleApp& app, MYSQL& my_sql);
void setupRoutes_my_products(crow::SimpleApp& app, MYSQL& my_sql);
void setupRoutes_my_orders(crow::SimpleApp& app, MYSQL& my_sql);
void setupRoutes_confirm_received(crow::SimpleApp& app, MYSQL& my_sql);

int server_main() {
    //连接数据库
    const char host[] = "localhost";
    const char user[] = "root";
    const char pswd[] = "111111";
    const char table[] = "second_hand_transactions";
    unsigned int port = 3306;
    MYSQL my_sql;
    if (!connectToDatabase(my_sql, host, user, pswd, table, port)) {
        cerr << "Failed to connect to database." << endl;
        return 1;
    }

    //设置路由
    crow::SimpleApp app;    
   
    setupRoutes_init(app);
    setupRoutes_login(app,my_sql);
    setupRoutes_register(app,my_sql);
    setupRoutes_welcome(app,my_sql);
    setupRoutes_userinfo(app,my_sql);
    setupRoutes_image(app,my_sql);
    setupRoutes_post_product(app,my_sql);
    setupRoutes_uploads(app,my_sql);
    setupRoutes_product_details(app, my_sql);
    setupRoutes_product_payment(app,my_sql);
    setupRoutes_recharge(app,my_sql);
    setupRoutes_my_products(app, my_sql);
    setupRoutes_my_orders(app, my_sql);
    setupRoutes_confirm_received(app,  my_sql);
    // 设置端口并运行应用程序
    app.port(8080).multithreaded().run();

    //关闭数据库
    mysql_close(&my_sql);

    return 0;
}


//函数实现
std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::unordered_map<std::string, std::string> parseUrlEncoded(const std::string& body) {
    std::unordered_map<std::string, std::string> data;
    std::istringstream ss(body);
    std::string token;
    
    while (std::getline(ss, token, '&')) {
        auto delimiterPos = token.find('=');
        auto key = token.substr(0, delimiterPos);
        auto value = token.substr(delimiterPos + 1);
        data[key] = value;
    }
    
    return data;
}

std::string replacePlaceholder(std::string str, const std::string& placeholder, const std::string& value) {
    size_t pos = 0;
    while ((pos = str.find(placeholder, pos)) != std::string::npos) {
        str.replace(pos, placeholder.length(), value);
        pos += value.length();
    }
    return str;
}
std::string generateUniqueFilename() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S");
    ss << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    return ss.str()+".jpg";
}

void setupRoutes_init(crow::SimpleApp& app){
    CROW_ROUTE(app, "/")
    ([] {
        auto html = readFile("./front/login.html");
        return crow::response(html);
    });
}

void setupRoutes_login(crow::SimpleApp& app, MYSQL& my_sql) {
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)
    ([&my_sql](const crow::request& req) {
        auto data = parseUrlEncoded(req.body);
        auto username = data["username"];
        auto password = data["password"];

        // Query database to validate user credentials
        std::string query = "SELECT uid FROM users WHERE username='" + username + "' AND password='" + password + "'";
        mysql_query(&my_sql, query.c_str());
        MYSQL_RES* result = mysql_store_result(&my_sql);
        int num_rows = mysql_num_rows(result);

        if (num_rows == 1) {
            MYSQL_ROW row = mysql_fetch_row(result);
            unsigned int current_user_id = std::stoi(row[0]);  
            session_id[req.get_header_value("remote_addr")] = current_user_id;

            // Redirect to welcome page with current_user_id
            crow::response res;
            res.code = 302;
            res.set_header("Location", "/welcome");
            mysql_free_result(result); // Free result before returning
            return res;
        } else {
            auto html = readFile("./front/login.html");
            html += "<p>Invalid username or password. Please try again.</p>";
            mysql_free_result(result); // Free result before returning
            return crow::response(html);
        }
    });
}






void setupRoutes_register(crow::SimpleApp& app,MYSQL &my_sql){
    // Route to serve the registration page
    CROW_ROUTE(app, "/register")
    ([] {
        auto html = readFile("./front/register.html");
        return crow::response(html);
    });

    // Route to handle registration form submission
    CROW_ROUTE(app, "/register").methods(crow::HTTPMethod::POST)
    ([&my_sql](const crow::request& req) { // Capture my_sql by reference
        auto data = parseUrlEncoded(req.body);
        auto username = data["username"];
        auto password = data["password"];
        auto school = data["school"];
        auto contact = data["contact"];

        // Check if any required field is empty
        if (username.empty() || password.empty() || school.empty() || contact.empty()) {
            auto html = readFile("./front/register.html");
            html += "<p>All fields are required. Please fill in all fields.</p>";
            return crow::response(html);
        }

        // Query to check if the username already exists
        mysql_query(&my_sql, ("SELECT * FROM users WHERE username='" + username + "'").c_str());
        MYSQL_RES *result = mysql_store_result(&my_sql);
        int num_rows = mysql_num_rows(result);

        if (num_rows == 0) {
            // Insert the new user into the database
            std::string insert_query = "INSERT INTO users (username, password, school, contact) VALUES ('" +
                                    username + "', '" + password + "', '" + school + "', '" + contact + "')";
            mysql_query(&my_sql, insert_query.c_str());
            
            auto html = readFile("./front/login.html");
            html += "<p>Registration successful! Please log in.</p>";
            mysql_free_result(result); // Free result before returning
            return crow::response(html);
        } else {
            auto html = readFile("./front/register.html");
            html += "<p>Username already taken. Please choose another one.</p>";
            mysql_free_result(result); // Free result before returning
            return crow::response(html);
        }
    });

}

void setupRoutes_welcome(crow::SimpleApp& app,MYSQL &my_sql){
     CROW_ROUTE(app, "/welcome")
    ([&my_sql](const crow::request& req) {
        auto html = readFile("./front/welcome.html");
        return crow::response(html);
       
    });
    

    CROW_ROUTE(app, "/products").methods(crow::HTTPMethod::GET)
    ([&my_sql](const crow::request& req) {
        crow::json::wvalue products_json;

        std::string query = "SELECT pid,productName, picture, price ,status,seller_id FROM products";
        if (mysql_query(&my_sql, query.c_str()) != 0) {
            return crow::response(500, "Database query failed");
        }

        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result == nullptr) {
            return crow::response(500, "Database store result failed");
        }

        auto it = session_id.find(req.get_header_value("remote_addr"));       
        unsigned int current_user_id = it->second;

        MYSQL_ROW row;
        int index = 0;
        while ((row = mysql_fetch_row(result))) {
            int productId = row[0] ? std::stoi(row[0]) : 0;
            std::string productName = row[1] ? row[1] : "";
            std::string picture = row[2] ? row[2] : "";
            std::string price = row[3] ? row[3] : "";
            std::string status = row[4] ? row[4] : "";
            int seller_id = row[5] ? std::stoi(row[5]) : 0;
            if(getUserschool(current_user_id,my_sql)!=getUserschool(seller_id,my_sql)) continue;
            if(status=="sold") continue;
          
            products_json["products"][index]["product_id"] = productId;
            products_json["products"][index]["productName"] = productName;
            products_json["products"][index]["picture"] = picture;
            products_json["products"][index]["price"] = price;
            index++;
        }
        mysql_free_result(result);

        return crow::response(products_json);
    });
}

void setupRoutes_userinfo(crow::SimpleApp& app, MYSQL& my_sql) {
    // 提供用户信息页面
    CROW_ROUTE(app, "/userinfo")
    ([&my_sql](const crow::request& req) {
        crow::response res;

        // 根据客户端 IP 从 session_id 中检索 current_user_id
        auto it = session_id.find(req.get_header_value("remote_addr"));
        if (it != session_id.end()) {
            unsigned int current_user_id = it->second;

            

            // 读取并准备包含用户信息的 HTML 响应
            auto html = readFile("./front/userinfo.html");
            html = replacePlaceholder(html, "{{ username }}", getUsername(current_user_id, my_sql));
            html = replacePlaceholder(html, "{{ school }}", getUserschool(current_user_id, my_sql));
            html = replacePlaceholder(html, "{{ contact }}", getUsercontact(current_user_id, my_sql));
            html = replacePlaceholder(html, "{{ wallet }}", getUserwallet(current_user_id, my_sql));
            html = replacePlaceholder(html, "{{ address }}", getUseraddress(current_user_id, my_sql));
            html = replacePlaceholder(html, "{{ uid }}", std::to_string(current_user_id));

            res.write(html);
        } else {
            // 会话未找到或已过期
            res.code = 404;
            res.write("Session expired or user not found.");
        }

        return res;
    });

    // 处理用户信息更新
    CROW_ROUTE(app, "/userinfo").methods(crow::HTTPMethod::POST)
    ([&my_sql](const crow::request& req) {
        
        std::unordered_map<std::string, std::string> form_data;
        // 解析表单数据
        std::istringstream iss(req.body);
        std::string pair;
        while (std::getline(iss, pair, '&')) {
            size_t pos = pair.find('=');
            if (pos != std::string::npos) {
                std::string key = pair.substr(0, pos);
                std::string value = pair.substr(pos + 1);
                form_data[key] = value;
            }
        }
        // 获取 school 和 contact 数据
        std::string school = form_data["school"];
        std::string contact = form_data["contact"];
        std::string address = form_data["address"];
        

        // 根据客户端 IP 从 session_id 中检索 current_user_id
        auto it = session_id.find(req.get_header_value("remote_addr"));
        if (it != session_id.end()) {
            
            unsigned int current_user_id = it->second;           

            // 更新用户信息
            std::string query = "UPDATE users SET school = '" + school + "', contact = '" + contact + "', address = '" + address +"' WHERE uid = '" + std::to_string(current_user_id) + "'";
            if (mysql_query(&my_sql, query.c_str()) == 0) {
                auto html = readFile("./front/userinfo.html");
                html = replacePlaceholder(html, "{{ username }}", getUsername(current_user_id, my_sql)); // Assuming getUsername retrieves username from database
                html = replacePlaceholder(html, "{{ school }}", school);
                html = replacePlaceholder(html, "{{ contact }}", contact);
                html = replacePlaceholder(html, "{{ address }}", address);
                html = replacePlaceholder(html, "{{ wallet }}", getUserwallet(current_user_id, my_sql));
                html += "<p>Information updated successfully..</p>";

                return crow::response(html);
            } else {
                return crow::response(500, "Failed to update information.");
            }
        } else {
            // 会话未找到或已过期
            return crow::response(401, "Session expired or user not found.");
        }
    });
}







void setupRoutes_image(crow::SimpleApp& app,MYSQL &my_sql){
    CROW_ROUTE(app, "/image/<string>")
    ([](const crow::request& req, std::string filename) {
        std::string path = "./front/image/" + filename;
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            // 如果找不到文件，则返回404
            return crow::response(404);
        }

        // 读取文件内容
        std::ostringstream file_contents;
        file_contents << file.rdbuf();

        // 根据文件扩展名设置响应的内容类型
        crow::response res;
        res.set_header("Content-Type", "image/jpg"); // 根据实际文件类型调整内容类型
        res.write(file_contents.str());
        return res;
    });
}

void setupRoutes_post_product(crow::SimpleApp& app, MYSQL& my_sql) {
    // 提供发布商品页面
    CROW_ROUTE(app, "/post_product")
    ([] {
        auto html = readFile("./front/post_product.html");
        return crow::response(html);
    });

    // 处理商品发布
    CROW_ROUTE(app, "/post_product").methods(crow::HTTPMethod::POST)
    ([&my_sql](const crow::request& req) {

        crow::multipart::message msg(req);

        std::string productName;
        std::string description;
        std::string price;
        std::string filename = generateUniqueFilename();
        productName=msg.get_part_by_name("productName").body;
        description=msg.get_part_by_name("description").body;
        price=msg.get_part_by_name("price").body;
        // 处理图片
        std::string picture = msg.get_part_by_name("picture").body;
        std::filesystem::path upload_dir("./uploads/");
        if (!std::filesystem::exists(upload_dir)) {
            std::filesystem::create_directories(upload_dir);
        }
        std::string image_file_path = (upload_dir / filename).string();
        std::ofstream ofs(image_file_path, std::ios::binary);
        ofs.write(picture.data(), picture.size());
        ofs.close();


        auto it = session_id.find(req.get_header_value("remote_addr"));
        if (it != session_id.end()) {
            unsigned int current_user_id = it->second;

            // 插入商品到数据库
            std::string query = "INSERT INTO products (productName, picture, description, price, seller_id) VALUES ('" +
                    productName + "', '" + image_file_path + "', '" + description + "', '" + price + "', '" + std::to_string(current_user_id) + "')";

            if (mysql_query(&my_sql, query.c_str()) == 0) {
                return crow::response(500, "success");
            } else {
                return crow::response(200, "fault");
            }
        } else {
            // 会话未找到或已过期
            return crow::response(401, "401");
        }
    });
}

void setupRoutes_uploads(crow::SimpleApp& app,MYSQL &my_sql){
    CROW_ROUTE(app, "/uploads/<path>")
    ([](const crow::request& req, crow::response& res, std::string path) {
        auto file_path = "./uploads/" + path;
        std::ifstream in(file_path.c_str(), std::ifstream::in | std::ifstream::binary);
        if (in) {
            std::ostringstream contents;
            contents << in.rdbuf();
            in.close();
            res.code = 200;
            res.set_header("Content-Type", "image/jpeg");
            res.write(contents.str());
            res.end();
        } else {
            res.code = 404;
            res.end();
        }
    });
}

void setupRoutes_product_details(crow::SimpleApp& app, MYSQL& my_sql) {
    CROW_ROUTE(app, "/product_details")
    .methods(crow::HTTPMethod::GET)
    ([&my_sql](const crow::request& req) {
        crow::response res;
        auto params = req.url_params;
        std::string pid_str = params.get("pid");
        if (pid_str.empty()) {
            res.code = 400;
            res.write("Product ID (pid) not provided.");
            return res;
        }
        // Convert pid to integer
        int pid = 0;
        try {
            pid = std::stoi(pid_str);
        } catch (const std::exception& e) {
            res.code = 400;
            res.write("Invalid Product ID (pid).");
            return res;
        }
        
            std::string productName = getProductname(pid,my_sql);
            std::string description = getProductdescription(pid,my_sql);
            std::string price = getProductsprice(pid,my_sql);
            std::string picturePath = getProductpicture(pid,my_sql);
            int sellerid = std::stoi(getProductsseller_id(pid,my_sql));
            std::string sellername = getUsername(sellerid,my_sql);
            std::string seller_contact = getUsercontact(sellerid,my_sql);

            auto html = readFile("./front/product_details.html");                             
            html = replacePlaceholder(html,"{{ productName }}", productName); 
            html = replacePlaceholder(html,"{{ description }}", description);
            html = replacePlaceholder(html,"{{ price }}", price);
            html = replacePlaceholder(html,"{{ picture }}", picturePath);
            html = replacePlaceholder(html,"{{ pid }}", std::to_string(pid));     
            html = replacePlaceholder(html,"{{ sellername }}", sellername);  
            html = replacePlaceholder(html,"{{ seller_contact }}", seller_contact); 

            res.write(html);
        
    
        return res;
    });
}

void setupRoutes_product_payment(crow::SimpleApp& app, MYSQL& my_sql){
    CROW_ROUTE(app, "/payment")
    .methods(crow::HTTPMethod::GET)
    ([&my_sql](const crow::request& req) {
        crow::response res;
        auto params = req.url_params;
        std::string pid_str = params.get("pid");
        if (pid_str.empty()) {
            res.code = 400;
            res.write("Product ID (pid) not provided.");
            return res;
        }
        // Convert pid to integer
        int pid = 0;
        try {
            pid = std::stoi(pid_str);
        } catch (const std::exception& e) {
            res.code = 400;
            res.write("Invalid Product ID (pid).");
            return res;
        }
        std::string query = "SELECT productName, price FROM products WHERE pid = " + std::to_string(pid);
        if (mysql_query(&my_sql, query.c_str())!=0) {
            res.code = 500;
            res.write("Database query failed.");
            return res;
        }

        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (!result) {
            res.code = 500;
            res.write("Failed to retrieve query result.");
            return res;
        }

        MYSQL_ROW row = mysql_fetch_row(result);
        if (!row) {
            res.code = 404;
            res.write("Product not found.");
            mysql_free_result(result);
            return res;
        }
        auto it = session_id.find(req.get_header_value("remote_addr"));    
        unsigned int current_user_id = it->second; 
        std::string productName = row[0];
        std::string price = row[1];
        mysql_free_result(result);
        auto html = readFile("./front/payment.html");    
        html = replacePlaceholder(html,"{{ productName }}", productName); // Assuming getUsername retrieves username from database
        html = replacePlaceholder(html,"{{ price }}", price);
        html = replacePlaceholder(html,"{{ pid }}", std::to_string(pid));
        html = replacePlaceholder(html,"{{ uid }}", std::to_string(current_user_id));

        res.write(html);       
    
        return res;

    });

    CROW_ROUTE(app, "/confirm_payment")
    .methods(crow::HTTPMethod::POST)
    ([&my_sql](const crow::request& req) {
        auto params = crow::json::load(req.body);
        if (!params) {
            return crow::response(400, "Invalid JSON data.");
        }

        int pid = params["pid"].i();
        int uid = params["uid"].i(); // Assuming you have user ID in the request
        double price = std::stod(params["price"].s());
        
        
        if(performTransaction(uid, pid, my_sql)){
            return crow::response(200, "Payment confirmed.");
        }
        else{
            return crow::response(500, "Failed to update wallet.");
        }
                
    });
}



void setupRoutes_recharge(crow::SimpleApp& app, MYSQL& my_sql){
    CROW_ROUTE(app, "/recharge")
    .methods(crow::HTTPMethod::GET)
    ([&my_sql](const crow::request& req) {
        
        crow::response res;
        
        
        auto html = readFile("./front/recharge.html");           
        res.write(html);          
        return res;

    });

    CROW_ROUTE(app, "/confirm_recharge")
    .methods(crow::HTTPMethod::POST)
    ([&my_sql](const crow::request& req) {
        auto it = session_id.find(req.get_header_value("remote_addr"));    
        unsigned int uid = it->second; 
        auto params = crow::json::load(req.body);
        if (!params) {
            return crow::response(400, "Invalid JSON data.");
        }

        int amount = params["amount"].i();
        
        // Update user's wallet in the database
        std::string query = "UPDATE users SET wallet = wallet + " + std::to_string(amount) + " WHERE uid = " + std::to_string(uid);
        if (mysql_query(&my_sql, query.c_str())) {
            std::cerr << "Error updating wallet: " << mysql_error(&my_sql) << std::endl;
            return crow::response(500, "Failed to update wallet.");
        }
        mysql_commit(&my_sql);

        std::cout << "Recharge successful! UID: " << uid << ", Amount: " << amount << std::endl;
        
        return crow::response(200, "Payment confirmed.");
        
       
    });

}


void setupRoutes_my_products(crow::SimpleApp& app, MYSQL& my_sql) {
    CROW_ROUTE(app, "/my_products")
    ([&my_sql](const crow::request& req) {
        auto html = readFile("./front/my_products.html");
        return crow::response(html);
       
    });

    
    CROW_ROUTE(app, "/my_products_show").methods(crow::HTTPMethod::GET)
    ([&my_sql](const crow::request& req) {
        auto it = session_id.find(req.get_header_value("remote_addr"));    
        unsigned int uid = it->second;

        crow::json::wvalue products_json;
        std::string query = "SELECT pid,productName, picture, price ,status FROM products WHERE seller_id= " + std::to_string(uid);
        if (mysql_query(&my_sql, query.c_str()) != 0) {
            return crow::response(500, "Database query failed");
        }

        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result == nullptr) {
            return crow::response(500, "Database store result failed");
        }

        MYSQL_ROW row;
        int index = 0;
        while ((row = mysql_fetch_row(result))) {
            int productId = row[0] ? std::stoi(row[0]) : 0;
            std::string productName = row[1] ? row[1] : "";
            std::string picture = row[2] ? row[2] : "";
            std::string price = row[3] ? row[3] : "";
            std::string status = row[4] ? row[4] : "";

          
            products_json["products"][index]["product_id"] = productId;
            products_json["products"][index]["productName"] = productName;
            products_json["products"][index]["picture"] = picture;
            products_json["products"][index]["price"] = price;
            products_json["products"][index]["status"] = status;

            index++;
        }
        mysql_free_result(result);

        return crow::response(products_json);
    });
}


void setupRoutes_my_orders(crow::SimpleApp& app, MYSQL& my_sql) {
    // 渲染HTML页面
    CROW_ROUTE(app, "/my_orders")
    ([&my_sql](const crow::request& req) {
        auto html = readFile("./front/my_orders.html");
        return crow::response(html);
    });

    // 获取订单数据
    CROW_ROUTE(app, "/my_orders_show").methods(crow::HTTPMethod::GET)
    ([&my_sql](const crow::request& req) {
        auto it = session_id.find(req.get_header_value("remote_addr"));
        if (it == session_id.end()) {
            return crow::response(403, "Unauthorized access");
        }
        unsigned int uid = it->second;

        crow::json::wvalue orders_json;
        std::string query = "SELECT orderNumber, Buyer_id, Seller_id, Product_id, Status FROM OrderClass WHERE Buyer_id = " + std::to_string(uid);
        if (mysql_query(&my_sql, query.c_str()) != 0) {
            return crow::response(500, "Database query failed");
        }

        MYSQL_RES* result = mysql_store_result(&my_sql);
        if (result == nullptr) {
            return crow::response(500, "Database store result failed");
        }

        MYSQL_ROW row;
        int index = 0;
        while ((row = mysql_fetch_row(result))) {
            int orderNumber = row[0] ? std::stoi(row[0]) : 0;
            int Buyer_id = row[1] ? std::stoi(row[1]) : 0;
            int Seller_id = row[2] ? std::stoi(row[2]) : 0;
            int Product_id = row[3] ? std::stoi(row[3]) : 0;
            std::string Status = row[4] ? row[4] : "";

            orders_json["orders"][index]["orderNumber"] = orderNumber;
            orders_json["orders"][index]["Buyer"] = getUsername(Buyer_id,my_sql);
            orders_json["orders"][index]["Buyer_contact"] = getUsercontact(Buyer_id,my_sql);
            orders_json["orders"][index]["Buyer_address"] = getUseraddress(Buyer_id,my_sql);
            orders_json["orders"][index]["Seller"] = getUsername(Seller_id,my_sql);
            orders_json["orders"][index]["Seller_contact"] = getUsercontact(Seller_id,my_sql);
            orders_json["orders"][index]["Seller_address"] = getUseraddress(Seller_id,my_sql);
            orders_json["orders"][index]["Product"] = getProductname(Product_id,my_sql);
            orders_json["orders"][index]["Status"] = Status;

            index++;
        }
        mysql_free_result(result);

        return crow::response(orders_json);
    });
}


void setupRoutes_confirm_received(crow::SimpleApp& app, MYSQL& my_sql) {    
    CROW_ROUTE(app, "/confirm_received")
    .methods(crow::HTTPMethod::POST)
    ([&my_sql](const crow::request& req) {
        auto it = session_id.find(req.get_header_value("remote_addr"));    
        if (it == session_id.end()) {
            return crow::response(403, "User session not found.");
        }
        
        unsigned int uid = it->second; 
        auto params = crow::json::load(req.body);
        if (!params) {
            return crow::response(400, "Invalid JSON data.");
        }

        int orderNumber = params["orderNumber"].i();
        std::string query = "UPDATE OrderClass SET Status = 'Completed' WHERE orderNumber = " + std::to_string(orderNumber);
        
        if (mysql_query(&my_sql, query.c_str())) {
            std::cerr << "Error updating order status: " << mysql_error(&my_sql) << std::endl;
            crow::json::wvalue response;
            response["success"] = false;
            response["message"] = "Failed to update order status.";
            return crow::response(500, response);
        }

        if (mysql_commit(&my_sql)) {
            std::cerr << "Error committing transaction: " << mysql_error(&my_sql) << std::endl;
            crow::json::wvalue response;
            response["success"] = false;
            response["message"] = "Failed to commit transaction.";
            return crow::response(500, response);
        }
        
        crow::json::wvalue response;
        response["success"] = true;
        response["message"] = "Confirm received.";
        return crow::response(200, response);
    });
}
