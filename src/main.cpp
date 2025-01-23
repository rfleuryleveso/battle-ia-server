
#include "executor/executor.hpp"
#include "server/server.hpp"

#include "visualizer/visualizer.hpp"
#include "visualizer/websocket-visualizer.hpp"
#include <spdlog/spdlog.h>

#ifdef ENABLE_RAYLIB
#include "visualizer/raylib-visualizer.hpp"
#endif

#include "world/world.hpp"
#include <boost/program_options.hpp>
#include <cstddef>
#include <iostream>
#include <memory>

int main(int argc, char *argv[]) {
  // Define the supported options
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()("help,h", "Show help message")(
      "port,p", boost::program_options::value<int>()->default_value(8080),
      "Set the server port")(
      "visualizer,v",
      boost::program_options::value<std::string>()->default_value("none"),
      "Set visualizer type (e.g., 'none', 'raylib', 'web')")(
      "ws-port", boost::program_options::value<int>()->default_value(9090),
      "Set the WebSocket visualizer port")(
      "generate-walls,g",
      boost::program_options::value<int>()->default_value(10),
      "Set the number of random walls to generate");
  boost::program_options::variables_map vm;

  try {
    // Parse command-line options
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    // Handle help option
    if (vm.count("help")) {
      std::cout << desc << "\n";
      return 0;
    }

    // Get options
    int port = vm["port"].as<int>();
    int ws_port = vm["ws-port"].as<int>();
    std::string visualizer_type = vm["visualizer"].as<std::string>();
    int num_walls = vm["generate-walls"].as<int>();

    // Validate port and num_walls
    if (port <= 0 || port > 65535) {
      std::cerr << "Invalid port number: " << port
                << ". Must be between 1 and 65535.\n";
      return 1;
    }
    if (num_walls < 0) {
      std::cerr << "Invalid number of walls: " << num_walls
                << ". Must be non-negative.\n";
      return 1;
    }

    // Initialize server, world, and visualizer
    auto server = std::make_shared<Server>();
    server->Start(port);

    while (true) {
      spdlog::info("Starting world");
      auto world = std::make_shared<World::World>();
      world->Start();

      world->GenerateRandomWalls(
          num_walls,
          1); // Use the number of walls from the argument
      world->GenWallBounds();

      // Select visualizer based on the --visualizer parameter
      std::shared_ptr<Visualizer> visualizer;
      if (visualizer_type == "none") {
        visualizer = std::make_shared<Visualizer>(world);
      }

#ifdef ENABLE_RAYLIB
      if (visualizer_type == "raylib") {
        visualizer = std::make_shared<RaylibVisualizer>(world);
      }
#endif

      else if (visualizer_type == "web") {
        visualizer = std::make_shared<WebSocketVisualizer>(world, ws_port);
      }

      else {
        std::cerr << "Unknown visualizer type: " << visualizer_type << "\n";
      }

      visualizer->Start();

      auto executor = std::make_shared<Executor>(server, world, visualizer);
      executor->Process();
      spdlog::info("Cleaning up world");
      world->Stop();
      spdlog::info("World killed");
    }
  } catch (const boost::program_options::error &ex) {
    std::cerr << "Error parsing options: " << ex.what() << "\n";
    return 1;
  }

  return 0;
}
