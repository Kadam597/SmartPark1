# ============================================================
# SmartPark - Production Multi-Stage Dockerfile
# Compiles C++ backend & bundles the web UI for Cloud Hosting
# ============================================================

# Stage 1: Build the C++ server binary
FROM debian:bookworm-slim AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ \
    make \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

# Compile for Linux with C++17 and pthread
RUN g++ -std=c++17 -O2 -Wall -Wextra -I. \
    server.cpp vehicle.cpp vehicle_types.cpp parking_slot.cpp parking_lot.cpp \
    -o SmartParkServer -lpthread

# Stage 2: Minimal production runtime
FROM debian:bookworm-slim

WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy compiled binary and frontend web assets
COPY --from=builder /app/SmartParkServer /app/SmartParkServer
COPY --from=builder /app/web /app/web

# Cloud platforms automatically provide $PORT
ENV HOST=0.0.0.0
ENV PORT=8080

EXPOSE 8080

CMD ["/app/SmartParkServer"]
