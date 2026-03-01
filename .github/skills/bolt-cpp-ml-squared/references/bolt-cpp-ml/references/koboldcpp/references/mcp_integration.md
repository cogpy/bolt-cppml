# MCP Server Integration with Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)

This guide covers creating Model Context Protocol (MCP) servers that integrate with Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml).

## Table of Contents
1. [Overview](#overview)
2. [Basic MCP Server](#basic-mcp-server)
3. [Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) Tools](#bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)-tools)
4. [Configuration](#configuration)

## Overview

Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) can serve as a backend for MCP servers, enabling AI-powered tools in Claude Desktop, VS Code, and other MCP clients.

## Basic MCP Server

Create an MCP server using FastMCP:

```python
from fastmcp import FastMCP
import requests

mcp = FastMCP("Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) Tools", stateless_http=True)

KCPP_ENDPOINT = "http://localhost:5001"

@mcp.tool()
def generate_text(prompt: str, max_tokens: int = 100) -> str:
    """Generate text using the local LLM."""
    response = requests.post(
        f"{KCPP_ENDPOINT}/v1/completions",
        json={"prompt": prompt, "max_tokens": max_tokens}
    )
    return response.json()["choices"][0]["text"]

@mcp.tool()
def chat(message: str) -> str:
    """Chat with the local LLM."""
    response = requests.post(
        f"{KCPP_ENDPOINT}/v1/chat/completions",
        json={"messages": [{"role": "user", "content": message}]}
    )
    return response.json()["choices"][0]["message"]["content"]

if __name__ == "__main__":
    import uvicorn
    http_app = mcp.http_app(transport="streamable-http", path="/mcp")
    uvicorn.run(http_app, host="0.0.0.0", port=8000)
```

## Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) Tools

### Text Generation Tool

```python
@mcp.tool()
def creative_write(
    prompt: str,
    style: str = "narrative",
    length: str = "medium"
) -> str:
    """
    Generate creative writing with the local LLM.
    
    Args:
        prompt: The writing prompt or starting text
        style: Writing style (narrative, dialogue, poetry, technical)
        length: Output length (short=50, medium=200, long=500 tokens)
    """
    length_map = {"short": 50, "medium": 200, "long": 500}
    max_tokens = length_map.get(length, 200)
    
    style_prompts = {
        "narrative": "Write a narrative story: ",
        "dialogue": "Write a dialogue: ",
        "poetry": "Write a poem: ",
        "technical": "Write technical documentation: "
    }
    
    full_prompt = style_prompts.get(style, "") + prompt
    
    response = requests.post(
        f"{KCPP_ENDPOINT}/v1/completions",
        json={
            "prompt": full_prompt,
            "max_tokens": max_tokens,
            "temperature": 0.8
        }
    )
    return response.json()["choices"][0]["text"]
```

### Code Assistant Tool

```python
@mcp.tool()
def code_assist(
    code: str,
    task: str = "explain"
) -> str:
    """
    Get coding assistance from the local LLM.
    
    Args:
        code: The code to analyze or modify
        task: Task type (explain, review, refactor, document)
    """
    task_prompts = {
        "explain": f"Explain this code:\n```\n{code}\n```\n\nExplanation:",
        "review": f"Review this code for issues:\n```\n{code}\n```\n\nReview:",
        "refactor": f"Refactor this code:\n```\n{code}\n```\n\nRefactored:",
        "document": f"Add documentation to this code:\n```\n{code}\n```\n\nDocumented:"
    }
    
    prompt = task_prompts.get(task, task_prompts["explain"])
    
    response = requests.post(
        f"{KCPP_ENDPOINT}/v1/completions",
        json={"prompt": prompt, "max_tokens": 500, "temperature": 0.3}
    )
    return response.json()["choices"][0]["text"]
```

### Summarization Tool

```python
@mcp.tool()
def summarize(text: str, style: str = "brief") -> str:
    """
    Summarize text using the local LLM.
    
    Args:
        text: Text to summarize
        style: Summary style (brief, detailed, bullets)
    """
    style_instructions = {
        "brief": "Provide a one-paragraph summary:",
        "detailed": "Provide a detailed summary with key points:",
        "bullets": "Summarize in bullet points:"
    }
    
    prompt = f"{style_instructions.get(style, style_instructions['brief'])}\n\n{text}\n\nSummary:"
    
    response = requests.post(
        f"{KCPP_ENDPOINT}/v1/completions",
        json={"prompt": prompt, "max_tokens": 300, "temperature": 0.5}
    )
    return response.json()["choices"][0]["text"]
```

## Configuration

### MCP Client Configuration (Claude Desktop)

Add to `claude_desktop_config.json`:

```json
{
  "mcpServers": {
    "bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)": {
      "url": "http://localhost:8000/mcp"
    }
  }
}
```

### Environment Variables

```bash
export KCPP_ENDPOINT="http://localhost:5001"
export MCP_PORT=8000
```

### Running with CORS

```python
from starlette.middleware import Middleware
from starlette.middleware.cors import CORSMiddleware

middleware = [
    Middleware(CORSMiddleware, allow_origins=["*"], allow_methods=["*"], allow_headers=["*"])
]

http_app = mcp.http_app(transport="streamable-http", path="/mcp", middleware=middleware)
```

## Complete Example

See `/home/ubuntu/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)/examples/demo_mcp.py` for a complete working example.
