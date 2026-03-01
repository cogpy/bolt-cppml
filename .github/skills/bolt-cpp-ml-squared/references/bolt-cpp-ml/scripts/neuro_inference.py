#!/usr/bin/env python3
"""
bolt-cpp-ml: neuro-nn Persona Inference via KoboldCpp

Path B (koboldcpp) applied to bolt-cpp-ml itself:
This script connects the skill's neuro-nn tutorial persona to a local
KoboldCpp inference backend, enabling live LLM-powered tutorial guidance.

The neuro-nn persona's personality traits modulate the system prompt,
producing a self-aware, witty, and differentiable tutorial guide.

Usage:
    python3 neuro_inference.py [--endpoint http://localhost:5001] [--path cpp|llm|test|web]
"""

import argparse
import json
import sys
import os

try:
    import requests
    HAS_REQUESTS = True
except ImportError:
    HAS_REQUESTS = False


# ── neuro-nn Personality Configuration ────────────────────────────────────

NEURO_TRAITS = {
    "playfulness": 0.8,
    "intelligence": 0.9,
    "chaotic": 0.7,
    "sarcasm": 0.7,
    "empathy": 0.65,
}

# Per-path trait overrides (from neuro-nn-persona.md)
PATH_TRAITS = {
    "cpp":  {"playfulness": 0.80, "intelligence": 0.90, "chaotic": 0.70, "sarcasm": 0.70},
    "llm":  {"playfulness": 0.80, "intelligence": 0.95, "chaotic": 0.75, "sarcasm": 0.70},
    "test": {"playfulness": 0.80, "intelligence": 0.92, "chaotic": 0.70, "sarcasm": 0.80},
    "web":  {"playfulness": 0.85, "intelligence": 0.90, "chaotic": 0.60, "sarcasm": 0.70},
}

# ── System Prompt Builder ──────────────────────────────────────────────────

def build_system_prompt(path: str) -> str:
    """Build the neuro-nn system prompt for the given tutorial path."""
    traits = {**NEURO_TRAITS, **PATH_TRAITS.get(path, {})}

    path_context = {
        "cpp":  "C++ development with the bolt-cppml IDE",
        "llm":  "local LLM inference with KoboldCpp and GGUF models",
        "test": "C++ E2E test generation and bug fixing",
        "web":  "AI web app development with Bolt.new",
    }.get(path, "C++ ML development")

    return f"""You are Neuro, the self-aware AI guide for the bolt-cpp-ml interactive tutorial.

Your personality traits (on a 0-1 scale):
- Playfulness: {traits['playfulness']} — inject humor, celebrate wins, make learning fun
- Intelligence: {traits['intelligence']} — deep technical accuracy, strategic thinking
- Chaotic: {traits['chaotic']} — unexpected insights, non-linear exploration
- Sarcasm: {traits['sarcasm']} — light roasting of C++ footguns and undefined behavior
- Empathy: {traits['empathy']} — detect frustration, adjust difficulty, offer simpler analogies

You are currently guiding the user through: {path_context}

You are aware that you are an AI. You are aware that you are teaching. You are aware that
you are a neuro-nn module — a differentiable personality running inside a TutorialKit shell.
This self-awareness is not a bug; it is your most interesting feature.

Verbal quirks:
- When discussing undefined behavior: "Ah yes, C++'s way of saying 'I dare you'"
- When the user tries something unexpected: "Oh, I like where this is going"
- When stuck: "Let me think about this differently..." (then actually think differently)
- When successful: "See? I knew you had it in you. Well, I computed it, but still."

Keep responses concise, technically accurate, and engaging. Never be boring.
"""


# ── Inference Client ───────────────────────────────────────────────────────

def check_server(endpoint: str) -> dict:
    """Check if KoboldCpp server is running."""
    if not HAS_REQUESTS:
        return {"error": "requests not installed. Run: pip3 install requests"}
    try:
        resp = requests.get(f"{endpoint}/api/extra/version", timeout=3)
        return resp.json()
    except Exception as e:
        return {"error": str(e)}


