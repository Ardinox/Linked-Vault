# --- STAGE 1: Builder (Compiles the code) ---
# We use 'gcc:bookworm' to match the specific Linux version of Stage 2.
# This fixes the "GLIBC_2.38 not found" error you saw earlier.
FROM gcc:bookworm AS builder

WORKDIR /app

# Copy source code
COPY . .

# COMPILE COMMAND (Linux Version):
# 1. src/*.c vendor/*.c : Compiles all your C files.
# 2. -Iinclude -Ivendor : Look for headers in these folders (Matches your Makefile).
# 3. -Dfseeko=fseek     : Fixes large file seeking (Matches your Makefile).
# 4. -pthread           : Enables threading (Replaces Windows -lpthread).
# 5. -static-libgcc     : Bundles libraries so it runs safely on the Slim image.
# NOTE: We intentionally DO NOT link -lws2_32 because that is Windows-only.
RUN gcc -o server src/*.c vendor/*.c -Iinclude -Ivendor -Dfseeko=fseek -pthread -static-libgcc

# --- STAGE 2: Runner (Runs the code) ---
# We use 'debian:bookworm-slim' to keep the image small (50MB vs 1GB).
FROM debian:bookworm-slim

WORKDIR /app

# Copy only the compiled 'server' file from Stage 1
COPY --from=builder /app/server .

# Create the bin directory for your data
RUN mkdir -p /app/bin

# Define the volume so data is saved to your host machine
VOLUME ["/app/bin"]

# Expose the port
EXPOSE 8000

# Start the server
CMD ["./server"]