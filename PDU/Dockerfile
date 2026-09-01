# Usar imagen base ligera
FROM debian:bookworm-slim

# Evitar prompts interactivos
ENV DEBIAN_FRONTEND=noninteractive

# Instalar herramientas: incluimos build-essential para que CLion lo detecte
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    make \
    ninja-build \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    git \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

CMD ["/bin/bash"]