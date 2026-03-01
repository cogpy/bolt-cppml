# Speech Processing with Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)

Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) supports speech-to-text (Whisper) and text-to-speech (OuteTTS, Kokoro, Parler, Dia).

## Table of Contents
1. [Speech-to-Text (Whisper)](#speech-to-text-whisper)
2. [Text-to-Speech](#text-to-speech)
3. [Voice Cloning](#voice-cloning)

## Speech-to-Text (Whisper)

### Setup

Load a Whisper model:
```bash
./bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) --model llm.gguf --whispermodel whisper-base.bin --port 5001
```

Download Whisper models from: https://huggingface.co/ggerganov/whisper.cpp

### Transcription API

```python
import requests
import base64

# Load audio file
with open("audio.mp3", "rb") as f:
    audio_b64 = base64.b64encode(f.read()).decode()

response = requests.post(
    "http://localhost:5001/api/extra/whisper",
    json={
        "audio_data": audio_b64,
        "suppress_non_speech": True,
        "langcode": "en"  # Optional language hint
    }
)

transcription = response.json()["text"]
print(transcription)
```

### OpenAI-Compatible Endpoint

```python
response = requests.post(
    "http://localhost:5001/v1/audio/transcriptions",
    files={"file": open("audio.mp3", "rb")},
    data={"model": "whisper-1"}
)
print(response.json()["text"])
```

## Text-to-Speech

### Setup

Load a TTS model:
```bash
./bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) --model llm.gguf --ttsmodel outetts-0.2.bin --port 5001
```

Supported TTS engines:
- OuteTTS
- Kokoro
- Parler
- Dia

### Basic TTS

```python
import requests
import base64

response = requests.post(
    "http://localhost:5001/api/extra/tts",
    json={
        "input": "Hello, this is a test of text to speech.",
        "speaker_seed": 42  # Voice consistency
    }
)

# Save audio
audio_data = response.json()["audio"]
with open("output.wav", "wb") as f:
    f.write(base64.b64decode(audio_data))
```

### OpenAI-Compatible Endpoint

```python
response = requests.post(
    "http://localhost:5001/v1/audio/speech",
    json={
        "input": "Hello world!",
        "model": "tts-1",
        "voice": "alloy"
    }
)

with open("speech.mp3", "wb") as f:
    f.write(response.content)
```

### Available Voices

```python
response = requests.get("http://localhost:5001/v1/audio/voices")
voices = response.json()["voices"]
```

## Voice Cloning

With OuteTTS, you can clone voices from audio samples:

```python
import base64

# Load reference audio
with open("reference_voice.wav", "rb") as f:
    voice_b64 = base64.b64encode(f.read()).decode()

response = requests.post(
    "http://localhost:5001/api/extra/tts",
    json={
        "input": "Text to speak in the cloned voice.",
        "custom_speaker_data": voice_b64,
        "custom_speaker_text": "Transcription of the reference audio."
    }
)
```

See `/home/ubuntu/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)/examples/outetts/voice_cloning.py` for a complete example.

## Parameters

### Whisper Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `audio_data` | str | Base64-encoded audio |
| `suppress_non_speech` | bool | Filter non-speech sounds |
| `langcode` | str | Language code (en, es, fr, etc.) |

### TTS Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `input` | str | Text to synthesize |
| `speaker_seed` | int | Voice seed for consistency |
| `audio_seed` | int | Audio generation seed |
| `custom_speaker_voice` | str | Reference audio (base64) |
| `custom_speaker_text` | str | Reference transcription |
