
#ifndef WEBSOCKET_VISUALIZER_HPP
#define WEBSOCKET_VISUALIZER_HPP

#include "visualizer.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <mutex>
#include <set>
#include <thread>

class WebSocketVisualizer : public Visualizer {
public:
  WebSocketVisualizer(std::shared_ptr<World::World> world, int port);
  ~WebSocketVisualizer() override;

  void Start() override;
  void DrawShoot(World::Vector3 start, World::Vector3 end) override;

private:
  void RunServer();

  int port_;
  std::thread server_thread_;
  std::atomic<bool> running_;
  std::mutex connections_mutex_;
  std::set<std::shared_ptr<
      boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>>
      connections_;

  void BroadcastMessage(const std::string &message);
};

#endif // WEBSOCKET_VISUALIZER_HPP
