FROM emscripten/emsdk:latest

WORKDIR /app

RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    git \
    && rm -rf /var/lib/apt/lists/*

COPY . /app

RUN mkdir -p build_wasm

# Build WASM module
RUN cd build_wasm && \
    cmake .. && \
    make

EXPOSE 8080

# Install a simple web server
RUN npm install -g http-server

# Set up entrypoint script
RUN echo '#!/bin/bash\n\
cd /app/web\n\
http-server -p 8080' > /entrypoint.sh && \
chmod +x /entrypoint.sh

CMD ["/entrypoint.sh"]
