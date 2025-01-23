
#include "server/server.hpp"
#include "server/peer.hpp"
#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

void Server::ProcessConnections() {
  try {
    while (true) {
      boost::asio::ip::tcp::socket socket(this->io_context_);
      this->acceptor_.accept(socket);

      auto peer = std::make_shared<Peer>(std::move(socket));
      this->peers_.push_back(peer);
      peer->Start();
    }
  } catch (const std::exception &e) {
    std::cerr << "Error in ProcessConnections: " << e.what() << std::endl;
  }
}

void Server::WriteToPeers() {
  while (true) {
    for (auto &peer : this->peers_) {
      if (peer && !peer->IsDead()) {
        peer->SendQueue();
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void Server::Start(uint64_t port) {
  try {
    // Create and configure the acceptor
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
    this->acceptor_ =
        boost::asio::ip::tcp::acceptor(this->io_context_, endpoint);

    std::cout << "Server listening on port " << port << "..." << std::endl;

    this->data_writer_ = std::thread(&Server::WriteToPeers, this);
    this->process_connections_thread_ =
        std::thread(&Server::ProcessConnections, this);
  } catch (const std::exception &e) {
    std::cerr << "Error in Server::Start: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Server::Wait() {
  if (this->process_connections_thread_.joinable()) {
    this->process_connections_thread_.join();
  }
  if (this->data_writer_.joinable()) {
    this->data_writer_.join();
  }
}
