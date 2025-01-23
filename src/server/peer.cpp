
#include "server/peer.hpp"
#include "battle_c.pb.h"
#include <boost/asio.hpp>
#include <iostream>
#include <spdlog/spdlog.h>
#include <thread>
#include <vector>

using ClientServerMessage = battle_c::ClientServerMessage;

void Peer::Start() { this->handler_thread_ = std::thread(&Peer::Handle, this); }

void Peer::Handle() {
  try {
    spdlog::info("New client connected from {}:{}",
                 this->socket_.remote_endpoint().address().to_string(),
                 this->socket_.remote_endpoint().port());

    uint32_t message_size;

    while (!this->dead_) {

      // Read message size
      boost::asio::read(this->socket_, boost::asio::buffer(&message_size, 4));

      // Read message data
      std::vector<uint8_t> data(message_size);
      boost::asio::read(this->socket_,
                        boost::asio::buffer(data.data(), message_size));

      // Parse and handle message
      ClientServerMessage client_server_message;
      if (!client_server_message.ParseFromArray(data.data(), data.size())) {
        std::cerr << "Failed to parse message" << std::endl;
        continue;
      }

      if (client_server_message.has_client_init()) {
        this->SetPeerType(client_server_message.client_init().is_spectator()
                              ? PeerType::SPECTATOR
                              : PeerType::PAWN);
      } else {
        this->recvq.push(client_server_message);
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "Error in Handle: " << e.what() << std::endl;
    this->dead_ = true;
  }
}

void Peer::QueueMessage(ServerClientMessage message) {
  this->sendq.push(std::move(message));
}

void Peer::SendQueue() {
  try {
    while (!this->sendq.empty() && !this->dead_) {
      ServerClientMessage message = this->sendq.front();
      uint64_t size = message.ByteSizeLong();
      std::vector<uint8_t> buffer(size);

      message.SerializeToArray(buffer.data(), size);

      // Write message size
      uint32_t packet_size = buffer.size();
      boost::asio::write(this->socket_, boost::asio::buffer(&packet_size, 4));

      // Write message data
      boost::asio::write(this->socket_,
                         boost::asio::buffer(buffer.data(), size));

      this->sendq.pop();
    }
  } catch (const std::exception &e) {
    std::cerr << "Error in SendQueue: " << e.what() << std::endl;
    this->dead_ = true;
  }
}

std::optional<ClientServerMessage> Peer::PopMessage() {
  if (!this->recvq.empty()) {
    auto message = this->recvq.front();
    this->recvq.pop();
    return message;
  }
  return {};
}
