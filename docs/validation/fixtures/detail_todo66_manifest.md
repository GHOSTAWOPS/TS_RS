# Detail fixture manifest - todo66

## Purpose

This manifest makes the real old Tushi Detail evidence reproducible enough for `TODO-022 DetailPackageReader P0` without committing large or private project files into the repo.

`TODO-022` may use this manifest as the mandatory real-evidence gate. If the local files are unavailable or hashes differ, the Reader node must report the fixture as unavailable instead of silently replacing it with a synthetic XML sample.

## Source

```text
C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\docs\phase1\todo66
```

Observed from user-run old Tushi output. User reported `Detail.xml` remains `<StyleRoot/>` across multiple machines and does not appear to update during drawing generation / CAD import.

## Git policy

```text
Tracked in git:
  this manifest and schema observations.

Not tracked by default:
  source .sfl
  screenshots
  full local export directory
  generated zip archives
```

If a later node needs repo-local fixtures, create a separate sanitized fixture set under `samples/fixtures/detail/` and document the sanitization.

## File manifest

| file | bytes | sha256 | lastWrite | XML root | XML node count |
| --- | ---: | --- | --- | --- | ---: |
| `Detail.xml` | 14 | `ccbd220d75d7f9c7e26e2540d639fa5956a369a31d7902f75ed36461f778f271` | 2026-06-09 10:10:04 | `StyleRoot` | 1 |
| `Detail01.stl` | 17085 | `444be32ed907c0393104f415639ced2fb698f21c93df7f27f9ed830a70e40be6` | 2026-06-09 10:09:37 | `DrawingRoot` | 138 |
| `Detail02.stl` | 6604 | `478d166a5dce69a3bd8042e0118443780c8c7fbbdb93e7ac910eeb2ccb4e32f6` | 2026-06-09 10:09:38 | `DrawingRoot` | 67 |
| `Detail03.stl` | 8179 | `2531d11a15184909cc9fb3aa8cc96866f765e8254e443422854c176453a8fb43` | 2026-06-09 10:09:39 | `DrawingRoot` | 73 |
| `Detail04.stl` | 9717 | `03617911fbfcc2d53bcc310f515f8c375b3c6b78640c638cb2591da1494d4d0e` | 2026-06-09 10:09:40 | `DrawingRoot` | 81 |
| `下料表.xls` | 28160 | `f86041d592f350782020d1d7d0047fddb01ada25c7ca8914ad75899469ce365b` | 2026-06-09 09:54:37 | n/a | n/a |
| `消力池下游侧带齿槽底板结构图石钢筋模型.sfl` | 564135 | `6706f10836ca43d1d3275acd09b373cac5af08c0d7a66b1c7025d30006c4b007` | 2026-06-05 10:56:14 | n/a | n/a |
| `PixPin_2026-06-09_09-49-51.png` | 366437 | `180f7fe81fac351e76135ffca6c39e6e1a1810dc52284ed3b407038525ddd895` | 2026-06-09 09:49:51 | n/a | n/a |
| `PixPin_2026-06-09_09-50-13.png` | 374008 | `e177e48d954eae200b62863de7d3289fa1e8bf94b7614c9ec9d902a4c6e31da7` | 2026-06-09 09:50:13 | n/a | n/a |
| `todo66.zip` | 8345 | `b6d2c246b585d6284b9003c0a9062373db0a5676681ff9329cc5b0724b408996` | 2026-06-10 16:46:57 | n/a | n/a |

## TODO-022 gate

Before marking `TODO-022` done, the implementation record must show one of these:

```text
A. Reader tests used this local fixture and hash checks passed.
B. Reader tests used a repo-local sanitized fixture derived from this manifest.
```

Synthetic XML tests may exist, but they cannot be the only evidence for `TODO-022`.
