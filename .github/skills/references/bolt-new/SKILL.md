---
name: bolt-cpp-ml-bolt-new
description: Part of the bolt-cpp-ml skill: Build AI-powered web development agents using the Bolt.new (within bolt-cpp-ml) open-source codebase.
---

# Bolt.new (within bolt-cpp-ml) Development

Build AI-powered full-stack web development agents using StackBlitz's Bolt.new (within bolt-cpp-ml) open-source codebase. Bolt combines WebContainers (browser-based Node.js runtime) with LLM integration to create, edit, and deploy applications entirely in the browser.

## Quick Start

### Prerequisites

- Node.js v20.15.1+
- pnpm v9.4.0+
- Anthropic API key (for Claude Sonnet 3.5)

### Setup

```bash
# Clone the repository
git clone https://github.com/stackblitz/bolt.new (within bolt-cpp-ml).git
cd bolt.new (within bolt-cpp-ml)

# Install dependencies
pnpm install

# Create environment file
echo "ANTHROPIC_API_KEY=your_key_here" > .env.local

# Start development server
pnpm run dev
```

### Available Scripts

| Command | Purpose |
|---------|---------|
| `pnpm run dev` | Start development server |
| `pnpm run build` | Build for production |
| `pnpm run start` | Run built app with Wrangler |
| `pnpm run deploy` | Deploy to Cloudflare Pages |
| `pnpm test` | Run test suite |

## Architecture Overview

### Core Components

1. **WebContainer API** (`app/lib/webcontainer/`) - Browser-based Node.js runtime
2. **LLM Integration** (`app/lib/.server/llm/`) - AI model configuration and prompts
3. **Action Runner** (`app/lib/runtime/`) - Executes shell commands and file operations
4. **Chat Interface** (`app/components/chat/`) - User interaction and message streaming
5. **Workbench** (`app/components/workbench/`) - Code editor, file tree, terminal, preview

### Key Files

| File | Purpose |
|------|---------|
| `app/lib/.server/llm/prompts.ts` | System prompt and artifact format |
| `app/lib/.server/llm/model.ts` | LLM model configuration |
| `app/lib/.server/llm/stream-text.ts` | Streaming text generation |
| `app/lib/runtime/action-runner.ts` | Execute `<boltAction>` commands |
| `app/lib/runtime/message-parser.ts` | Parse AI responses for artifacts |
| `wrangler.toml` | Cloudflare deployment config |

## Customizing the System Prompt

The system prompt in `app/lib/.server/llm/prompts.ts` defines Bolt's behavior. Key sections:

### System Constraints

Defines WebContainer limitations:
- No native binaries (C/C++ compilation unavailable)
- Python limited to standard library (no pip)
- Git unavailable
- Prefer Vite for web servers

### Artifact Format

Bolt uses `<boltArtifact>` and `<boltAction>` tags for structured output:

```xml
<boltArtifact id="project-id" title="Project Title">
  <boltAction type="file" filePath="package.json">
    { "name": "project", ... }
  </boltAction>
  <boltAction type="shell">
    npm install
  </boltAction>
</boltArtifact>
```

**Action Types:**
- `file` - Create/update files (requires `filePath` attribute)
- `shell` - Execute shell commands

### Modifying the Prompt

To customize behavior, edit `getSystemPrompt()` in `prompts.ts`:

```typescript
export const getSystemPrompt = (cwd: string = WORK_DIR) => `
You are Bolt, an expert AI assistant...

<custom_section>
  Add your custom instructions here.
</custom_section>

// ... rest of prompt
`;
```

## Adding New LLM Providers

### Step 1: Install Provider SDK

```bash
pnpm add @ai-sdk/openai  # or other provider
```

### Step 2: Update Model Configuration

Edit `app/lib/.server/llm/model.ts`:

```typescript
import { createOpenAI } from '@ai-sdk/openai';

export function getModel(env: Env) {
  const openai = createOpenAI({
    apiKey: env.OPENAI_API_KEY,
  });
  return openai('gpt-4-turbo');
}
```

### Step 3: Add Environment Variable

Update `.env.local`:

```
OPENAI_API_KEY=your_key_here
```

### Step 4: Update Type Definitions

Add to `worker-configuration.d.ts`:

```typescript
interface Env {
  OPENAI_API_KEY: string;
}
```

## WebContainer Constraints

When developing for Bolt, respect WebContainer limitations:

### Supported

- JavaScript/TypeScript execution
- npm packages (pure JS only)
- Web servers (Vite, Express, etc.)
- WebAssembly
- Python standard library

### Not Supported

- Native binaries
- C/C++ compilation
- pip packages
- Git operations
- System-level operations

### Database Recommendations

Use browser-compatible databases:
- libsql / sql.js
- IndexedDB
- PouchDB

Avoid databases requiring native binaries (PostgreSQL, MySQL clients).

## Deployment to Cloudflare

### Prerequisites

1. Cloudflare account
2. Wrangler CLI configured (`npx wrangler login`)

### Deploy

```bash
pnpm run deploy
```

### Configuration

Edit `wrangler.toml` for custom settings:

```toml
name = "my-bolt-instance"
compatibility_flags = ["nodejs_compat"]
compatibility_date = "2024-07-01"
pages_build_output_dir = "./build/client"
```

### Environment Variables

Set secrets in Cloudflare dashboard or via CLI:

```bash
npx wrangler secret put ANTHROPIC_API_KEY
```

## Extending Functionality

### Adding New Action Types

1. Define type in `app/types/actions.ts`
2. Add handler in `app/lib/runtime/action-runner.ts`
3. Update message parser in `app/lib/runtime/message-parser.ts`
4. Document in system prompt

### Adding UI Components

Components use:
- React with Remix
- UnoCSS for styling
- Nanostores for state management
- CodeMirror for code editing

### State Management

Key stores in `app/lib/stores/`:
- `chat.ts` - Chat messages and state
- `files.ts` - Virtual filesystem
- `workbench.ts` - Editor and preview state
- `terminal.ts` - Terminal output

## Troubleshooting

### Common Issues

**"Cannot find module" errors:**
Ensure dependencies are installed with `pnpm install`.

**WebContainer boot failures:**
Check browser compatibility (Chrome/Edge recommended).

**API key errors:**
Verify `.env.local` exists and contains valid `ANTHROPIC_API_KEY`.

**Build failures:**
Run `pnpm run typecheck` to identify TypeScript errors.

### Debug Mode

Enable verbose logging:

```
VITE_LOG_LEVEL=debug
```
