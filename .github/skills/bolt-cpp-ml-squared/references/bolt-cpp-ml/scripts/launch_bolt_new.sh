#!/bin/bash

# This script launches the bolt-new tutorial.

# The tutorial is located in the templates directory of the bolt-cpp-ml skill.
TUTORIAL_DIR="/home/ubuntu/skills/bolt-cpp-ml/templates/bolt-cpp-ml-tutorial"

# Start the tutorial using the tutorialkit CLI.
cd "$TUTORIAL_DIR"
pnpm start
