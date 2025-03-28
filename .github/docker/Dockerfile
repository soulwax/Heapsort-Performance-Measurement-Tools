FROM debian:bullseye-slim as builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    make \
    gcc \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /build

# Copy source code
COPY . .

# Install Python dependencies
RUN pip3 install -r requirements.txt

# Build the project with optimizations
RUN sed -i 's/-g -Wall -O2/-Wall -O3 -flto -fomit-frame-pointer/g' Makefile && \
    make clean && \
    make

# Create a smaller runtime image
FROM debian:bullseye-slim

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Copy only the necessary files from the builder
WORKDIR /app
COPY --from=builder /build/bin /app/bin
COPY --from=builder /build/requirements.txt /app/
COPY --from=builder /build/visualize_benchmark.py /app/
COPY --from=builder /build/README.md /app/

# Install Python dependencies for visualization
RUN pip3 install -r requirements.txt

# Create necessary directories
RUN mkdir -p /app/input /app/output /app/benchmark_results /app/benchmark_plots

# Set the path
ENV PATH="/app/bin:${PATH}"

# Set entry point
ENTRYPOINT ["/bin/bash", "-c"]
CMD ["echo 'Heapsort container is ready. Use one of the following commands:'; echo '- heapsort [numbers]'; echo '- genrand_f [options]'; echo '- benchmark [options]'"]
