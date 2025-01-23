
FROM teeks99/clang-ubuntu:19

RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


RUN apt-get update && apt-get install -y \
    ninja-build \
    pkg-config \
    libboost-all-dev \
    protobuf-compiler \
    libprotobuf-dev \
    wget \
    unzip \
    meson \
    pkg-config && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Install system dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-setuptools \
    python3-wheel \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /app 

COPY . .

RUN meson setup builddir/

RUN meson compile -C builddir/

# Specify the command to run the application
CMD ["/app/builddir/src/server", "--visualizer", "web", "--ws-port", "8081", "--port", "8080"]

EXPOSE 8080
EXPOSE 8081
