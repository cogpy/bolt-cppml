# bolt-cppml Deep Analysis Findings

## Build Status

- **Compilation**: PASSES (0 errors, 3 warnings)
- **Tests**: 83/86 pass (97%), 3 failures
- **Platform**: Linux (GCC 11.4.0), C++20, no GGML (llama.cpp not cloned)

## Critical Bugs

### BUG-1: Uninitialized ScrollPosition members (CRASH)
- **File**: `include/bolt/core/editor_store.hpp:21-32`
- **Issue**: `ScrollPosition::line` and `ScrollPosition::character` are `int` with no default initialization. When `EditorDocument` is constructed and `validateDocument()` is called, `scroll.validate()` reads garbage values, causing intermittent crashes.
- **Impact**: Integration tests `integration_all` and `integration_basic` fail with "Scroll position cannot be negative: line=-716036064"
- **Fix**: Initialize `int line = 0; int character = 0;`

### BUG-2: Operational Transform insert-vs-delete logic error
- **File**: `include/bolt/collaboration/operational_transform.hpp:115-122`
- **Issue**: When transforming an INSERT at pos(0,5) against a DELETE at pos(0,3) of content "abc" (length 3), the `deleteLen = min(3, 5-3) = 2` so result is `5-2=3`. But the test expects `2`. The correct result after deleting chars 3,4,5 ("abc") is that position 5 becomes position 2 (shifted left by 3, the full delete length). The `min()` clamp is wrong — it should be `min(other.getContent().length(), insertPos.character)` when the delete range fully covers positions before the insert.
- **Impact**: `unit_collaboration_tests` aborts with assertion failure.
- **Fix**: Correct the OT logic to use full delete length when delete range ends at or beyond insert position.

### BUG-3: Three compiler warnings
- `syntax_highlighter.hpp:68` — signed/unsigned comparison
- `test_comprehensive_e2e.cpp:1899` — unused variable `lm`
- `test_comprehensive_e2e.cpp:2435` — deprecated volatile compound assignment

## Structural Issues

### STRUCT-1: 72 root-level .cpp files (should be organized)
- 72 `.cpp` files scattered in root directory (demos, tests, standalone tools)
- Should be organized into `examples/`, `demos/`, or properly integrated into `test/`

### STRUCT-2: Compiled binaries tracked in git
- 10+ ELF binaries committed to git (main, simple_ai_demo, test_*, tinyllama_chat)
- Total ~2.4MB of binaries that should be in .gitignore

### STRUCT-3: Export archives tracked in git
- 11 `.tar.gz` files + 1 `.zip` totaling ~3.5MB
- These are Replit export artifacts that should never be in version control

### STRUCT-4: Duplicate LSP client headers
- `include/bolt/editor/lsp_client.hpp` (full implementation)
- `include/bolt/lsp/lsp_client.hpp` (different, simpler version)
- Confusing and potentially conflicting

### STRUCT-5: Header-only files without implementations
- 27 headers in `include/` have no matching `.cpp` in `src/`
- Some are legitimately header-only (interfaces, templates), but many are stubs

### STRUCT-6: Unused RAG/Tool manager headers
- `i_rag_manager.hpp`, `narrative_rag_manager.hpp`, `temporal_rag_manager.hpp`, `narrative_tool_manager.hpp`, `temporal_tool_manager.hpp` — never included from any source file
- Dead code that adds complexity without value

### STRUCT-7: 41 markdown files in root directory
- Many are session reports, implementation notes, and status documents
- Should be consolidated into `docs/` directory

## Skill Misalignments

### MISALIGN-1: Repo SKILL.md vs Agent SKILL.md divergence
- **Repo** `.github/skills/SKILL.md` describes "bolt-cppml" as an IDE skill
- **Agent** `skills/bolt-cpp-ml/SKILL.md` describes the meta-skill with 4 paths
- The repo SKILL.md is outdated and doesn't reflect the bolt-cpp-ml meta-skill

### MISALIGN-2: Repo skill paths don't match agent skill paths
- Repo has skills at `.github/skills/references/` and `.github/skills/templates/`
- Agent skill has them at `references/` and `templates/` (relative to skill root)
- The repo stores a copy but it's not the authoritative source

### MISALIGN-3: bolt-cpp-ml² self-application in repo
- `.github/skills/bolt-cpp-ml-squared/` exists in repo
- This is the fixed-point self-application — needs to be kept in sync

## .gitignore Gaps

- Missing: `*.tar.gz`, `*.zip` (for export artifacts)
- Missing: ELF binaries without extensions (main, simple_ai_demo, etc.)
- Missing: `*.png` for generated icons
- Missing: `attached_assets/` directory
- Missing: `.ccls-cache/` directory
- Missing: `build-coverage/` is listed but `build-shared/` pattern may not catch all

## Improvement Opportunities

1. **Initialize all struct members** — EditorDocument, ScrollPosition, Cursor
2. **Fix OT transform logic** — correct the insert-vs-delete transformation
3. **Clean root directory** — move demos to examples/, remove binaries/archives
4. **Update .gitignore** — add patterns for binaries, archives, caches
5. **Remove dead code** — unused RAG headers, duplicate LSP client
6. **Fix compiler warnings** — all 3 are trivial to fix
7. **Add missing E2E tests** — for KoboldCpp streaming, RAG router, DrawKern
8. **Sync skill definitions** — update repo SKILL.md to match agent skill
9. **Consolidate documentation** — move root .md files to docs/
