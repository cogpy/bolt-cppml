# bolt-cpp-ml: Operator Algebra Reference

This document formalizes the algebraic structure of the `bolt-cpp-ml` skill composition.

## The Composition Expression

```
bolt-cpp-ml = skill-creator(
  function-creator(
    circled-operators[ bolt-new ⊕ koboldcpp ⊕ janext ⊕ cpp-e2e-test-gen ]
  ) → tutorialkit( neuro-nn )
)
```

## Semiring Analysis

The skill operates in the **Skills semiring** `(S, ⊕, ⊗, ∅, id)`:

| Element | Meaning |
|---|---|
| `⊕` | Additive composition — independent alternatives, fork/merge |
| `⊗` | Multiplicative composition — pipeline, interaction, entanglement |
| `∅` (zero) | NoOp skill — does nothing |
| `id` (one) | Identity skill — passes through unchanged |

## Layer 1: The Core Sum (⊕)

```
C = bolt-new ⊕ koboldcpp ⊕ janext ⊕ cpp-e2e-test-gen
```

This is a **direct sum** (coproduct in the category of skills). The four skills are independent; the user chooses one path. This is the **polynomial pattern** — a sum of independent terms.

**Properties:**
- `bolt-new ⊕ koboldcpp ≅ koboldcpp ⊕ bolt-new` (commutativity)
- `(bolt-new ⊕ koboldcpp) ⊕ janext ≅ bolt-new ⊕ (koboldcpp ⊕ janext)` (associativity)
- `C ⊕ ∅ ≅ C` (identity)

## Layer 2: The Functor (function-creator)

```
F = function-creator: Skill → Skill
F(C) = function-creator(bolt-new ⊕ koboldcpp ⊕ janext ⊕ cpp-e2e-test-gen)
```

`function-creator` is a **functor** over the skill space. It preserves the additive structure:

```
F(A ⊕ B) ≅ F(A) ⊕ F(B)   (functor preserves coproducts)
```

This means the four transformed skills remain independent alternatives after transformation.

## Layer 3: The Pipeline (→)

```
F(C) → tutorialkit(neuro-nn)
```

The `→` operator is a **sequential composition** (pipeline). The output of `function-creator` is fed as input to `tutorialkit`, which wraps it in an interactive shell.

In the skills semiring, this is:

```
result = F(C) ⊗ tutorialkit(neuro-nn)
```

This is the **tensorial pattern** — the tutorial and the transformed skills **interact**. The tutorial's structure depends on the skills, and the skills' presentation depends on the tutorial.

## Layer 4: The Tensor Product (tutorialkit ⊗ neuro-nn)

```
T = tutorialkit ⊗ neuro-nn
```

The tutorial framework and the AI persona are **entangled**. The persona modulates the tutorial content, and the tutorial structure constrains the persona's expression. This is a true tensor product — neither component is independent of the other.

## The Full Composition

```
bolt-cpp-ml = skill-creator(F(C) ⊗ T)
            = skill-creator(
                function-creator(bolt-new ⊕ koboldcpp ⊕ janext ⊕ cpp-e2e-test-gen)
                ⊗ (tutorialkit ⊗ neuro-nn)
              )
```

Expanding via distributivity (Law 7: `A ⊗ (B ⊕ C) ≅ (A⊗B) ⊕ (A⊗C)`):

```
bolt-cpp-ml ≅ skill-creator(
    (tutorialkit(neuro-nn) ⊗ F(bolt-new))
  ⊕ (tutorialkit(neuro-nn) ⊗ F(koboldcpp))
  ⊕ (tutorialkit(neuro-nn) ⊗ F(janext))
  ⊕ (tutorialkit(neuro-nn) ⊗ F(cpp-e2e-test-gen))
)
```

This expanded form reveals the true structure: **four independent interactive tutorial experiences**, each combining the neuro-nn persona with one of the four core skills.

## Extension Points

To add a new capability path `X` to `bolt-cpp-ml`:

```
bolt-cpp-ml' = bolt-cpp-ml ⊕ tutorialkit(neuro-nn) ⊗ F(X)
```

The additive structure ensures `X` integrates cleanly without disrupting existing paths.
