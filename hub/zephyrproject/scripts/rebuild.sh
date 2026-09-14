#!/bin/bash

APP_NAME="app"
BOARD="pic32cx_sg41_cult"

echo "♻️  Rebuilding $APP_NAME for $BOARD (Pristine)..."

west build -b $BOARD $APP_NAME --pristine

if [ $? -eq 0 ]; then
    echo "✅ Build Successful!"
else
    echo "❌ Build Failed"
    exit 1
fi