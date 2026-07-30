FROM golang:1.23-alpine AS builder
WORKDIR /app
COPY go.mod go.sum ./
RUN go mod download
COPY . .
RUN CGO_ENABLED=0 GOOS=linux go build -ldflags="-s -w" -o hub .

FROM alpine:3.20
RUN addgroup -S hub && adduser -S -G hub -u 10001 hub && \
    mkdir -p /data && chown hub:hub /data
COPY --from=builder /app/hub /usr/local/bin/hub
USER hub
VOLUME ["/data"]
EXPOSE 8080
ENV DB_PATH=/data/hub.db
HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
    CMD wget -qO- http://localhost:8080/health || exit 1
ENTRYPOINT ["hub"]
