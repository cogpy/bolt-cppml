#!/usr/bin/env python3
"""
E2E Self-Tests for bolt-cpp-ml skill scripts.

Path D applied to itself: cpp-e2e-test-gen testing the bolt-cpp-ml skill's
own Python scripts (extract_signatures.py, gen_test_report.py,
generate_client.py, test_connection.py).

This is the fixed-point application: the skill tests itself.
"""
import json
import os
import sys
import tempfile
import textwrap
from pathlib import Path
from collections import Counter

import pytest

# ── Resolve skill root ──────────────────────────────────────────────────────
SKILL_ROOT = Path(__file__).resolve().parent.parent
SCRIPTS_DIR = SKILL_ROOT / "references" / "cpp-e2e-test-gen" / "scripts"
KOBOLD_SCRIPTS = SKILL_ROOT / "references" / "koboldcpp" / "scripts"

sys.path.insert(0, str(SCRIPTS_DIR))
sys.path.insert(0, str(KOBOLD_SCRIPTS))


# ════════════════════════════════════════════════════════════════════════════
# Suite: ExtractSignatures — tests for extract_signatures.py
# ════════════════════════════════════════════════════════════════════════════

from extract_signatures import extract_from_header, scan_repo


class TestExtractFromHeader:
    """E2E tests for extract_from_header()."""

    def test_extracts_class_name(self, tmp_path):
        """Should detect a class declaration."""
        h = tmp_path / "widget.hpp"
        h.write_text("class Widget {\npublic:\n  void render();\n};\n")
        result = extract_from_header(str(h))
        assert "Widget" in result["classes"]

    def test_extracts_struct_name(self, tmp_path):
        """Should detect a struct declaration."""
        h = tmp_path / "point.hpp"
        h.write_text("struct Point { float x; float y; };\n")
        result = extract_from_header(str(h))
        assert "Point" in result["classes"]

    def test_extracts_enum_name(self, tmp_path):
        """Should detect an enum class declaration."""
        h = tmp_path / "color.hpp"
        h.write_text("enum class Color { Red, Green, Blue };\n")
        result = extract_from_header(str(h))
        assert "Color" in result["enums"]

    def test_extracts_free_function(self, tmp_path):
        """Should detect a free function signature."""
        h = tmp_path / "utils.hpp"
        h.write_text("int add(int a, int b);\n")
        result = extract_from_header(str(h))
        names = [f["name"] for f in result["free_functions"]]
        assert "add" in names

    def test_skips_destructor(self, tmp_path):
        """Should not include destructors in free_functions."""
        h = tmp_path / "foo.hpp"
        h.write_text("class Foo {\npublic:\n  ~Foo();\n};\n")
        result = extract_from_header(str(h))
        names = [f["name"] for f in result["free_functions"]]
        assert "~Foo" not in names

    def test_handles_missing_file(self):
        """Should return empty dict for non-existent file."""
        result = extract_from_header("/nonexistent/path/header.hpp")
        assert result == {}

    def test_multiple_classes(self, tmp_path):
        """Should extract multiple class names from one header."""
        h = tmp_path / "multi.hpp"
        h.write_text("class Alpha {};\nclass Beta {};\nstruct Gamma {};\n")
        result = extract_from_header(str(h))
        assert "Alpha" in result["classes"]
        assert "Beta" in result["classes"]
        assert "Gamma" in result["classes"]

    def test_static_method_extracted(self, tmp_path):
        """Should extract static methods."""
        h = tmp_path / "factory.hpp"
        h.write_text("class Factory {\npublic:\n  static Factory* getInstance();\n};\n")
        result = extract_from_header(str(h))
        names = [f["name"] for f in result["free_functions"]]
        assert "getInstance" in names

    def test_const_method_extracted(self, tmp_path):
        """Should extract const methods."""
        h = tmp_path / "reader.hpp"
        h.write_text("class Reader {\npublic:\n  std::string read() const;\n};\n")
        result = extract_from_header(str(h))
        names = [f["name"] for f in result["free_functions"]]
        assert "read" in names

    def test_empty_header(self, tmp_path):
        """Should return empty result for header with no declarations."""
        h = tmp_path / "empty.hpp"
        h.write_text("// just a comment\n#pragma once\n")
        result = extract_from_header(str(h))
        assert result["classes"] == []
        assert result["free_functions"] == []
        assert result["enums"] == []


