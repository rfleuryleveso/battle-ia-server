
#include "visualizer/websocket-visualizer.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace json = nlohmann;

WebSocketVisualizer::WebSocketVisualizer(std::shared_ptr<World::World> world,
                                         int port)
    : Visualizer(world), port_(port), running_(false) {}

WebSocketVisualizer::~WebSocketVisualizer() {
  running_ = false;
  if (server_thread_.joinable()) {
    server_thread_.join();
  }
}

void WebSocketVisualizer::Start() {
  if (running_) {
    std::cerr << "WebSocket Visualizer is already running!" << std::endl;
    return;
  }

  running_ = true;
  server_thread_ = std::thread(&WebSocketVisualizer::RunServer, this);
}

void WebSocketVisualizer::DrawShoot(World::Vector3 start, World::Vector3 end) {
  // Create JSON message for the shoot
  json::json message = {
      {"type", "shoot"},
      {"start",
       {{"x", start.GetX()}, {"y", start.GetY()}, {"z", start.GetZ()}}},
      {"end", {{"x", end.GetX()}, {"y", end.GetY()}, {"z", end.GetZ()}}}};
  BroadcastMessage(message.dump());
}

void WebSocketVisualizer::RunServer() {
  try {
    boost::asio::io_context ioc;
    tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), port_));

    spdlog::info("WebSocket server started on port {}", port_);

    // Launch a separate thread for broadcasting world state
    std::thread broadcaster([this]() {
      while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        json::json world_state = {{"type", "world"}};

        for (const auto &obj : world_->GetWorldObjects()) {
          if (obj->IsDestroyed()) {
            continue;
          }
          json::json object_data = {{"position",
                                     {{"x", obj->GetPosition().GetX()},
                                      {"y", obj->GetPosition().GetY()}}},
                                    {"radius", obj->GetRadius()},
                                    {"type", obj->GetWorldObjectType()}};

          world_state["objects"].push_back(object_data);
        }

        spdlog::debug("Broadcasting world state: {}", world_state.dump());
        BroadcastMessage(world_state.dump());
      }
    });

    // Main server loop: accept and handle connections
    while (running_) {
      tcp::socket socket(ioc);
      acceptor.accept(socket);

      auto ws =
          std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));
      ws->accept();

      {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connections_.insert(ws);
        spdlog::info("New client connected. Active connections: {}",
                     connections_.size());
      }

      // Launch a thread for this WebSocket connection
      std::thread([this, ws]() {
        try {
          while (running_) {
            boost::beast::flat_buffer buffer;
            ws->read(buffer); // Read messages (not used here, but placeholder)
          }
        } catch (const boost::system::system_error &e) {
          spdlog::warn("Client disconnected: {}", e.what());
          std::lock_guard<std::mutex> lock(connections_mutex_);
          connections_.erase(ws);
          spdlog::info("Client removed. Active connections: {}",
                       connections_.size());
        }
      }).detach();
    }

    broadcaster.join(); // Wait for the broadcaster to finish
  } catch (const std::exception &e) {
    spdlog::error("WebSocket server error: {}", e.what());
  }
}

void WebSocketVisualizer::BroadcastMessage(const std::string &message) {
  std::lock_guard<std::mutex> lock(connections_mutex_);
  for (auto &conn : connections_) {
    try {
      conn->write(boost::asio::buffer(message));
    } catch (const std::exception &e) {
      std::cerr << "Failed to send message: " << e.what() << std::endl;
    }
  }
}
