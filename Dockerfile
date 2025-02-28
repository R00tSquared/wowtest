FROM ubuntu:20.04

# Set timezone to prevent tzdata from prompting
ENV DEBIAN_FRONTEND=noninteractive
RUN ln -fs /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    echo "Etc/UTC" > /etc/timezone

# Install dependencies without prompts
RUN apt update && apt install -y \
    build-essential \
    gcc g++ \
    cmake \
    libssl-dev \
    libmariadb-dev \
    mariadb-client \
    git \
    curl \
    unzip \
    autoconf \
    libtool \
    make \
    wget \
    tzdata \
    software-properties-common \
    libtbb-dev \
    libace-dev \
    libreadline-dev && \
    apt install -y libmysqlclient-dev mysql-client-8.0 && \
    rm -rf /var/lib/apt/lists/*


# Manually install CMake 3.22+
RUN apt remove -y cmake && \
    wget -qO- "https://github.com/Kitware/CMake/releases/download/v3.27.6/cmake-3.27.6-linux-x86_64.tar.gz" | tar --strip-components=1 -xz -C /usr    

# Set working directory
WORKDIR /mwcore

# Copy source files from the local directory to the container
COPY . /mwcore

# Create a build directory
RUN mkdir build
WORKDIR /mwcore/build

# Ensure MySQL development files are available
RUN echo "Checking MySQL installation..." && mysql_config --libs

# Configure and build the server
RUN cmake .. -DCMAKE_INSTALL_PREFIX=/mwcore/bin -DACE_USE_EXTERNAL=1 && make -j$(nproc) && make install

# Expose necessary ports
EXPOSE 8085 3724

# Set entrypoint script
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
