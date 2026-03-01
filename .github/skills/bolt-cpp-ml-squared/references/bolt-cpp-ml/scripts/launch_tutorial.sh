#!/bin/bash
# bolt-cpp-ml: Launch the interactive TutorialKit tutorial
# This script starts the TutorialKit dev server for the bolt-cpp-ml tutorial.
# The tutorial is structured into 4 parts:
#   1. bolt-new: AI Web App Development
#   2. koboldcpp: Local LLM Inference
#   3. janext: Jan Extension Development
#   4. cpp-e2e-test-gen: C++ E2E Test Generation

set -e

TUTORIAL_DIR="$(dirname "$0")/../templates/bolt-cpp-ml-tutorial"

if [ ! -d "$TUTORIAL_DIR" ]; then
  echo "Error: Tutorial directory not found at $TUTORIAL_DIR"
  exit 1
fi

cd "$TUTORIAL_DIR"

# Install dependencies if needed
if [ ! -d "node_modules" ]; then
  echo "Installing tutorial dependencies..."
  pnpm install
fi

echo "Starting bolt-cpp-ml interactive tutorial at http://localhost:4321"
pnpm start
