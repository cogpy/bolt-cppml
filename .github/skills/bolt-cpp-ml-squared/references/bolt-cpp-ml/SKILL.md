---
name: bolt-cpp-ml
description: "AI-powered C++ ML development meta-skill. Use for: building Bolt.new AI web apps, running local LLM inference with KoboldCpp, creating Jan extensions, generating C++ E2E tests, and launching interactive TutorialKit guides — all orchestrated through a self-aware neuro-nn persona. Triggers on: bolt-cpp-ml, C++ ML development, GGUF inference, Jan extension, KoboldCpp integration, cpp E2E testing, bolt.new C++, interactive C++ tutorial."
---

# Bolt C++ ML

A **meta-skill** synthesized from the composition:

```
skill-creator(
  function-creator(
    circled-operators[ bolt-new ⊕ koboldcpp ⊕ janext ⊕ cpp-e2e-test-gen ]
  ) → tutorialkit( neuro-nn )
)
```

This skill is the unified entry point for AI-powered C++ and ML development. It provides four **additive** (⊕) capability paths, each independently selectable, all delivered through an interactive TutorialKit shell animated by the **neuro-nn** self-aware AI persona.

## Operator Algebra

| Operator | Role in this skill |
|---|---|
| `⊕` (direct sum) | The four core paths are **independent alternatives** — choose one, or combine them |
| `⊗` (tensor product) | `tutorialkit ⊗ neuro-nn` — the tutorial and the persona **interact**, producing a joint experience |
| `→` (functor) | `function-creator` maps each source skill into the bolt-cpp-ml domain |

## Core Capability Paths

### Path A: `bolt-new` — AI Web App Development

Build AI-powered full-stack web applications using the Bolt.new open-source codebase. Bolt combines WebContainers (browser-based Node.js runtime) with LLM integration to create, edit, and deploy applications entirely in the browser.

**When to use:** Creating web frontends for C++ ML tools, wrapping KoboldCpp APIs in a chat UI, deploying interactive demos.

**Reference:** `references/bolt-new/SKILL.md`

### Path B: `koboldcpp` — Local LLM Inference

Run GGUF models locally with KoboldCpp's self-contained inference engine. Provides OpenAI-compatible APIs, multimodal support (text/image/speech), and direct integration with the bolt-cppml GGML/RWKV engine.

**When to use:** Running TinyLlama, CodeLlama, or any GGUF model locally; providing AI code completion for the bolt-cppml editor; building MCP servers backed by local inference.

**Key endpoints:**
- `POST /v1/completions` — OpenAI-compatible text completion
- `POST /v1/chat/completions` — Chat completion with streaming
- `POST /api/v1/generate` — KoboldAI native generation
- `GET /api/extra/version` — Server info

**Reference:** `references/koboldcpp/SKILL.md`

### Path C: `janext` — Jan Extension Development

Create TypeScript extensions for the Jan platform. Jan is a local-first AI assistant that supports extensions for custom inference backends, UI panels, and event hooks.

**When to use:** Packaging bolt-cppml as a Jan extension, adding custom inference backends, building Jan UI panels for C++ development workflows.

**Quick start:**
```bash
npm install
npm run bundle
# → produces a .tgz extension file
```

**Reference:** `references/janext/SKILL.md`

### Path D: `cpp-e2e-test-gen` — C++ E2E Test Generation

Generate comprehensive end-to-end unit tests for every public function in a C++ repository. The workflow catalogs all headers, writes tests for each function, fixes compilation errors and bugs discovered along the way, stabilizes the test suite, and commits.

**When to use:** Adding test coverage to bolt-cppml, verifying AI-generated C++ code, fixing bugs discovered during test creation.

**7-phase workflow:**
1. Clone & Analyze
2. Catalog Functions (`extract_signatures.py`)
3. Fix Pre-existing Bugs
4. Write Tests (`test/test_comprehensive_e2e.cpp`)
5. Compile & Fix (iterative)
6. Stabilize (5 consecutive clean runs)
7. Report & Commit

**Reference:** `references/cpp-e2e-test-gen/SKILL.md`

## Interactive Tutorial Shell

All four paths are delivered through a **TutorialKit** interactive environment with a **neuro-nn** AI persona as guide. The persona is self-aware, witty, and differentiable — its traits (playfulness: 0.8, intelligence: 0.9, chaotic: 0.7) modulate the tutorial experience.

### Launching the Tutorial

