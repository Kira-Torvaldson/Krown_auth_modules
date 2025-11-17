# Dockerfile multi-stage pour krown_auth
# Supporte Debian, Ubuntu et Arch Linux

# ============================================
# Stage 1: Builder pour Debian
# ============================================
FROM debian:bookworm-slim AS debian-builder

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    make \
    openssh-client \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copier les fichiers sources avec la nouvelle structure
COPY include/ ./include/
COPY src/ ./src/
COPY Makefile ./

RUN make clean && make

# ============================================
# Stage 2: Builder pour Ubuntu
# ============================================
FROM ubuntu:22.04 AS ubuntu-builder

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    make \
    openssh-client \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copier les fichiers sources avec la nouvelle structure
COPY include/ ./include/
COPY src/ ./src/
COPY Makefile ./

RUN make clean && make

# ============================================
# Stage 3: Builder pour Arch Linux
# ============================================
FROM archlinux:latest AS arch-builder

RUN pacman -Syu --noconfirm \
    base-devel \
    gcc \
    make \
    openssh \
    && rm -rf /var/cache/pacman/pkg/*

WORKDIR /app

# Copier les fichiers sources avec la nouvelle structure
COPY include/ ./include/
COPY src/ ./src/
COPY Makefile ./

RUN make clean && make

# ============================================
# Stage 4: Image runtime Debian
# ============================================
FROM debian:bookworm-slim AS debian-runtime

RUN apt-get update && apt-get install -y \
    openssh-client \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=debian-builder /app/build/krown_auth .

CMD ["./krown_auth"]

# ============================================
# Stage 5: Image runtime Ubuntu
# ============================================
FROM ubuntu:22.04 AS ubuntu-runtime

RUN apt-get update && apt-get install -y \
    openssh-client \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=ubuntu-builder /app/build/krown_auth .

CMD ["./krown_auth"]

# ============================================
# Stage 6: Image runtime Arch Linux
# ============================================
FROM archlinux:latest AS arch-runtime

RUN pacman -Syu --noconfirm \
    openssh \
    && rm -rf /var/cache/pacman/pkg/*

WORKDIR /app

COPY --from=arch-builder /app/build/krown_auth .

CMD ["./krown_auth"]
