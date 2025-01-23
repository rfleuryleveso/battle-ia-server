
#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include "server/peer.hpp"
#include <boost/asio.hpp>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

class Server : public std::enable_shared_from_this<Server> {
private:
  void ProcessConnections();
  void WriteToPeers();

  std::vector<std::shared_ptr<Peer>> peers_;

  boost::asio::io_context io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;

  std::thread process_connections_thread_;
  std::thread data_writer_;

public:
  /**
   * Constructor
   */
  Server() : acceptor_(io_context_) {}

  /**
   * Starts the server in a thread
   */
  void Start(uint64_t port);

  ~Server() {}

  /**
   * Waits for the server threads to complete
   */
  void Wait();

  /**
   * Retrieves the list of connected peers
   */
  std::vector<std::shared_ptr<Peer>> &GetPeers() { return peers_; }
};

#endif // SERVER_SERVER_HPP
