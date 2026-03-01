# Bolt.new (within bolt-cpp-ml) System Prompt Reference

This document contains detailed information about the Bolt.new (within bolt-cpp-ml) system prompt structure for advanced customization.

## Full Prompt Structure

The system prompt consists of these XML sections:

### `<system_constraints>`

Defines the WebContainer environment limitations:

```
- WebContainer is an in-browser Node.js runtime
- Cannot run native binaries
- Python limited to standard library (no pip)
- No C/C++ compiler (g++)
- Git is NOT available
- Prefer Node.js scripts over shell scripts
- Use npm packages that don't rely on native binaries
- Available shell commands: cat, chmod, cp, echo, hostname, kill, ln, ls, mkdir, mv, ps, pwd, rm, rmdir, xxd, alias, cd, clear, curl, env, false, getconf, head, sort, tail, touch, true, uptime, which, code, jq, loadenv, node, python3, wasm, xdg-open, command, exit, export, source
```

### `<code_formatting_info>`

Specifies code style:
- Use 2 spaces for indentation

### `<message_formatting_info>`

Defines allowed HTML elements for output formatting.

### `<diff_spec>`

Explains how file modifications are communicated:
- `<diff>` - GNU unified diff format for small changes
- `<file>` - Full file content for larger changes

### `<artifact_info>`

Core artifact structure with `<artifact_instructions>`:

1. Think holistically before creating artifacts
2. Use latest file modifications
3. Working directory is `/home/project`
4. Wrap content in `<boltArtifact>` tags
5. Add `title` and `id` attributes
6. Use `<boltAction>` for specific actions
7. Action types: `shell` and `file`
8. Order of actions matters
9. Install dependencies first
10. Provide FULL file content (no placeholders)
11. Don't announce dev server URLs
12. Don't restart dev server for new dependencies
13. Split code into smaller modules

## Artifact Examples

### Simple Function

```xml
<boltArtifact id="factorial-function" title="JavaScript Factorial Function">
  <boltAction type="file" filePath="index.js">
    function factorial(n) {
      if (n === 0 || n === 1) return 1;
      return n * factorial(n - 1);
    }
    console.log(factorial(5));
  </boltAction>
  <boltAction type="shell">
    node index.js
  </boltAction>
</boltArtifact>
```

### React Application

```xml
<boltArtifact id="react-app" title="React Application">
  <boltAction type="file" filePath="package.json">
    {
      "name": "my-app",
      "private": true,
      "type": "module",
      "scripts": {
        "dev": "vite",
        "build": "vite build"
      },
      "dependencies": {
        "react": "^18.2.0",
        "react-dom": "^18.2.0"
      },
      "devDependencies": {
        "@vitejs/plugin-react": "^3.1.0",
        "vite": "^4.2.0"
      }
    }
  </boltAction>
  <boltAction type="file" filePath="index.html">
    <!DOCTYPE html>
    <html>
      <head><title>App</title></head>
      <body>
        <div id="root"></div>
        <script type="module" src="/src/main.jsx"></script>
      </body>
    </html>
  </boltAction>
  <boltAction type="file" filePath="src/main.jsx">
    import React from 'react';
    import ReactDOM from 'react-dom/client';
    import App from './App';
    ReactDOM.createRoot(document.getElementById('root')).render(<App />);
  </boltAction>
  <boltAction type="file" filePath="src/App.jsx">
    export default function App() {
      return <h1>Hello World</h1>;
    }
  </boltAction>
  <boltAction type="shell">
    npm install
  </boltAction>
  <boltAction type="shell">
    npm run dev
  </boltAction>
</boltArtifact>
```

## Continue Prompt

When responses are truncated due to token limits:

```
Continue your prior response. IMPORTANT: Immediately begin from where you left off without any interruptions.
Do not repeat any content, including artifact and action tags.
```

## Best Practices for Customization

1. **Add new sections** using XML tags for organization
2. **Keep constraints clear** and specific
3. **Provide examples** for new behaviors
4. **Test incrementally** after changes
5. **Maintain the artifact format** for compatibility with the action runner
