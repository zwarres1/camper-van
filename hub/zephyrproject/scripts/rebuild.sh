#!/bin/bash
# usage: ./rebuild.sh [app_name]

APP_NAME="app" # Default to 'my-app' if no arg provided
BOARD="pic32cx_sg41_cult"

echo "♻️  Rebuilding $APP_NAME for $BOARD (Pristine)..."

west build -b $BOARD $APP_NAME --pristine

if [ $? -eq 0 ]; then
    echo "✅ Build Successful!"
else
    echo "❌ Build Failed"
    exit 1
fi