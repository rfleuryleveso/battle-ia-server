
#ifndef SERVER_PEER_HPP
#define SERVER_PEER_HPP

#include "battle_c.pb.h"
#include <boost/asio.hpp>
#include <cstdint>
#include <optional>
#include <queue>
#include <thread>

enum class PeerType { UNDEFINED, PAWN, SPECTATOR };
using namespace battle_c;

class Peer {
private:
  boost::asio::ip::tcp::socket socket_;
  std::thread handler_thread_;
  PeerType peer_type_ = PeerType::PAWN;

  void Handle();

  std::queue<ClientServerMessage> recvq = {};
  std::queue<ServerClientMessage> sendq = {};

  bool dead_ = false;

public:
  // Constructor accepting an io_context (existing behavior)
  Peer(boost::asio::io_context &io_context) : socket_(io_context) {}

  // New constructor accepting a socket
  Peer(boost::asio::ip::tcp::socket &&socket) : socket_(std::move(socket)) {}

  /**
   * Start the peer's thread
   */
  void Start();

  /**
   * Queue a message to send
   */
  void QueueMessage(ServerClientMessage message);

  /**
   * Pop a received message
   */
  std::optional<ClientServerMessage> PopMessage();

  /**
   * Send messages from the queue
   */
  void SendQueue();

  /**
   * Get and set the peer type
   */
  PeerType GetPeerType() const { return peer_type_; }
  void SetPeerType(PeerType peer_type) { peer_type_ = peer_type; }

  /**
   * Get the Boost.Asio socket
   */
  boost::asio::ip::tcp::socket &GetSocket() { return socket_; }

  /**
   * Check if the peer is dead
   */
  bool IsDead() const { return dead_; }
  void SetIsDead(bool is_dead) { dead_ = is_dead; };
};

#endif // SERVER_PEER_HPP
