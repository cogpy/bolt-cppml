#!/bin/bash
# Bolt.new (within bolt-cpp-ml) Development Environment Setup Script
# Usage: ./setup_bolt.sh [project_name]

set -e

PROJECT_NAME="${1:-bolt-custom}"
REPO_URL="https://github.com/stackblitz/bolt.new (within bolt-cpp-ml).git"

echo "🚀 Setting up Bolt.new (within bolt-cpp-ml) development environment..."
echo "   Project name: $PROJECT_NAME"

# Check prerequisites
echo "📋 Checking prerequisites..."

if ! command -v node &> /dev/null; then
    echo "❌ Node.js is not installed. Please install Node.js v20.15.1+"
    exit 1
fi

NODE_VERSION=$(node -v | cut -d'v' -f2 | cut -d'.' -f1)
if [ "$NODE_VERSION" -lt 20 ]; then
    echo "⚠️  Warning: Node.js version should be 20.15.1+. Current: $(node -v)"
fi

if ! command -v pnpm &> /dev/null; then
    echo "📦 Installing pnpm..."
    npm install -g pnpm@9.4.0
fi

# Clone repository
if [ -d "$PROJECT_NAME" ]; then
    echo "⚠️  Directory $PROJECT_NAME already exists. Skipping clone."
else
    echo "📥 Cloning Bolt.new (within bolt-cpp-ml) repository..."
    git clone "$REPO_URL" "$PROJECT_NAME"
fi

cd "$PROJECT_NAME"

# Install dependencies
echo "📦 Installing dependencies..."
pnpm install

# Create environment file if it doesn't exist
if [ ! -f ".env.local" ]; then
    echo "📝 Creating .env.local template..."
    cat > .env.local << 'EOF'
# Anthropic API Key (required)
ANTHROPIC_API_KEY=your_anthropic_api_key_here

# Optional: Debug logging
# VITE_LOG_LEVEL=debug
EOF
    echo "⚠️  Please edit .env.local and add your Anthropic API key"
else
    echo "✅ .env.local already exists"
fi

echo ""
echo "✅ Bolt.new (within bolt-cpp-ml) development environment is ready!"
echo ""
echo "Next steps:"
echo "  1. cd $PROJECT_NAME"
echo "  2. Edit .env.local with your Anthropic API key"
echo "  3. pnpm run dev"
echo ""
echo "Available commands:"
echo "  pnpm run dev     - Start development server"
echo "  pnpm run build   - Build for production"
echo "  pnpm run deploy  - Deploy to Cloudflare Pages"
echo "  pnpm test        - Run tests"
