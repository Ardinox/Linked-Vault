# Use Node.js Alpine (Lightweight)
FROM node:alpine

# Set working directory
WORKDIR /app

# Copy package.json first (optimizes build speed)
COPY package.json .

# Install dependencies
RUN npm install

# Copy source code
COPY . .

# Expose Port 4000
EXPOSE 4000

# Start the server
CMD ["node", "server.js"]