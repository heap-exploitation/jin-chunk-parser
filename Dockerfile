FROM ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Update and install required dependencies
RUN apt update && apt install -y \
    wget curl build-essential flex bison \
    libssl-dev libelf-dev bc kmod \
    gdb gcc make vim && \
    apt clean && rm -rf /var/lib/apt/lists/*

# Download and extract Linux 6.8.0
WORKDIR /usr/src
RUN wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.8.tar.xz && \
    tar -xf linux-6.8.tar.xz && \
    rm linux-6.8.tar.xz

# Build and install the kernel
WORKDIR /usr/src/linux-6.8
RUN make defconfig && \
    make -j$(nproc) && \
    make modules_install && \
    make install

# Set the new kernel version (optional, depends on container capabilities)
CMD ["/bin/bash"]
