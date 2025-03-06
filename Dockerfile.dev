FROM emscripten/emsdk:latest

# Install basic tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    unzip \
    pkg-config \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Install Google Test for native tests
RUN git clone https://github.com/google/googletest.git /googletest \
    && cd /googletest \
    && mkdir build \
    && cd build \
    && cmake .. -DBUILD_GMOCK=OFF -DINSTALL_GTEST=ON \
    && make -j$(nproc) \
    && make install \
    && ldconfig \
    && cd / \
    && rm -rf /googletest

# Set the working directory
WORKDIR /app

# Create a script to build for WebAssembly
RUN echo '#!/bin/bash\n\
mkdir -p wasm_build\n\
cd wasm_build\n\
emcmake cmake .. \
-DCMAKE_BUILD_TYPE=Release \
&& make -j$(nproc) VERBOSE=1\n\
' > /usr/local/bin/build_wasm.sh \
&& chmod +x /usr/local/bin/build_wasm.sh

# Create a script to build native and run tests
RUN echo '#!/bin/bash\n\
mkdir -p native_build\n\
cd native_build\n\
cmake .. \
-DCMAKE_BUILD_TYPE=Debug \
&& make -j$(nproc) \
&& ctest --output-on-failure\n\
' > /usr/local/bin/build_and_test.sh \
&& chmod +x /usr/local/bin/build_and_test.sh

# Create a script to build both WebAssembly and native
RUN echo '#!/bin/bash\n\
build_wasm.sh && build_and_test.sh\n\
' > /usr/local/bin/build_all.sh \
&& chmod +x /usr/local/bin/build_all.sh

# Set the entrypoint to allow overriding the command
ENTRYPOINT ["/bin/bash", "-c"]
CMD ["build_all.sh"]