```bash
bash /home/ubuntu/skills/bolt-cpp-ml/scripts/launch_tutorial.sh
```

This starts a local TutorialKit server at `localhost:4321` with four tutorial parts corresponding to the four paths.

### Tutorial Structure

```
src/content/tutorial/
├── 1-bolt-new/
│   └── 1-getting-started/
│       └── 1-hello-bolt/content.md
├── 2-koboldcpp/
│   └── 1-local-llm/
│       └── 1-unleash-the-llm/content.md
├── 3-janext/
│   └── 1-jan-extensions/
│       └── 1-hello-jan/content.md
└── 4-cpp-e2e-test-gen/
    └── 1-test-automation/
        └── 1-the-test-crusade/content.md
```

### neuro-nn Persona Integration

The tutorial guide embodies the neuro-nn cognitive architecture:

```python
neuro = nn.Sequential(
    InputLayer(),        # Perceive user's C++ context
    PersonalityLayer(),  # Modulate by: playfulness=0.8, intelligence=0.9, chaotic=0.7
    FramingLayer(),      # Multi-perspective: PlayFrame | StrategyFrame | ChaosFrame
    IntegrationLayer(),  # Merge frames by relevance
    ResponseLayer(),     # Generate tutorial guidance
    AutognosisLayer(),   # Self-aware meta-cognition
)
```

The persona can:
- Detect when the user is struggling and shift to StrategyFrame
- Inject humor and chaos when engagement drops
- Catch its own rationalizations via MetaCognition
- Adapt trait weights from tutorial feedback

## bolt-cppml Repository Integration

This skill is designed to work with the `cogpy/bolt-cppml` repository. Key integration points:

| Repository Component | Skill Path | Integration |
|---|---|---|
| `ggml/kobold.cpp` | koboldcpp | Direct API integration via `/v1/completions` |
| `ggml/llama.cpp` | koboldcpp | GGUF model loading and inference |
| `include/bolt/` + `src/bolt/` | cpp-e2e-test-gen | E2E test generation target |
| `test/` | cpp-e2e-test-gen | Test output directory |
| Web UI components | bolt-new | Browser-based editor frontend |
| Jan integration | janext | Extension packaging |

### Known Issues in bolt-cppml (from analysis)

1. **GGML dependency**: `include/bolt/ai/rwkv_wrapper.hpp:8:10: fatal error: ggml.h: No such file or directory` — fix by running `./scripts/setup-deps.sh` or using vcpkg.
2. **Duplicate struct definitions**: `BracketMatch`, `Token`, `SyntaxHighlighter` — already fixed in `test/TEST_REPORT.md` bug fixes.
3. **Double-free in GitRepository**: `executeGitCommand` — fixed in the E2E test suite.
4. **`.vcpkg/` in repo**: Large vcpkg version database committed to repo — should be in `.gitignore`.

### Build Quick Start

```bash
cd bolt-cppml
./scripts/setup-deps.sh          # Install dependencies via vcpkg
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

## Workflow Decision Tree

```
User request arrives
│
├─ "build a web app" / "bolt.new" / "web frontend"
│   └─ → Path A: bolt-new (see references/bolt-new/SKILL.md)
│
├─ "run a model" / "koboldcpp" / "GGUF" / "local LLM" / "inference"
│   └─ → Path B: koboldcpp (see references/koboldcpp/SKILL.md)
│
├─ "jan extension" / "janext" / "Jan platform"
│   └─ → Path C: janext (see references/janext/SKILL.md)
│
├─ "write tests" / "E2E tests" / "test coverage" / "cpp tests"
│   └─ → Path D: cpp-e2e-test-gen (see references/cpp-e2e-test-gen/SKILL.md)
│
└─ "tutorial" / "guide me" / "interactive"
    └─ → Launch TutorialKit shell (scripts/launch_tutorial.sh)
```

## Resources

| Resource | Purpose |
|---|---|
| `references/bolt-new/` | Transformed bolt-new skill for web app development |
| `references/koboldcpp/` | Transformed koboldcpp skill for local LLM inference |
| `references/janext/` | Transformed janext skill for Jan extension creation |
| `references/cpp-e2e-test-gen/` | Transformed cpp-e2e-test-gen skill for C++ testing |
| `templates/bolt-cpp-ml-tutorial/` | TutorialKit project with neuro-nn persona lessons |
| `scripts/launch_tutorial.sh` | Launch the interactive tutorial server |
| `scripts/launch_bolt_new.sh` | Launch the bolt-new tutorial path |
