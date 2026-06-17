# External review 2026-06-17 after TODO-020B

## Source

The full web review response is kept locally under:

```text
external_reviews/TS_RS_external_review_after_TODO020B_response_20260617.md
```

`external_reviews/` is ignored by git, so this tracked note records the actionable project decisions.

## Verdict

```text
Conditional Go for TODO-022 only.
No-Go for jumping to TODO-021 Viewer selection or claiming P0A / Detail compatibility.
```

The reviewer confirmed that the main direction is correct:

```text
STEP-only standalone app
Qt6 for UI / commands
OCCT for STEP / AIS / selection / geometry
RebarSmart evidence for rebar generation logic
VisualTS / old Tushi only for Detail package and drawing compatibility evidence
```

## Must-fix before TODO-022

1. Add a real Detail fixture or fixture manifest.
2. Update README and todo state to remove stale truth sources.
3. Mark TODO-020B as a registry-library P0, not an integrated selection-ready state.
4. Tighten TODO-022 acceptance to read-only old package statistics plus raw XML preservation.

## Follow-up risks registered

1. Application still transports `TopoDS_Shape` through `StepDisplayModel` for the STEP display POC.
2. `ShapeStore` and `TopologyBindingRegistry` are not yet integrated into the import / display session chain.
3. `TopologyBindingRegistry::restore()` does not use fallback fields yet.
4. Edge fingerprint endpoint order is not canonicalized yet.
5. Real viewer smoke is weaker than a full visual display check.
6. STEP unit / scale policy is not in `StepImportResult` yet.
7. CommandService boundary is only implemented for STEP import so far.

## Decision

Proceed with a small review-response patch first. Do not implement `TODO-022` until this patch is committed and tagged.

## Local verification for this response patch

```text
todo.csv parsed as 38 rows including header.
expected field count is 12.
status=next count is 1.
only next item is TODO-022.
CSV bad row count is 0.
```
