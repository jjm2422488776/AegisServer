#!/usr/bin/env bash

HOST=${1:-127.0.0.1}
PORT=${2:-9222}
PATH_URL=${3:-/}
REQUESTS=${4:-1000}
CONCURRENCY=${5:-50}

echo "Running ab test..."
echo "Host: $HOST"
echo "Port: $PORT"
echo "Path: $PATH_URL"
echo "Requests: $REQUESTS"
echo "Concurrency: $CONCURRENCY"

ab -n "$REQUESTS" -c "$CONCURRENCY" "http://$HOST:$PORT$PATH_URL"