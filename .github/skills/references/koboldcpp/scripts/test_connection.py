#!/usr/bin/env python3
"""Test connection to a Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) server and verify API availability."""

import sys
import requests
import json

def test_bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)(endpoint: str = "http://localhost:5001") -> dict:
    """
    Test Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) server connection and return status info.
    
    Args:
        endpoint: Base URL of the Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) server
        
    Returns:
        dict with connection status and server info
    """
    results = {
        "endpoint": endpoint,
        "connected": False,
        "model": None,
        "version": None,
        "apis": {},
        "error": None
    }
    
    # Test basic connection
    try:
        response = requests.get(f"{endpoint}/api/v1/model", timeout=5)
        if response.status_code == 200:
            results["connected"] = True
            results["model"] = response.json().get("result")
    except requests.exceptions.ConnectionError:
        results["error"] = f"Cannot connect to {endpoint}"
        return results
    except Exception as e:
        results["error"] = str(e)
        return results
    
    # Get version info
    try:
        response = requests.get(f"{endpoint}/api/extra/version", timeout=5)
        if response.status_code == 200:
            results["version"] = response.json().get("result")
    except:
        pass
    
    # Test API endpoints
    api_tests = {
        "koboldai": "/api/v1/model",
        "openai": "/v1/models",
        "ollama": "/api/tags",
        "sdapi": "/sdapi/v1/sd-models"
    }
    
    for api_name, path in api_tests.items():
        try:
            response = requests.get(f"{endpoint}{path}", timeout=5)
            results["apis"][api_name] = response.status_code == 200
        except:
            results["apis"][api_name] = False
    
    return results

def main():
    endpoint = sys.argv[1] if len(sys.argv) > 1 else "http://localhost:5001"
    
    print(f"Testing Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) at {endpoint}...")
    results = test_bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)(endpoint)
    
    if results["connected"]:
        print(f"✅ Connected successfully")
        print(f"   Model: {results['model']}")
        print(f"   Version: {results['version']}")
        print(f"   Available APIs:")
        for api, available in results["apis"].items():
            status = "✅" if available else "❌"
            print(f"     {status} {api}")
    else:
        print(f"❌ Connection failed: {results['error']}")
        sys.exit(1)
    
    return results

if __name__ == "__main__":
    main()
