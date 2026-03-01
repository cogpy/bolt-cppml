#!/usr/bin/env python3
"""Extract public function/method signatures from C++ header files.

Usage:
    python extract_signatures.py <repo_root> [--exclude-dirs vcpkg,build,.git]

Output: JSON catalog of all public functions grouped by header file,
        printed to stdout. Redirect to a file for later use.
"""
import argparse
import json
import os
import re
import sys
from pathlib import Path

# Regex patterns for C++ declarations
CLASS_RE = re.compile(r'(?:class|struct)\s+(\w+)')
METHOD_RE = re.compile(
    r'^\s*(?:virtual\s+|static\s+|inline\s+|explicit\s+|constexpr\s+)*'
    r'([\w:<>&*\s,]+?)\s+'
    r'(\w+)\s*\(([^)]*)\)\s*'
    r'(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:=\s*0\s*)?[;{]',
    re.MULTILINE
)
ENUM_RE = re.compile(r'enum\s+(?:class\s+)?(\w+)')

def extract_from_header(filepath: str) -> dict:
    """Extract classes, methods, enums from a single header file."""
    try:
        with open(filepath, 'r', errors='replace') as f:
            content = f.read()
    except Exception:
        return {}

    result = {
        'file': filepath,
        'classes': [],
        'free_functions': [],
        'enums': []
    }

    # Extract class/struct names
    for m in CLASS_RE.finditer(content):
        result['classes'].append(m.group(1))

    # Extract enum names
    for m in ENUM_RE.finditer(content):
        result['enums'].append(m.group(1))

    # Extract method signatures
    for m in METHOD_RE.finditer(content):
        ret_type = m.group(1).strip()
        name = m.group(2).strip()
        params = m.group(3).strip()
        # Skip destructors, constructors that look like class names, operators
        if name.startswith('~') or name == 'operator':
            continue
        # Skip common non-function patterns
        if ret_type in ('if', 'while', 'for', 'switch', 'return', 'delete', 'new'):
            continue
        result['free_functions'].append({
            'return_type': ret_type,
            'name': name,
            'params': params
        })

    return result


def scan_repo(root: str, exclude_dirs: list) -> list:
    """Scan repository for all .hpp/.h header files."""
    catalog = []
    root_path = Path(root)

    for dirpath, dirnames, filenames in os.walk(root_path):
        # Prune excluded directories
        dirnames[:] = [d for d in dirnames if d not in exclude_dirs]

        for fname in filenames:
            if fname.endswith(('.hpp', '.h', '.hxx')):
                fpath = os.path.join(dirpath, fname)
                info = extract_from_header(fpath)
                if info and (info['classes'] or info['free_functions'] or info['enums']):
                    # Make path relative to repo root
                    info['file'] = os.path.relpath(fpath, root)
                    catalog.append(info)

    return catalog


def main():
    parser = argparse.ArgumentParser(description='Extract C++ function signatures')
    parser.add_argument('repo_root', help='Path to repository root')
    parser.add_argument('--exclude-dirs', default='vcpkg,build,.git,third_party,external,.ccls-cache,node_modules,ggml',
                        help='Comma-separated directories to exclude')
    args = parser.parse_args()

    exclude = [d.strip() for d in args.exclude_dirs.split(',')]
    catalog = scan_repo(args.repo_root, exclude)

    # Summary
    total_classes = sum(len(h['classes']) for h in catalog)
    total_funcs = sum(len(h['free_functions']) for h in catalog)
    total_enums = sum(len(h['enums']) for h in catalog)

    output = {
        'summary': {
            'headers_scanned': len(catalog),
            'total_classes': total_classes,
            'total_functions': total_funcs,
            'total_enums': total_enums
        },
        'headers': catalog
    }

    json.dump(output, sys.stdout, indent=2)
    print()  # trailing newline

    # Print summary to stderr
    print(f"\nScanned {len(catalog)} headers: "
          f"{total_classes} classes, {total_funcs} functions, {total_enums} enums",
          file=sys.stderr)


if __name__ == '__main__':
    main()
