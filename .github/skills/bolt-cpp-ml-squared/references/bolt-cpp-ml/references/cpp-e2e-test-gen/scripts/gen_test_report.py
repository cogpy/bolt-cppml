#!/usr/bin/env python3
"""Generate a Markdown test report from test runner output.

Usage:
    python gen_test_report.py <test_output_file> <test_source_file> [--output report.md]

Parses the stdout of a bolt-style test runner (lines like "[Suite] Name ... PASS/FAIL")
and the source file (counting BOLT_TEST/TEST macros) to produce a summary report.
"""
import argparse
import re
import sys
from collections import Counter
from pathlib import Path


def parse_test_output(output_text: str) -> dict:
    """Parse test runner output lines."""
    results = []
    pattern = re.compile(r'^\[(\w+)\]\s+(\w+)\s+\.\.\.\s*(PASS|FAIL|ERROR)', re.MULTILINE)
    for m in pattern.finditer(output_text):
        results.append({
            'suite': m.group(1),
            'name': m.group(2),
            'status': m.group(3)
        })
    return results


def count_tests_in_source(source_text: str) -> dict:
    """Count test macros per suite in source file."""
    pattern = re.compile(r'(?:BOLT_TEST|TEST|TEST_F|TEST_CASE)\s*\(\s*(\w+)\s*,\s*(\w+)')
    suites = Counter()
    for m in pattern.finditer(source_text):
        suites[m.group(1)] += 1
    return dict(suites)


def generate_report(results: list, suite_counts: dict, source_lines: int) -> str:
    """Generate Markdown report."""
    passed = sum(1 for r in results if r['status'] == 'PASS')
    failed = sum(1 for r in results if r['status'] == 'FAIL')
    errors = sum(1 for r in results if r['status'] == 'ERROR')
    total = len(results)

    lines = [
        "# E2E Unit Test Report\n",
        "## Summary\n",
        "| Metric | Value |",
        "|---|---|",
        f"| **Total Tests** | {total} |",
        f"| **Passed** | {passed} ({passed*100//max(total,1)}%) |",
        f"| **Failed** | {failed} |",
        f"| **Errors** | {errors} |",
        f"| **Source Lines** | {source_lines:,} |",
        f"| **Test Suites** | {len(suite_counts)} |",
        "",
        "## Breakdown by Suite\n",
        "| Suite | Tests | Status |",
        "|---|---|---|",
    ]

    suite_results = Counter()
    suite_pass = Counter()
    for r in results:
        suite_results[r['suite']] += 1
        if r['status'] == 'PASS':
            suite_pass[r['suite']] += 1

    for suite in sorted(suite_results.keys()):
        count = suite_results[suite]
        p = suite_pass[suite]
        status = "All Pass" if p == count else f"{p}/{count} Pass"
        lines.append(f"| {suite} | {count} | {status} |")

    if failed > 0 or errors > 0:
        lines.extend(["", "## Failures\n"])
        for r in results:
            if r['status'] in ('FAIL', 'ERROR'):
                lines.append(f"- **[{r['suite']}] {r['name']}** — {r['status']}")

    lines.extend(["", "---", f"*Generated from {total} test results*"])
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description='Generate test report')
    parser.add_argument('test_output', help='Path to test runner output file')
    parser.add_argument('test_source', help='Path to test source .cpp file')
    parser.add_argument('--output', '-o', default=None, help='Output report path')
    args = parser.parse_args()

    output_text = Path(args.test_output).read_text(errors='replace')
    source_text = Path(args.test_source).read_text(errors='replace')
    source_lines = source_text.count('\n')

    results = parse_test_output(output_text)
    suite_counts = count_tests_in_source(source_text)
    report = generate_report(results, suite_counts, source_lines)

    if args.output:
        Path(args.output).write_text(report)
        print(f"Report written to {args.output}", file=sys.stderr)
    else:
        print(report)


if __name__ == '__main__':
    main()