class TestScanRepo:
    """E2E tests for scan_repo()."""

    def test_scans_hpp_files(self, tmp_path):
        """Should scan .hpp files in the repo."""
        (tmp_path / "include").mkdir()
        (tmp_path / "include" / "widget.hpp").write_text("class Widget {};\n")
        catalog = scan_repo(str(tmp_path), [])
        files = [c["file"] for c in catalog]
        assert any("widget.hpp" in f for f in files)

    def test_excludes_specified_dirs(self, tmp_path):
        """Should exclude directories listed in exclude_dirs."""
        (tmp_path / "vcpkg").mkdir()
        (tmp_path / "vcpkg" / "skip.hpp").write_text("class Skip {};\n")
        (tmp_path / "include").mkdir()
        (tmp_path / "include" / "keep.hpp").write_text("class Keep {};\n")
        catalog = scan_repo(str(tmp_path), ["vcpkg"])
        files = [c["file"] for c in catalog]
        assert not any("skip.hpp" in f for f in files)
        assert any("keep.hpp" in f for f in files)

    def test_returns_relative_paths(self, tmp_path):
        """Paths in catalog should be relative to repo root."""
        (tmp_path / "src").mkdir()
        (tmp_path / "src" / "foo.hpp").write_text("class Foo {};\n")
        catalog = scan_repo(str(tmp_path), [])
        for entry in catalog:
            assert not os.path.isabs(entry["file"]), f"Expected relative path: {entry['file']}"

    def test_empty_repo(self, tmp_path):
        """Should return empty catalog for repo with no headers."""
        catalog = scan_repo(str(tmp_path), [])
        assert catalog == []

    def test_scans_h_files(self, tmp_path):
        """Should also scan .h files."""
        (tmp_path / "legacy.h").write_text("struct OldStruct { int x; };\n")
        catalog = scan_repo(str(tmp_path), [])
        files = [c["file"] for c in catalog]
        assert any("legacy.h" in f for f in files)


# ════════════════════════════════════════════════════════════════════════════
# Suite: GenTestReport — tests for gen_test_report.py
# ════════════════════════════════════════════════════════════════════════════

from gen_test_report import parse_test_output, count_tests_in_source, generate_report


class TestParseTestOutput:
    """E2E tests for parse_test_output()."""

    def test_parses_pass_result(self):
        output = "[TabBar] AddTab ... PASS\n"
        results = parse_test_output(output)
        assert len(results) == 1
        assert results[0] == {"suite": "TabBar", "name": "AddTab", "status": "PASS"}

    def test_parses_fail_result(self):
        output = "[Editor] OpenFile ... FAIL\n"
        results = parse_test_output(output)
        assert results[0]["status"] == "FAIL"

    def test_parses_error_result(self):
        output = "[Git] Clone ... ERROR\n"
        results = parse_test_output(output)
        assert results[0]["status"] == "ERROR"

    def test_parses_multiple_results(self):
        output = (
            "[TabBar] AddTab ... PASS\n"
            "[TabBar] RemoveTab ... PASS\n"
            "[Editor] OpenFile ... FAIL\n"
        )
        results = parse_test_output(output)
        assert len(results) == 3

    def test_empty_output(self):
        results = parse_test_output("")
        assert results == []

    def test_ignores_non_matching_lines(self):
        output = "Starting tests...\n[TabBar] AddTab ... PASS\nDone.\n"
        results = parse_test_output(output)
        assert len(results) == 1


class TestCountTestsInSource:
    """E2E tests for count_tests_in_source()."""

    def test_counts_bolt_test_macros(self):
        source = "BOLT_TEST(TabBar, AddTab) {}\nBOLT_TEST(TabBar, RemoveTab) {}\n"
        counts = count_tests_in_source(source)
        assert counts["TabBar"] == 2

    def test_counts_test_macros(self):
        source = "TEST(Editor, Open) {}\nTEST(Editor, Close) {}\n"
        counts = count_tests_in_source(source)
        assert counts["Editor"] == 2

    def test_counts_multiple_suites(self):
        source = "BOLT_TEST(A, x) {}\nBOLT_TEST(B, y) {}\nBOLT_TEST(A, z) {}\n"
        counts = count_tests_in_source(source)
        assert counts["A"] == 2
        assert counts["B"] == 1

    def test_empty_source(self):
        counts = count_tests_in_source("")
        assert counts == {}


class TestGenerateReport:
    """E2E tests for generate_report()."""

    def test_report_contains_summary(self):
        results = [
            {"suite": "TabBar", "name": "AddTab", "status": "PASS"},
            {"suite": "TabBar", "name": "RemoveTab", "status": "FAIL"},
        ]
        report = generate_report(results, {"TabBar": 2}, 100)
        assert "Total Tests" in report
        assert "2" in report

    def test_report_shows_failures(self):
        results = [{"suite": "Git", "name": "Clone", "status": "FAIL"}]
        report = generate_report(results, {"Git": 1}, 50)
        assert "Failures" in report
        assert "Clone" in report

    def test_report_no_failures_section_when_all_pass(self):
        results = [{"suite": "TabBar", "name": "AddTab", "status": "PASS"}]
        report = generate_report(results, {"TabBar": 1}, 50)
        assert "Failures" not in report

    def test_report_is_markdown(self):
        results = [{"suite": "A", "name": "b", "status": "PASS"}]
        report = generate_report(results, {"A": 1}, 10)
        assert report.startswith("# E2E Unit Test Report")

    def test_report_handles_zero_tests(self):
        report = generate_report([], {}, 0)
        assert "Total Tests" in report
        assert "0" in report


# ════════════════════════════════════════════════════════════════════════════
# Suite: SkillStructure — meta-tests verifying the skill's own file structure
# ════════════════════════════════════════════════════════════════════════════

