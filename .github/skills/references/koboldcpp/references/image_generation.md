# Image Generation with Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)

Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) includes Stable Diffusion support for image generation via an A1111-compatible API.

## Table of Contents
1. [Setup](#setup)
2. [Text-to-Image](#text-to-image)
3. [Image-to-Image](#image-to-image)
4. [Parameters](#parameters)

## Setup

Load an SD model alongside your LLM:

```bash
./bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) --model llm.gguf --sdmodel sd_model.safetensors --port 5001
```

Supported formats:
- SD 1.5 (.safetensors)
- SDXL (.safetensors)
- SD3 (.safetensors)
- Flux (.safetensors)

## Text-to-Image

### Basic Generation

```python
import requests
import base64

response = requests.post(
    "http://localhost:5001/sdapi/v1/txt2img",
    json={
        "prompt": "a beautiful sunset over mountains, detailed, 4k",
        "negative_prompt": "blurry, low quality",
        "width": 512,
        "height": 512,
        "steps": 20,
        "cfg_scale": 7.0
    }
)

# Decode and save image
image_data = response.json()["images"][0]
with open("output.png", "wb") as f:
    f.write(base64.b64decode(image_data))
```

### With Seed Control

```python
response = requests.post(
    "http://localhost:5001/sdapi/v1/txt2img",
    json={
        "prompt": "cyberpunk city at night",
        "seed": 42,  # Reproducible results
        "width": 768,
        "height": 512,
        "steps": 25
    }
)
```

## Image-to-Image

```python
import base64

# Load source image
with open("source.png", "rb") as f:
    source_b64 = base64.b64encode(f.read()).decode()

response = requests.post(
    "http://localhost:5001/sdapi/v1/img2img",
    json={
        "init_images": [source_b64],
        "prompt": "oil painting style",
        "denoising_strength": 0.7,
        "steps": 20
    }
)
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `prompt` | str | "" | Positive prompt |
| `negative_prompt` | str | "" | Negative prompt |
| `width` | int | 512 | Image width |
| `height` | int | 512 | Image height |
| `steps` | int | 20 | Sampling steps |
| `cfg_scale` | float | 7.0 | Classifier-free guidance |
| `seed` | int | -1 | Random seed (-1 = random) |
| `sampler_name` | str | "euler" | Sampler algorithm |
| `denoising_strength` | float | 0.7 | For img2img (0-1) |

### Available Samplers

Query available samplers:
```python
response = requests.get("http://localhost:5001/sdapi/v1/samplers")
samplers = [s["name"] for s in response.json()]
```

Common samplers: `euler`, `euler_a`, `dpm++_2m`, `dpm++_sde`

## ComfyUI-Compatible API

Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) also provides ComfyUI-compatible endpoints:

```python
# Get available models
response = requests.get("http://localhost:5001/api/models/checkpoints")

# View last generated image
response = requests.get("http://localhost:5001/view")
```
