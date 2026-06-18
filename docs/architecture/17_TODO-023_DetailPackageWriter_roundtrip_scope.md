# TODO-023 DetailPackageWriter round-trip scope

## Decision

TODO-023 is a preserve-mode round-trip node only.

The only approved P0 chain is:

```text
DetailPackageReader
  -> DetailPackageSnapshot
  -> DetailPackageWriter
  -> output directory
  -> DetailPackageReader
  -> comparison
```

## Allowed in TODO-023

```text
Write Detail.xml and DetailNN.stl from existing DetailFileSnapshot.rawXml.
Preserve original fileName.
Preserve sheetIndex order.
Write to a caller-provided output directory.
Validate written XML by reading it again.
Compare key knownSummary counts before / after.
Compare rawAttributes and unknownChildren counts before / after.
Report write / validation diagnostics.
```

## Explicitly forbidden in TODO-023

```text
No minimal Detail generation.
No CAD plugin autoin validation.
No DrawingModel.
No RebarModel.
No Rebar generator connection.
No Viewer / selection connection.
No structured rewrite of unknown XML subtrees.
No claim that old Tushi CAD plugin compatibility is complete.
```

## Preservation contract

P0 Writer must use raw passthrough:

```text
unchanged input file -> exact rawXml bytes written as output file contents
```

`rawAttributes` and `unknownChildren` in TODO-023 are diagnostics and guardrail signals.
They are not yet a complete structural preservation model.

Hard rule after the TODO-023 / TODO-024 external review:

```text
preserve-mode guarantee currently comes from rawXml passthrough only.
rawAttributes/unknownChildren are diagnostics, not structural preservation.
rawAttributes/unknownChildren are not mutation-ready preservation data.
```

If a future node needs structured XML mutation, it must first add a stronger XML model:

```text
raw subtree preservation
unknown text nodes
namespace declarations
attribute ordering policy
encoding policy
strict validation mode
```

## Fixture gate

TODO-023 tests must include:

```text
synthetic round-trip fixture
todo66 local fixture probe, reported as passed or explicit CTest skipped
unknown node / attribute preservation fixture
bad XML validation fixture
```

If the local todo66 fixture is missing, the probe must not count as a normal pass.

## Non-goal

TODO-023 does not prove Detail compatibility.

Compatibility is not claimed until:

```text
TODO-024 creates an extreme minimal Detail package.
The user runs old AutoCAD plugin autoin.
The import result is recorded with files / screenshots / logs.
```
