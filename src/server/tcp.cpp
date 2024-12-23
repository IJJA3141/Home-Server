#include "../log.hpp"
#include "reqres.hpp"
#include "server.hpp"

#include <thread>
#include <unistd.h>

Tcp::Tcp(const size_t _client_size, const Router *_router) : client_size_(_client_size)
{
  if (_router == nullptr) {
    ERR("Tcp was initialized without a parser");
    exit(1);
  }

  this->state.listening = false;
  this->state.stop = false;

  this->router_ = _router;
  this->client_array_ = new Client *[_client_size];

  LOG("server initialization...");
  LOG("socket initialization...");

  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket_ == -1) {
    VERBERR("socket creation failed.");
    exit(1);
  }

  LOG("setting options...");
  int opt = 1;
  if (setsockopt(this->socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    VERBERR("setting options failed.");
    exit(1);
  }

  return;
}

Tcp::~Tcp()
{
  this->state.stop = true;
  ::close(this->socket_);
  this->thread.join();

  for (int i = 0; i < this->client_size_; i++)
    if (this->client_array_[i] != nullptr) delete this->client_array_[i];

  return;
}

void Tcp::bind(const int _port)
{
  LOG("binding socket to sockaddr on port " << _port << "...");

  this->hint_.sin_family = AF_INET;
  this->hint_.sin_addr.s_addr = htonl(INADDR_ANY);
  this->hint_.sin_port = htons(_port);
  if (::bind(this->socket_, (struct sockaddr *)&this->hint_, sizeof(this->hint_)) == -1) {
    VERBERR("IP/PORT binding failed.");
    exit(1);
  }

  return;
}

void Tcp::listen()
{
  LOG("mark socket for listening...");
  if (::listen(this->socket_, SOMAXCONN) == -1) {
    VERBERR("listening failed.");
    exit(1);
  }

  this->state.listening = true;

  this->thread = std::thread([this]() -> void {
    while (!this->state.stop) {
      Client *incoming_client = this->await_client();
      int i = this->inactive_client_index();

      if (i != -1) {
        this->client_array_[i] = incoming_client;
        incoming_client->thread = new std::thread(&Tcp::connect, this, i);

      } else {
        i = this->clean_client_array();

        if (i != -1) {
          this->client_array_[i] = incoming_client;
          incoming_client->thread = new std::thread(&Tcp::connect, this, i);

        } else {
          incoming_client->send(this->router_->handle_err(Request(Request::Failure::SERVERFULL)));
          delete incoming_client;
        }
      }
    };

    LOG("socket stoped listening...");
    return;
  });

  return;
}

void Tcp::update_client_state()
{
  for (int i = 0; i < this->client_size_; i++) {
    if (this->client_array_[i] != nullptr) {
      if (this->client_array_[i]->state.sleeping)
        this->client_array_[i]->state.dead = true;
      else
        this->client_array_[i]->state.dead = true;
    }
  }

  return;
}

int Tcp::clean_client_array()
{
  int last = -1;

  for (int i = 0; i < this->client_size_; i++) {
    if (this->client_array_[i] != nullptr && this->client_array_[i]->state.dead) {
      this->client_array_[i]->close(); // might cause problemes
      last = i;
    }
  }

  return last;
}

std::string Tcp::client_array_state()
{
  std::string out = "client array: \n{\n";
  int sleeping = 0;
  int running = 0;
  int empty = 0;
  int dead = 0;

  for (int i = 0; i < this->client_size_; i++) {
    out += i;

    if (this->client_array_[i]) {
      out += ": nullptr\n";
      empty++;
    } else {
      if (this->client_array_[i]->state.dead) {
        out += ": dead\n";
        dead++;
      } else if (this->client_array_[i]->state.sleeping) {
        out += ": sleeping\n";
        sleeping++;
      } else {
        out += ": running\n";
        running++;
      }
    }
  }

  (out += "}\nsize: ") += this->client_size_;
  (out += "\nrunning: ") += running;
  (out += "\nsleeping: ") += sleeping;
  (out += "\ndead: ") += dead;
  (out += "\nempty: ") += empty;

  return out;
}

Client *Tcp::await_client() { return new Client(this->socket_); }

void Tcp::connect(const size_t _index)
{
  Request request;
  Response response;

  request = this->client_array_[_index]->read();

  if (request.headers["Connection"] != "keep-alive") {
    response = this->router_->respond(request);
    this->client_array_[_index]->send(response);

    delete this->client_array_[_index];
    this->client_array_[_index] = nullptr;

    return;
  }

  while (request.headers["Connection"] == "keep-alive" &&
         this->client_array_[_index]->state.running) {
    response = this->router_->respond(request);
    this->client_array_[_index]->send(response);

    this->client_array_[_index]->state.sleeping = false;
    this->client_array_[_index]->state.dead = false;

    request = this->client_array_[_index]->read();
  };

  if (this->client_array_[_index]->state.running) {
    response = this->router_->respond(request);
    this->client_array_[_index]->send(response);
  }

  delete this->client_array_[_index];
  this->client_array_[_index] = nullptr;

  return;
};

int Tcp::inactive_client_index()
{
  int i = -1;

  while (++i < this->client_size_)
    if (this->client_array_[i] == nullptr) return i;

  return -1;
}
