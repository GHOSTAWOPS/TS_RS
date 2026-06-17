# External review 2026-06-17 for TODO-022 DetailPackageReader P0

## Source

Review request was sent to Web ChatGPT Pro with:

```text
Package: TS_RS_TODO-022_external_review_20260617.zip
Conversation: https://chatgpt.com/c/6a28c1d7-c50c-83ec-8bf4-29c4df9ea152
Commit under review: 29f00f2
Tag under review: tsrs-todo-022-detail-reader-p0-20260617
```

## Verdict

```text
Conditional Go for TODO-023.
```

The reviewer confirmed that TODO-022 kept the Reader-only boundary:

```text
No Writer.
No Exporter.
No DrawingModel / RebarModel mapping.
No Viewer or generator connection.
No OCCT / AIS leakage into detail DTO.
No commercial runtime dependency.
```

The reviewer did not accept an unconditional move to TODO-023 because the Reader still had
weak guardrails for real fixture reproducibility, raw preservation DTOs, and parse diagnostics.

## Must-fix before TODO-023

1. Real todo66 fixture verification must not silently pass when the fixture is missing.
   The test must either use the local fixture or report an explicit CTest skip.
2. TODO-023 scope must be frozen as preserve-mode raw passthrough only:

   ```text
   Reader -> Writer -> Reader
   unchanged file = rawXml passthrough
   ```

3. TODO-023 must not implement:

   ```text
   minimal generate
   CAD autoin
   DrawingModel mapping
   RebarModel mapping
   Viewer / generator wiring
   ```

## Accepted implementation actions

The review comments are technically valid for this codebase and were accepted:

```text
rawAttributes path-only is too weak for round-trip observation.
unknownChildren path-only is too weak for round-trip observation.
parse diagnostics need line / column.
todo66 fixture missing must not be counted as a normal pass.
knownSummary should separate StbGroups container and StbGroup entries.
sheet name parsing should not incorrectly parse Detail100.stl as sheet 10.
```

## Follow-up risks

These are not blocking TODO-023 preserve-mode passthrough, but remain registered:

```text
Reader P0 still does not preserve raw subtree XML per unknown node.
Reader P0 does not promise byte-identical XML normalization if a later writer rewrites structure.
Non-UTF-8 legacy Detail XML remains a compatibility risk to verify on more real samples.
Root mismatch currently remains warning-level for inspect mode; Writer tests need strict validation.
Review package generation should avoid control-character escape mistakes in README text.
```

## Decision

Proceed only after a small TODO-022 hardening patch:

```text
1. Strengthen Reader DTO / diagnostics.
2. Split local todo66 fixture probe into an explicit CTest-skippable test.
3. Add TODO-023 round-trip scope document.
4. Re-run targeted Detail tests and default CTest.
```

