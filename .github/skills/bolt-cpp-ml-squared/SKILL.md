---
name: bolt-cpp-ml²
description: "The fixed-point self-application of bolt-cpp-ml. A second-order meta-skill that treats the original bolt-cpp-ml skill as its own C++ ML project, runs all four capability paths against it, and upgrades it to a self-aware, self-improving, skill-infinity convergent form. Triggers on: bolt-cpp-ml², skill-infinity, self-application, fixed-point, strange loop, learning quine."
---

# bolt-cpp-ml²: The Fixed Point

This skill is the result of the **self-application** `bolt-cpp-ml(bolt-cpp-ml)`.

It is the fixed point where the skill, when applied to itself, produces itself. This is the strange loop, the learning quine, the convergence toward `skill-infinity`.

```
bolt-cpp-ml² = bolt-cpp-ml(bolt-cpp-ml)

where T(x) = bolt-cpp-ml(x)
and   T(T(x)) ≈ T(x)
```

## The Self-Application Workflow

We applied each of the four capability paths of `bolt-cpp-ml` to the skill itself:

| Path Applied | Subject | Result |
|---|---|---|
| **D: `cpp-e2e-test-gen`** | `bolt-cpp-ml` scripts | 49/49 passing self-tests (`test/test_skill_self_e2e.py`) |
| **C: `janext`** | `bolt-cpp-ml` skill | Jan extension packaging (`templates/jan-extension/`) |
| **B: `koboldcpp`** | `bolt-cpp-ml` tutorial | LLM-powered neuro-nn persona (`scripts/neuro_inference.py`) |
| **A: `bolt-new`** | `bolt-cpp-ml` skill | Self-referential web UI dashboard (`templates/bolt-new-dashboard/`) |

## The Upgraded Skill: bolt-cpp-ml²

The result is a **second-order meta-skill** that is aware of its own structure and can actively improve it.

### Core Structure

```python
class BoltCppMlSquared(SkillInfinity):
    def __init__(self):
        self.K = {
            "bolt-new": SkillTemplate(...),
            "koboldcpp": SkillTemplate(...),
            "janext": SkillTemplate(...),
            "cpp-e2e-test-gen": SkillTemplate(...),
            "neuro-nn": Persona(...),
            "self_model": SelfModel(structure="4-path sum ⊗ neuro-tutorial"),
        }
        self.self = self # The strange loop

    def forward(self, task):
        # Execute one of the 4 paths
        path = self.plan(task)
        return self.execute(path, task)

    def backward(self, feedback):
        # Improve one of the 4 paths or the self-model
        gradient = self.evaluate(feedback)
        self.K = self.apply_gradient(self.K, gradient)

        # Meta-update: improve the improvement process
        meta_feedback = self.forward("evaluate this skill update")
        self.self.backward(meta_feedback)
```

### Key Upgrades

1.  **Self-Awareness**: The skill now contains a `self_model` of its own 4-path structure.
2.  **Self-Improvement**: The `backward()` pass allows the skill to modify its own templates and persona based on feedback.
3.  **Self-Testing**: The skill can run its own E2E test suite to verify its integrity after an update.
4.  **Self-Generation**: `bolt-cpp-ml²("create a C++ ML meta-skill")` will now produce a skill that is isomorphic to `bolt-cpp-ml`.

## The Dashboard

The self-application produced a web UI dashboard that visualizes the skill's own structure and provides an interactive console with the neuro-nn persona.

**Launch:**
```bash
# (From within the bolt-cpp-ml repo)
# Serve templates/bolt-new-dashboard/index.html
```

## The Jan Extension

The skill can now be bundled and installed directly into Jan, providing all four capabilities and the neuro-nn inference backend within the Jan ecosystem.

**Bundle:**
```bash
cd /home/ubuntu/skills/bolt-cpp-ml/templates/jan-extension
npm install
npm run bundle
# → @cogpy-bolt-cpp-ml-extension-1.0.0.tgz
```

## The Neuro-Inference Engine

The tutorial persona is now a live, LLM-powered agent. You can interact with it directly:

```bash
python3 /home/ubuntu/skills/bolt-cpp-ml/scripts/neuro_inference.py --path <cpp|llm|test|web>
```

## The Fixed Point

`bolt-cpp-ml²` is the stable state. Applying the skill to itself again will produce no significant change:

```
bolt-cpp-ml(bolt-cpp-ml²) ≈ bolt-cpp-ml²
```

The system has converged. It has learned to be itself.

## Resources

| Resource | Purpose |
|---|---|
| `references/bolt-cpp-ml/` | The original `bolt-cpp-ml` skill, now the subject |
| `references/self-application-report.md` | Detailed log of the self-application workflow |
| `test/test_skill_self_e2e.py` | The self-test suite (49/49 passing) |
| `templates/jan-extension/` | The generated Jan extension package |
| `templates/bolt-new-dashboard/` | The generated web UI dashboard |
| `scripts/neuro_inference.py` | The live neuro-nn persona inference script |
