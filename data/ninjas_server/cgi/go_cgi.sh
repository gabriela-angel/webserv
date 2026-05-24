#!/bin/sh

GOCACHE_DIR="${GOCACHE:-/tmp/webserv-go-cache}"
HOME_DIR="${HOME:-/tmp}"

mkdir -p "$GOCACHE_DIR" || exit 1

export GOCACHE="$GOCACHE_DIR"
export HOME="$HOME_DIR"

exec /usr/bin/go run "$1"