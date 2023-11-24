#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>

std::string get_file_contents(const std::string& path) {
    std::ifstream file_stream(path.c_str());

    if (!file_stream) {
        return "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";
    }

    std::string contents((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(contents.size()) + "\r\n\r\n" + contents;
    return response;
}

void handle_client(int client_sock) {
    char buffer[1024] = {0};	
    read(client_sock, buffer, 1024);
    std::string request(buffer);
    std::istringstream request_stream(request);
    std::string method;
    std::string path;
    std::string version;
    // parsing "GET / HTTP/1.1"
    request_stream >> method >> path >> version;

    std::cout << "Path: " << path << std::endl;
    std::cout << "Thread Id: " << std::this_thread::get_id() << std::endl;

    if (path == "/") {
        path = "/index.html";
    }

    std::string http_response = get_file_contents("www" + path);
    send(client_sock, http_response.c_str(), http_response.size(), 0);
    close(client_sock);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    //char buffer[1024] = {0};
    // const char *http_response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    } else {
	std::cout << "Listening on: " << ntohs(address.sin_port) << std::endl;
    }

    while (true) {
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        	perror("accept");
        	exit(EXIT_FAILURE);
    	}
	std::thread client_thread(handle_client, new_socket);
        client_thread.detach();
    }


    return 0;
}

