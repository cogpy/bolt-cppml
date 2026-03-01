#!/usr/bin/env python3
"""Generate a typed Python client for Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) API."""

import argparse

CLIENT_TEMPLATE = '''"""Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) API Client - Auto-generated typed client."""

from dataclasses import dataclass, field
from typing import Optional, List, Iterator
import requests
import json


@dataclass
class GenerationParams:
    """Parameters for text generation."""
    prompt: str = ""
    max_tokens: int = 100
    temperature: float = 0.7
    top_p: float = 0.9
    top_k: int = 40
    rep_pen: float = 1.1
    rep_pen_range: int = 512
    stop: List[str] = field(default_factory=list)
    stream: bool = False


@dataclass
class ChatMessage:
    """A chat message."""
    role: str  # "system", "user", or "assistant"
    content: str


@dataclass
class GenerationResult:
    """Result from text generation."""
    text: str
    prompt_tokens: int = 0
    completion_tokens: int = 0
    stop_reason: str = ""


class Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)Client:
    """Typed client for Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) API."""
    
    def __init__(self, endpoint: str = "http://localhost:5001", timeout: int = 120):
        """
        Initialize the client.
        
        Args:
            endpoint: Base URL of the Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) server
            timeout: Request timeout in seconds
        """
        self.endpoint = endpoint.rstrip("/")
        self.timeout = timeout
        self._session = requests.Session()
    
    def is_connected(self) -> bool:
        """Check if server is reachable."""
        try:
            response = self._session.get(f"{self.endpoint}/api/v1/model", timeout=5)
            return response.status_code == 200
        except:
            return False
    
    def get_model(self) -> Optional[str]:
        """Get the currently loaded model name."""
        try:
            response = self._session.get(f"{self.endpoint}/api/v1/model", timeout=5)
            if response.status_code == 200:
                return response.json().get("result")
        except:
            pass
        return None
    
    def complete(self, params: GenerationParams) -> GenerationResult:
        """
        Generate text completion using OpenAI-compatible API.
        
        Args:
            params: Generation parameters
            
        Returns:
            GenerationResult with generated text
        """
        payload = {
            "prompt": params.prompt,
            "max_tokens": params.max_tokens,
            "temperature": params.temperature,
            "top_p": params.top_p,
            "stop": params.stop,
            "stream": False
        }
        
        response = self._session.post(
            f"{self.endpoint}/v1/completions",
            json=payload,
            timeout=self.timeout
        )
        response.raise_for_status()
        
        data = response.json()
        choice = data["choices"][0]
        usage = data.get("usage", {})
        
        return GenerationResult(
            text=choice["text"],
            prompt_tokens=usage.get("prompt_tokens", 0),
            completion_tokens=usage.get("completion_tokens", 0),
            stop_reason=choice.get("finish_reason", "")
        )
    
    def chat(self, messages: List[ChatMessage], params: Optional[GenerationParams] = None) -> GenerationResult:
        """
        Generate chat completion using OpenAI-compatible API.
        
        Args:
            messages: List of chat messages
            params: Optional generation parameters
            
        Returns:
            GenerationResult with assistant response
        """
        if params is None:
            params = GenerationParams()
        
        payload = {
            "messages": [{"role": m.role, "content": m.content} for m in messages],
            "max_tokens": params.max_tokens,
            "temperature": params.temperature,
            "top_p": params.top_p,
            "stop": params.stop,
            "stream": False
        }
        
        response = self._session.post(
            f"{self.endpoint}/v1/chat/completions",
            json=payload,
            timeout=self.timeout
        )
        response.raise_for_status()
        
        data = response.json()
        choice = data["choices"][0]
        usage = data.get("usage", {})
        
        return GenerationResult(
            text=choice["message"]["content"],
            prompt_tokens=usage.get("prompt_tokens", 0),
            completion_tokens=usage.get("completion_tokens", 0),
            stop_reason=choice.get("finish_reason", "")
        )
    
    def stream_complete(self, params: GenerationParams) -> Iterator[str]:
        """
        Stream text completion tokens.
        
        Args:
            params: Generation parameters
            
        Yields:
            Generated text tokens
        """
        payload = {
            "prompt": params.prompt,
            "max_tokens": params.max_tokens,
            "temperature": params.temperature,
            "top_p": params.top_p,
            "stop": params.stop,
            "stream": True
        }
        
        response = self._session.post(
            f"{self.endpoint}/v1/completions",
            json=payload,
            stream=True,
            timeout=self.timeout
        )
        response.raise_for_status()
        
        for line in response.iter_lines():
            if line and line.startswith(b"data: "):
                data_str = line.decode()[6:]
                if data_str.strip() == "[DONE]":
                    break
                try:
                    data = json.loads(data_str)
                    if "choices" in data and data["choices"]:
                        text = data["choices"][0].get("text", "")
                        if text:
                            yield text
                except json.JSONDecodeError:
                    continue
    
    def stream_chat(self, messages: List[ChatMessage], params: Optional[GenerationParams] = None) -> Iterator[str]:
        """
        Stream chat completion tokens.
        
        Args:
            messages: List of chat messages
            params: Optional generation parameters
            
        Yields:
            Generated text tokens
        """
        if params is None:
            params = GenerationParams()
        
        payload = {
            "messages": [{"role": m.role, "content": m.content} for m in messages],
            "max_tokens": params.max_tokens,
            "temperature": params.temperature,
            "top_p": params.top_p,
            "stop": params.stop,
            "stream": True
        }
        
        response = self._session.post(
            f"{self.endpoint}/v1/chat/completions",
            json=payload,
            stream=True,
            timeout=self.timeout
        )
        response.raise_for_status()
        
        for line in response.iter_lines():
            if line and line.startswith(b"data: "):
                data_str = line.decode()[6:]
                if data_str.strip() == "[DONE]":
                    break
                try:
                    data = json.loads(data_str)
                    if "choices" in data and data["choices"]:
                        delta = data["choices"][0].get("delta", {})
                        content = delta.get("content", "")
                        if content:
                            yield content
                except json.JSONDecodeError:
                    continue
    
    def generate_kobold(self, prompt: str, max_length: int = 100, **kwargs) -> str:
        """
        Generate text using KoboldAI native API.
        
        Args:
            prompt: Input prompt
            max_length: Maximum tokens to generate
            **kwargs: Additional generation parameters
            
        Returns:
            Generated text
        """
        payload = {
            "prompt": prompt,
            "max_length": max_length,
            "max_context_length": kwargs.get("max_context_length", 4096),
            "temperature": kwargs.get("temperature", 0.7),
            "top_k": kwargs.get("top_k", 40),
            "top_p": kwargs.get("top_p", 0.9),
            "rep_pen": kwargs.get("rep_pen", 1.1),
            "rep_pen_range": kwargs.get("rep_pen_range", 512)
        }
        
        response = self._session.post(
            f"{self.endpoint}/api/v1/generate",
            json=payload,
            timeout=self.timeout
        )
        response.raise_for_status()
        
        return response.json()["results"][0]["text"]


# Example usage
if __name__ == "__main__":
    client = Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)Client()
    
    if client.is_connected():
        print(f"Connected to model: {client.get_model()}")
        
        # Simple completion
        result = client.complete(GenerationParams(
            prompt="The meaning of life is",
            max_tokens=50
        ))
        print(f"Completion: {result.text}")
        
        # Chat
        messages = [
            ChatMessage(role="user", content="Hello, who are you?")
        ]
        result = client.chat(messages)
        print(f"Chat response: {result.text}")
    else:
        print("Not connected to Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) server")
'''

def main():
    parser = argparse.ArgumentParser(description="Generate Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) API client")
    parser.add_argument("--output", "-o", default="bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)_client.py",
                        help="Output file path (default: bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)_client.py)")
    args = parser.parse_args()
    
    with open(args.output, "w") as f:
        f.write(CLIENT_TEMPLATE)
    
    print(f"✅ Generated client at: {args.output}")
    print(f"   Usage: from {args.output.replace('.py', '')} import Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)Client")

if __name__ == "__main__":
    main()
