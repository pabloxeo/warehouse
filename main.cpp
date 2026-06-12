#include <sys/socket.h>   // socket, bind, listen, accept
#include <netinet/in.h>   // struct sockaddr_in, direcciones IP
#include <unistd.h>       // read, write, close
#include <string.h>       // memset
#include <fstream>
#include <vector>
#include <iostream>      // std::cout

std::string get_content_type(const std::string& path) {
    std::string extension = path.substr(path.rfind(".") + 1);
    if (extension == "html") {
        return "text/html";
    } else if (extension == "css") {
        return "text/css";
    } else if (extension == "js") {
        return "application/javascript";
    } else if (extension == "png") {
        return "image/png";
    } else if (extension == "jpg" || extension == "jpeg") {
        return "image/jpeg";
    }
    return "application/octet-stream"; // default
}

int main() {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        return 1;
    }

    listen(server_fd, 10);
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);

        std::cout << "Alguien se conectó! fd=" << client_fd << std::endl;
        char buffer[4096] = {0};
        read(client_fd, buffer, sizeof(buffer));
        //get the path from the request
        std::string ruta = "";
        int start = std::string(buffer).find(" ");
        int end = std::string(buffer).find(" ", start + 1);
        ruta = "public" + std::string(buffer).substr(start + 1, end - start - 1);
        if (ruta == "public/") {
            ruta = "public/index.html";
        }
        std::ifstream file(ruta, std::ios::binary);
        if (!file.is_open()) {
            std::string body = "<h1>404 Not Found</h1>";
            std::string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " 
                                + std::to_string(body.size()) + "\r\n\r\n" + body;
            write(client_fd, response.c_str(), response.size());
            close(client_fd);
            continue;
        }

        std::vector<char> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string header = "HTTP/1.1 200 OK\r\nContent-Type: " + get_content_type(ruta) + "\r\nContent-Length: " + std::to_string(content.size()) + "\r\n\r\n";
        write(client_fd, header.c_str(), header.size());

        
        write(client_fd, content.data(), content.size());
                close(client_fd);
    }


    close(server_fd);
    return 0;
}