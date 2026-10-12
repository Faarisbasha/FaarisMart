
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install C++ build tools and PostgreSQL development libraries
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libpq-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy backend source files, including api.cpp and httplib.h
COPY frontend/Backend/ ./

# Configure and build the API server
RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --target FaarisAPI -j2

# Render provides the PORT environment variable
ENV PORT=8080

EXPOSE 8080

# Start the API server
CMD ["./build/FaarisAPI"]