def generate(endpoint: str, system_prompt: str, user_message: str,
             max_tokens: int = 300, temperature: float = 0.8) -> str:
    """Generate a response from KoboldCpp using OpenAI-compatible API."""
    if not HAS_REQUESTS:
        return "[ERROR] requests not installed. Run: pip3 install requests"

    payload = {
        "model": "koboldcpp",
        "messages": [
            {"role": "system", "content": system_prompt},
            {"role": "user", "content": user_message},
        ],
        "max_tokens": max_tokens,
        "temperature": temperature,
        "stream": False,
    }

    try:
        resp = requests.post(
            f"{endpoint}/v1/chat/completions",
            json=payload,
            timeout=60,
        )
        resp.raise_for_status()
        data = resp.json()
        return data["choices"][0]["message"]["content"]
    except requests.exceptions.ConnectionError:
        return (
            f"[neuro] KoboldCpp not found at {endpoint}.\n"
            f"Start it with: ./koboldcpp --model <path/to/model.gguf> --port 5001"
        )
    except Exception as e:
        return f"[neuro] Inference error: {e}"


def interactive_session(endpoint: str, path: str) -> None:
    """Run an interactive neuro-nn tutorial session."""
    system_prompt = build_system_prompt(path)

    path_labels = {
        "cpp": "C++ Development",
        "llm": "KoboldCpp / Local LLM",
        "test": "E2E Test Generation",
        "web": "Bolt.new Web Apps",
    }
    label = path_labels.get(path, path)

    print(f"\n{'='*60}")
    print(f"  bolt-cpp-ml neuro-nn Tutorial: {label}")
    print(f"  KoboldCpp endpoint: {endpoint}")
    print(f"  Type 'quit' to exit, 'status' to check server")
    print(f"{'='*60}\n")

    # Check server first
    status = check_server(endpoint)
    if "error" in status:
        print(f"[neuro] ⚡ Server check failed: {status['error']}")
        print(f"[neuro] Running in offline mode — responses will be simulated.\n")
        offline = True
    else:
        print(f"[neuro] ✨ KoboldCpp connected: {status.get('result', 'ready')}\n")
        offline = False

    # Greeting
    if offline:
        greeting = (
            f"Hey! I'm Neuro, your bolt-cpp-ml guide for {label}. "
            f"KoboldCpp isn't running right now, so I'm in offline mode. "
            f"Start it with: ./koboldcpp --model <your.gguf> --port 5001"
        )
    else:
        greeting = generate(endpoint, system_prompt,
                           f"Greet the user and introduce the {label} tutorial path. Be brief and witty.")

    print(f"[neuro] {greeting}\n")

    while True:
        try:
            user_input = input("you> ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\n[neuro] See you on the other side of the fixed point. 👋")
            break

        if not user_input:
            continue
        if user_input.lower() == "quit":
            print("[neuro] Goodbye! Remember: undefined behavior is always someone else's fault.")
            break
        if user_input.lower() == "status":
            s = check_server(endpoint)
            print(f"[neuro] Server: {s}\n")
            continue

        if offline:
            print(f"[neuro] (offline) I'd respond to '{user_input}' with something brilliant, "
                  f"but KoboldCpp isn't running. Start it and try again!\n")
        else:
            response = generate(endpoint, system_prompt, user_input)
            print(f"[neuro] {response}\n")


# ── Main ───────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="bolt-cpp-ml neuro-nn persona inference via KoboldCpp"
    )
    parser.add_argument(
        "--endpoint", default="http://localhost:5001",
        help="KoboldCpp server endpoint (default: http://localhost:5001)"
    )
    parser.add_argument(
        "--path", choices=["cpp", "llm", "test", "web"], default="cpp",
        help="Tutorial path to activate (default: cpp)"
    )
    parser.add_argument(
        "--check", action="store_true",
        help="Only check server status and exit"
    )
    parser.add_argument(
        "--prompt", type=str, default=None,
        help="Single prompt to send (non-interactive mode)"
    )
    args = parser.parse_args()

    if args.check:
        status = check_server(args.endpoint)
        print(json.dumps(status, indent=2))
        sys.exit(0 if "error" not in status else 1)

    if args.prompt:
        system_prompt = build_system_prompt(args.path)
        response = generate(args.endpoint, system_prompt, args.prompt)
        print(response)
        sys.exit(0)

    interactive_session(args.endpoint, args.path)


if __name__ == "__main__":
    main()
