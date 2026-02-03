# Use Nginx Alpine (Lightweight Web Server)
FROM nginx:alpine

# Set working directory to Nginx's default public folder
WORKDIR /usr/share/nginx/html

# 1. Clean existing default files
RUN rm -rf ./*

# 2. Copy frontend files
# This copies css/, js/, views/, index.html, etc. exactly as they are structure-wise
COPY . .

# 3. Overwrite the default Nginx config with custom proxy config
COPY nginx.conf /etc/nginx/conf.d/default.conf

# Expose port 80 (Internal Container Port)
EXPOSE 80

# Start Nginx
CMD ["nginx", "-g", "daemon off;"]