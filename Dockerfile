

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libpq-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY frontend/Backend/ ./ 

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --target FaarisAPI -j2

ENV PORT=8080

EXPOSE 8080

CMD ["./build/FaarisAPI"]