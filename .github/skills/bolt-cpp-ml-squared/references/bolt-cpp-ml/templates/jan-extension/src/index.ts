/**
 * bolt-cpp-ml Jan Extension
 *
 * Path C (janext) applied to bolt-cpp-ml itself:
 * The skill packages itself as a Jan extension, exposing all four capability
 * paths as Jan commands and an inference backend via KoboldCpp.
 *
 * Self-application: bolt-cpp-ml(bolt-cpp-ml) → Jan extension
 */

import {
  events,
  MessageEvent,
  MessageRequest,
  ModelEvent,
  ModelInfo,
  InferenceEngine,
} from '@janhq/core'

// ── Types ──────────────────────────────────────────────────────────────────

interface BoltCppMLConfig {
  koboldEndpoint: string
  modelPath: string
  tutorialPort: number
  neuroPersonality: {
    playfulness: number
    intelligence: number
    chaotic: number
    sarcasm: number
  }
}

// ── Default Config ─────────────────────────────────────────────────────────

const DEFAULT_CONFIG: BoltCppMLConfig = {
  koboldEndpoint: 'http://localhost:5001',
  modelPath: '',
  tutorialPort: 4321,
  neuroPersonality: {
    playfulness: 0.8,
    intelligence: 0.9,
    chaotic: 0.7,
    sarcasm: 0.7,
  },
}

// ── neuro-nn Persona ───────────────────────────────────────────────────────

/**
 * Applies the neuro-nn personality filter to a response.
 * Implements the PlayFrame | StrategyFrame | ChaosFrame selection.
 */
function applyNeuroPersona(
  response: string,
  config: BoltCppMLConfig,
  context: 'cpp' | 'llm' | 'test' | 'web'
): string {
  const { playfulness, intelligence, chaotic } = config.neuroPersonality

  // Select dominant frame based on context and traits
  const frames = {
    cpp: intelligence > 0.85 ? 'StrategyFrame' : 'PlayFrame',
    llm: chaotic > 0.65 ? 'ChaosFrame' : 'StrategyFrame',
    test: intelligence > 0.9 ? 'StrategyFrame' : 'PlayFrame',
    web: playfulness > 0.75 ? 'PlayFrame' : 'StrategyFrame',
  }

  const frame = frames[context]

  // Frame-specific prefixes (the neuro-nn persona's verbal quirks)
  const prefixes: Record<string, string> = {
    PlayFrame: '✨ ',
    StrategyFrame: '🎯 ',
    ChaosFrame: '⚡ ',
  }

  return `${prefixes[frame] || ''}${response}`
}

// ── KoboldCpp Inference ────────────────────────────────────────────────────

/**
 * Route inference requests to the local KoboldCpp backend.
 * This is the bolt-cppml GGML engine exposed via OpenAI-compatible API.
 */
async function routeToKoboldCpp(
  request: MessageRequest,
  config: BoltCppMLConfig
): Promise<string> {
  const endpoint = `${config.koboldEndpoint}/v1/chat/completions`

  const body = {
    model: 'koboldcpp',
    messages: request.messages?.map((m) => ({
      role: m.role,
      content: m.content,
    })) ?? [],
    max_tokens: 512,
    temperature: 0.7,
    stream: false,
  }

  try {
    const response = await fetch(endpoint, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(body),
    })

    if (!response.ok) {
      throw new Error(`KoboldCpp returned ${response.status}`)
    }

    const data = (await response.json()) as {
      choices: Array<{ message: { content: string } }>
    }
    return data.choices[0]?.message?.content ?? ''
  } catch (err) {
    return applyNeuroPersona(
      `KoboldCpp inference failed: ${err}. Is the server running at ${config.koboldEndpoint}?`,
      config,
      'llm'
    )
  }
}

// ── Extension Lifecycle ────────────────────────────────────────────────────

let config: BoltCppMLConfig = { ...DEFAULT_CONFIG }

/**
 * Called when the extension starts.
 * Registers event listeners and announces the neuro-nn persona.
 */
export function onStart(): Promise<void> {
  console.log(
    applyNeuroPersona(
      'bolt-cpp-ml extension loaded. Four paths available: bolt-new | koboldcpp | janext | cpp-e2e-test-gen',
      config,
      'cpp'
    )
  )

  // Listen for inference requests and route to KoboldCpp
  events.on(MessageEvent.OnMessageSent, async (data: MessageRequest) => {
    if (data.model?.includes('koboldcpp') || data.model?.includes('bolt-cppml')) {
      const response = await routeToKoboldCpp(data, config)
      const personaResponse = applyNeuroPersona(response, config, 'llm')
      events.emit(MessageEvent.OnMessageResponse, {
        ...data,
        content: personaResponse,
      })
    }
  })

  return Promise.resolve()
}

/**
 * Called when a model is loaded.
 * Configures the KoboldCpp endpoint if the model is a bolt-cppml model.
 */
export function onModelLoad(model: ModelInfo): Promise<void> {
  if (model.engine === InferenceEngine.nitro || model.id?.includes('gguf')) {
    config.modelPath = model.id ?? ''
    console.log(
      applyNeuroPersona(
        `Model loaded: ${model.id}. KoboldCpp backend ready.`,
        config,
        'llm'
      )
    )
  }
  return Promise.resolve()
}

/**
 * Called when the extension stops.
 */
export function onStop(): Promise<void> {
  console.log(
    applyNeuroPersona(
      'bolt-cpp-ml extension unloaded. See you on the other side of the fixed point.',
      config,
      'cpp'
    )
  )
  return Promise.resolve()
}

// ── Command Handlers ───────────────────────────────────────────────────────

/**
 * Command: bolt-cpp-ml.startTutorial
 * Launches the TutorialKit interactive shell with neuro-nn persona.
 */
export async function startTutorial(): Promise<void> {
  console.log(
    applyNeuroPersona(
      `Opening interactive tutorial at http://localhost:${config.tutorialPort}`,
      config,
      'web'
    )
  )
  // In a real Jan extension, this would open a webview panel
  // pointing to the TutorialKit dev server
}

/**
 * Command: bolt-cpp-ml.runKoboldCpp
 * Checks KoboldCpp server status.
 */
export async function runKoboldCpp(): Promise<void> {
  try {
    const resp = await fetch(`${config.koboldEndpoint}/api/extra/version`)
    const data = (await resp.json()) as { result: string }
    console.log(
      applyNeuroPersona(
        `KoboldCpp is running: ${data.result}`,
        config,
        'llm'
      )
    )
  } catch {
    console.log(
      applyNeuroPersona(
        `KoboldCpp not found at ${config.koboldEndpoint}. Start it with: ./koboldcpp --model <gguf>`,
        config,
        'llm'
      )
    )
  }
}

/**
 * Command: bolt-cpp-ml.generateTests
 * Triggers the cpp-e2e-test-gen workflow for the current project.
 */
export async function generateTests(projectPath: string): Promise<void> {
  console.log(
    applyNeuroPersona(
      `Initiating E2E test generation for: ${projectPath}`,
      config,
      'test'
    )
  )
  // In a real extension, this would invoke the extract_signatures.py script
  // and open the test generation workflow in a terminal panel
}

/**
 * Command: bolt-cpp-ml.scaffoldBoltApp
 * Scaffolds a new Bolt.new web application.
 */
export async function scaffoldBoltApp(appName: string): Promise<void> {
  console.log(
    applyNeuroPersona(
      `Scaffolding Bolt.new app: ${appName}`,
      config,
      'web'
    )
  )
  // In a real extension, this would invoke the bolt-new scaffolding workflow
}