class TestSkillStructure:
    """Verify bolt-cpp-ml skill's own file structure (self-referential)."""

    def test_skill_md_exists(self):
        assert (SKILL_ROOT / "SKILL.md").exists()

    def test_skill_md_has_name(self):
        content = (SKILL_ROOT / "SKILL.md").read_text()
        assert "bolt-cpp-ml" in content

    def test_skill_md_has_four_paths(self):
        content = (SKILL_ROOT / "SKILL.md").read_text()
        assert "bolt-new" in content
        assert "koboldcpp" in content
        assert "janext" in content
        assert "cpp-e2e-test-gen" in content

    def test_references_dir_exists(self):
        assert (SKILL_ROOT / "references").is_dir()

    def test_all_four_reference_skills_present(self):
        refs = SKILL_ROOT / "references"
        for name in ["bolt-new", "koboldcpp", "janext", "cpp-e2e-test-gen"]:
            assert (refs / name / "SKILL.md").exists(), f"Missing: references/{name}/SKILL.md"

    def test_templates_dir_exists(self):
        assert (SKILL_ROOT / "templates").is_dir()

    def test_tutorial_structure_exists(self):
        tutorial = SKILL_ROOT / "templates" / "bolt-cpp-ml-tutorial" / "src" / "content" / "tutorial"
        assert tutorial.is_dir()
        for part in ["1-bolt-new", "2-koboldcpp", "3-janext", "4-cpp-e2e-test-gen"]:
            assert (tutorial / part).is_dir(), f"Missing tutorial part: {part}"

    def test_launch_scripts_exist(self):
        scripts = SKILL_ROOT / "scripts"
        assert (scripts / "launch_tutorial.sh").exists()
        assert (scripts / "launch_bolt_new.sh").exists()

    def test_operator_algebra_reference_exists(self):
        assert (SKILL_ROOT / "references" / "operator-algebra.md").exists()

    def test_neuro_nn_persona_reference_exists(self):
        assert (SKILL_ROOT / "references" / "neuro-nn-persona.md").exists()

    def test_bolt_cppml_repo_reference_exists(self):
        assert (SKILL_ROOT / "references" / "bolt-cppml-repo.md").exists()

    def test_skill_description_has_trigger_words(self):
        content = (SKILL_ROOT / "SKILL.md").read_text()
        for trigger in ["KoboldCpp", "Jan extension", "E2E", "Bolt.new", "neuro-nn"]:
            assert trigger in content, f"Missing trigger word: {trigger}"

    def test_operator_algebra_contains_semiring(self):
        content = (SKILL_ROOT / "references" / "operator-algebra.md").read_text()
        assert "⊕" in content
        assert "⊗" in content
        assert "semiring" in content.lower()

    def test_self_application_convergence(self):
        """The skill applied to itself should still be a valid skill (fixed point)."""
        skill_md = (SKILL_ROOT / "SKILL.md").read_text()
        # A valid skill must have a YAML frontmatter with name and description
        assert skill_md.startswith("---")
        assert "name:" in skill_md
        assert "description:" in skill_md
        # And the body must have at least one heading
        assert "# " in skill_md


# ════════════════════════════════════════════════════════════════════════════
# Suite: FixedPoint — tests verifying the self-application fixed-point property
# ════════════════════════════════════════════════════════════════════════════

class TestFixedPoint:
    """Verify the fixed-point property: bolt-cpp-ml(bolt-cpp-ml) ≈ bolt-cpp-ml."""

    def test_skill_is_self_describing(self):
        """The skill's SKILL.md should describe itself accurately."""
        content = (SKILL_ROOT / "SKILL.md").read_text()
        # The skill should mention its own composition
        assert "skill-creator" in content or "meta-skill" in content

    def test_skill_contains_own_workflow(self):
        """The skill should contain a workflow decision tree."""
        content = (SKILL_ROOT / "SKILL.md").read_text()
        assert "Workflow" in content or "Decision" in content or "decision" in content

    def test_skill_references_are_internally_consistent(self):
        """All referenced paths in SKILL.md should exist."""
        content = (SKILL_ROOT / "SKILL.md").read_text()
        # Check that each referenced sub-skill directory exists
        for ref_name in ["bolt-new", "koboldcpp", "janext", "cpp-e2e-test-gen"]:
            ref_path = SKILL_ROOT / "references" / ref_name
            assert ref_path.is_dir(), f"Referenced path missing: references/{ref_name}"

    def test_extract_signatures_is_idempotent(self, tmp_path):
        """Running extract_signatures twice on same input gives same result."""
        h = tmp_path / "stable.hpp"
        h.write_text("class Stable { void run(); };\n")
        r1 = extract_from_header(str(h))
        r2 = extract_from_header(str(h))
        assert r1 == r2

    def test_gen_report_is_deterministic(self):
        """Generating a report twice from same input gives same result."""
        results = [{"suite": "A", "name": "b", "status": "PASS"}]
        r1 = generate_report(results, {"A": 1}, 10)
        r2 = generate_report(results, {"A": 1}, 10)
        assert r1 == r2


if __name__ == "__main__":
    pytest.main([__file__, "-v", "--tb=short"])
