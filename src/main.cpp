#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  std::cout << "Creating server socket ..." << std::endl;

  int socRes = socket(AF_INET, SOCK_STREAM, 0);
  if (socRes == -1) {
    std::cerr << "Socket creation failed";
    return -1;
  }

  struct sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(54000);
  inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

  std::cout << "Biding socket to sockaddr..." << std::endl;
  if (bind(socRes, (struct sockaddr *)&hint.sin_addr, sizeof(hint)) == -1) {
    std::cerr << "IP/port binding failed";
    return -2;
  }

  std::cout << "Mark the socket for listening..." << std::endl;
  if (listen(socRes, SOMAXCONN) == -1) {
    std::cerr << "Listening failed";
    return -3;
  }

  sockaddr_in client;
  socklen_t clientSize = sizeof(client);

  std::cout << "Accept client call..." << std::endl;
  int clientSocket = accept(socRes, (struct sockaddr *)&client, &clientSize);

  std::cout << "Received call..." << std::endl;
  if (clientSocket == -1) {
    std::cerr << "Connection with client failed";
    return -4;
  }

  std::cout << "Client address: " << inet_ntoa(client.sin_addr)
            << "; PORT: " << client.sin_port << ";" << std::endl;

  close(socRes);

  char buffer[4096];
  while (true) {
    //  clear buffer
    memset(buffer, 0, 4096);

    // wait for message
    int bytesRecv = recv(clientSocket, buffer, 4096, 0);
    if (bytesRecv == -1)
      std::cerr << "Connection issue" << std::endl;
    if (bytesRecv == 0)
      std::cout << "Client disconnected" << std::endl;

    // display message
    std::cout << "Received: " << std::string(buffer, 0, bytesRecv);

    // return message
    send(clientSocket, buffer, bytesRecv + 1, 0);
  }

  close(clientSocket);

  return 0;
}
