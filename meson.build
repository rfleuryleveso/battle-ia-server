project('battle_c_server', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++17'])

inc_dir = include_directories('include')

spdlog_dep = dependency('spdlog', required: true)

json_dep = dependency('nlohmann_json', required : true) 

boost_dep = dependency('boost', modules : ['program_options'], required : true)

# Define protobuf compiler and libraries
protobuf_dep = dependency('protobuf')

protoc = find_program('protoc', required : false)

raylib_dep = dependency('raylib', required : false)

if not protoc.found() or not protobuf_dep.found()
  error('MESON_SKIP_TEST: protoc tool and/or protobuf pkg-config dependency not found')
endif

subdir('src')
