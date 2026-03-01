# bolt-cpp-ml Jan Extension

A Jan extension that brings the full `bolt-cpp-ml` meta-skill into the Jan AI assistant platform.

## What It Does

This extension exposes all four `bolt-cpp-ml` capability paths as Jan commands:

| Command | Description |
|---|---|
| `bolt-cpp-ml.startTutorial` | Launch the interactive TutorialKit tutorial with neuro-nn persona |
| `bolt-cpp-ml.runKoboldCpp` | Check/start the KoboldCpp local LLM inference server |
| `bolt-cpp-ml.generateTests` | Run cpp-e2e-test-gen on the current C++ project |
| `bolt-cpp-ml.scaffoldBoltApp` | Scaffold a new Bolt.new AI web application |

It also registers as an inference backend, routing messages to the local KoboldCpp/bolt-cppml GGML engine when the model ID contains `koboldcpp` or `bolt-cppml`.

## Installation

```bash
npm install
npm run bundle
# → produces @cogpy-bolt-cpp-ml-extension-1.0.0.tgz
```

Install in Jan: Settings → Extensions → Install from file → select the `.tgz`.

## Configuration

The extension uses these defaults (configurable in Jan settings):

```json
{
  "koboldEndpoint": "http://localhost:5001",
  "tutorialPort": 4321,
  "neuroPersonality": {
    "playfulness": 0.8,
    "intelligence": 0.9,
    "chaotic": 0.7,
    "sarcasm": 0.7
  }
}
```

## The Self-Application

This extension is the result of applying `bolt-cpp-ml` to itself (Path C: janext):

```
bolt-cpp-ml(bolt-cpp-ml) → Jan extension
```

The skill packages itself, demonstrating the fixed-point property: the output of applying the skill is a valid instance of the skill in a new domain.
