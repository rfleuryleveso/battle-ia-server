#include "visualizer/raylib-visualizer.hpp"
#include "raylib.h"
#include "world/pawn.hpp"
#include <memory>

void RaylibVisualizer::Process() {
  const int screenWidth = 800;
  const int screenHeight = 800;

  // Initialize Raylib
  InitWindow(screenWidth, screenHeight, "World Visualizer");
  SetTargetFPS(60);

  while (running_ && !WindowShouldClose()) {
    // Start drawing
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawText("World Visualizer", 10, 10, 20, DARKGRAY);

    // Draw world objects
    DrawWorldObjects();

    // Draw and persist shoots
    {
      std::lock_guard<std::mutex> lock(shoot_mutex_);
      RemoveExpiredShoots(); // Remove old shoots

      for (const auto &timed_shoot : shoots_) {
        const auto &shoot = timed_shoot.first;
        const auto &start = shoot.first;
        const auto &end = shoot.second;

        // Convert World::Vector3 to Raylib 2D representation
        DrawLine(static_cast<int>(start.GetX()), static_cast<int>(start.GetY()),
                 static_cast<int>(end.GetX()), static_cast<int>(end.GetY()),
                 RED);
      }
    }

    EndDrawing();
  }

  CloseWindow(); // Close the Raylib window
}

void RaylibVisualizer::DrawWorldObjects() {
  const double sizeX = world_->GetSizeX();
  const double sizeY = world_->GetSizeY();

  for (const auto &obj : world_->GetWorldObjects()) {
    World::Vector3 position = obj->GetPosition();
    double radius = obj->GetRadius();

    // Normalize position to screen coordinates
    int x = static_cast<int>((position.GetX() / sizeX) * screen_width_);
    int y = static_cast<int>((position.GetY() / sizeY) * screen_height_);
    int r = static_cast<int>((radius / sizeX) * screen_width_);

    // Determine color based on object type
    Color color;
    switch (obj->GetWorldObjectType()) {
    case World::WorldObjectType::PAWN:
      color = RED;
      break;
    case World::WorldObjectType::WALL:
      color = GRAY;
      break;
    case World::WorldObjectType::BOOST:
      color = GREEN;
      break;
    default:
      color = BLUE;
      break;
    }

    DrawCircle(x, y, r, color);

    // Draw health for PAWN objects
    if (obj->GetWorldObjectType() == World::WorldObjectType::PAWN) {
      std::shared_ptr<World::WorldObject> world_object = obj;

      // Now apply dynamic_pointer_cast
      std::shared_ptr<World::Pawn> pawn =
          std::dynamic_pointer_cast<World::Pawn>(world_object);
      if (pawn) {
        char healthText[10];
        snprintf(healthText, sizeof(healthText), "%d", pawn->GetHealth());
        DrawText(healthText, x - 10, y - 10, 10, BLACK);
      }
    } else if (obj->GetWorldObjectType() == World::WorldObjectType::WALL) {
      // Draw wall as a gray rectangle
      double radius =
          obj->GetRadius(); // Assuming radius is the wall's half-width
      int rectWidth = static_cast<int>((radius * 2 / sizeX) * screen_width_);
      int rectHeight = static_cast<int>((radius * 2 / sizeY) * screen_height_);

      DrawRectangle(x - rectWidth / 2, y - rectHeight / 2, rectWidth,
                    rectHeight, GRAY);
    } else if (obj->GetWorldObjectType() == World::WorldObjectType::BOOST) {
      // Draw boost as a green circle
      double radius = obj->GetRadius();
      int r = static_cast<int>((radius / sizeX) * screen_width_);

      DrawCircle(x, y, r, GREEN);
    }
  }
}

void RaylibVisualizer::Start() {
  if (running_) {
    std::cerr << "Visualizer is already running!" << std::endl;
    return;
  }

  running_ = true;
  visualizer_thread_ = std::thread(&RaylibVisualizer::Process, this);
}

void RaylibVisualizer::RemoveExpiredShoots() {
  // Define how long shoots persist
  const auto duration = std::chrono::seconds(2);
  const auto now = std::chrono::steady_clock::now();

  // Remove shoots that have persisted longer than the defined duration
  shoots_.erase(std::remove_if(shoots_.begin(), shoots_.end(),
                               [&](const TimedShoot &timed_shoot) {
                                 return now - timed_shoot.second > duration;
                               }),
                shoots_.end());
}

void RaylibVisualizer::DrawShoot(World::Vector3 start, World::Vector3 end) {
  const double sizeX = world_->GetSizeX();
  const double sizeY = world_->GetSizeY();
  // Normalize the shoot positions to screen coordinates
  World::Vector3 normalized_start((start.GetX() / sizeX) * screen_width_,
                                  (start.GetY() / sizeY) * screen_height_,
                                  start.GetZ());
  World::Vector3 normalized_end((end.GetX() / sizeX) * screen_width_,
                                (end.GetY() / sizeY) * screen_height_,
                                end.GetZ());

  // Store the normalized shoot positions with a timestamp
  std::lock_guard<std::mutex> lock(shoot_mutex_);
  shoots_.emplace_back(std::make_pair(normalized_start, normalized_end),
                       std::chrono::steady_clock::now());
}
