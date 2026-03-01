---
name: bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)
description: Part of the bolt-cpp-ml skill: Build applications with Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) LLM inference engine.
---

# Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) Integration Skill

Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) is a self-contained LLM inference engine supporting GGUF models with multiple APIs and multimodal capabilities.

## Quick Start

### API Connection

```python
import requests

ENDPOINT = "http://localhost:5001"

# Text generation (OpenAI-compatible)
response = requests.post(f"{ENDPOINT}/v1/completions", json={
    "prompt": "Hello, ",
    "max_tokens": 100,
    "temperature": 0.7
})
print(response.json()["choices"][0]["text"])

# Chat completion
response = requests.post(f"{ENDPOINT}/v1/chat/completions", json={
    "messages": [{"role": "user", "content": "Hello!"}],
    "max_tokens": 100
})
print(response.json()["choices"][0]["message"]["content"])
```

### Streaming Responses

```python
import requests

response = requests.post(
    "http://localhost:5001/v1/chat/completions",
    json={"messages": [{"role": "user", "content": "Tell me a story"}], "stream": True},
    stream=True
)
for line in response.iter_lines():
    if line and line.startswith(b"data: "):
        print(line.decode()[6:])
```

## Core Workflows

### 1. Server Setup

**Local deployment:**
```bash
# Download from https://github.com/LostRuins/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)/releases
./bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) --model model.gguf --port 5001 --gpulayers 99
```

**Key flags:**
- `--gpulayers N` - Offload N layers to GPU (use 99 for all)
- `--contextsize N` - Max context (default 4096)
- `--usecuda` / `--usevulkan` - GPU backend
- `--quiet` - Suppress logs

### 2. API Integration

**Determine API type:**
- **OpenAI-compatible** → Use `/v1/completions` or `/v1/chat/completions`
- **KoboldAI native** → Use `/api/v1/generate`
- **Image generation** → Use `/sdapi/v1/txt2img`
- **Speech-to-text** → Use `/api/extra/whisper`
- **Text-to-speech** → Use `/api/extra/tts`

### 3. Mobile/Web App Integration

For apps connecting to Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) backend:
1. Configure CORS if needed (Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) allows all origins by default)
2. Use OpenAI-compatible endpoints for broad compatibility
3. Implement streaming for real-time responses
4. Handle connection errors gracefully

## API Reference

### Text Generation

**POST `/v1/completions`** (OpenAI-compatible)
```json
{
  "prompt": "string",
  "max_tokens": 100,
  "temperature": 0.7,
  "top_p": 0.9,
  "stop": ["\\n"],
  "stream": false
}
```

**POST `/v1/chat/completions`** (OpenAI-compatible)
```json
{
  "messages": [{"role": "user", "content": "Hello"}],
  "max_tokens": 100,
  "temperature": 0.7,
  "stream": false
}
```

**POST `/api/v1/generate`** (KoboldAI native)
```json
{
  "prompt": "string",
  "max_context_length": 4096,
  "max_length": 100,
  "temperature": 0.8,
  "top_k": 100,
  "top_p": 0.9,
  "rep_pen": 1.1,
  "rep_pen_range": 512
}
```

### Multimodal

**Image Generation** - See `references/image_generation.md`
**Speech-to-Text** - See `references/speech.md`
**Text-to-Speech** - See `references/speech.md`

## Generation Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `temperature` | float | 0.7 | Randomness (0.0-2.0) |
| `top_k` | int | 40 | Keep top K tokens |
| `top_p` | float | 0.9 | Nucleus sampling |
| `rep_pen` | float | 1.1 | Repetition penalty |
| `max_tokens` | int | 100 | Max generation length |
| `stop` | array | [] | Stop sequences |
| `stream` | bool | false | Enable SSE streaming |

## Chat Templates

Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) auto-detects chat templates. For manual override, use adapters:

- **ChatML** (Qwen, Phi): `<|im_start|>user\n...<|im_end|>`
- **Llama 3**: `<|start_header_id|>user<|end_header_id|>\n\n...<|eot_id|>`
- **Gemma**: `<start_of_turn>user\n...<end_of_turn>`

## Scripts

### Test Connection
Run `scripts/test_connection.py` to verify Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) is running:
```bash
python scripts/test_connection.py http://localhost:5001
```

### Generate Client
Run `scripts/generate_client.py` to create a typed API client:
```bash
python scripts/generate_client.py --output client.py
```

## Advanced Topics

- **MCP Server Integration**: See `references/mcp_integration.md`
- **Image Generation (SD/SDXL)**: See `references/image_generation.md`
- **Speech (Whisper/TTS)**: See `references/speech.md`
- **Build from Source**: See `references/building.md`
