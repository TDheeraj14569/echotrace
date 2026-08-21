# Stage 1: Build C++ engine
FROM gcc:14 AS engine-builder
WORKDIR /build
COPY include/ include/
COPY src/ src/
COPY Makefile .
RUN make cli

# Stage 2: Build frontend
FROM node:22-alpine AS frontend-builder
WORKDIR /app
COPY web/frontend/package*.json ./
RUN npm ci
COPY web/frontend/ .
RUN npm run build

# Stage 3: Production image
FROM node:22-alpine
WORKDIR /app

# Copy engine binary
COPY --from=engine-builder /build/build/bin/echotrace /usr/local/bin/echotrace
RUN chmod +x /usr/local/bin/echotrace

# Copy backend
COPY web/backend/package*.json ./
RUN npm ci --omit=dev
COPY web/backend/src/ src/

# Copy frontend build
COPY --from=frontend-builder /app/dist/ ../frontend/dist/

# Create directories
RUN mkdir -p uploads data

ENV NODE_ENV=production
ENV PORT=3001
EXPOSE 3001

CMD ["node", "src/server.js"]
