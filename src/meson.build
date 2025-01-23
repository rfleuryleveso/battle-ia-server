gen = generator(protoc, \
  output    : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
  arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--cpp_out=@BUILD_DIR@', '--experimental_allow_proto3_optional', '@INPUT@'])

generated = gen.process('battle_c.proto')

src_files = ['main.cpp', 'server/server.cpp', 'server/peer.cpp', 'executor/executor.cpp', 'world/world_object.cpp', 'world/boost.cpp', 'world/world.cpp', 'world/pawn.cpp', 'visualizer/websocket-visualizer.cpp', generated]

if raylib_dep.found()
  src_files = src_files + ['visualizer/raylib-visualizer.cpp']
endif


# Conditional dependencies
optional_dependencies = [boost_dep, protobuf_dep, spdlog_dep, json_dep]
if raylib_dep.found()
  optional_dependencies += raylib_dep
endif

# Add a preprocessor flag for Raylib support if found
cpp_args = []
if raylib_dep.found()
  cpp_args += ['-DENABLE_RAYLIB']
endif

# Define the executable
executable(
    'server',
    src_files,
    dependencies: optional_dependencies,
    include_directories: [inc_dir],
    cpp_args: cpp_args
)
