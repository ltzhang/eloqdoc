# MongoDB 4.0 -> 8.0 API Backport Analysis

This directory is a fresh analysis output for the task in `Agents.md`.  The existing `analysis/` and `analysis_cc/` directories were not used as inputs.

## Scope and Refs

Analyzed refs:

| Version | Git ref |
| --- | --- |
| 4.0 | `origin/v4.0` |
| 5.0 | `origin/v5.0` |
| 6.0 | `origin/v6.0` |
| 7.0 | `origin/v7.0` |
| 8.0 | `origin/v8.0` |

Scope follows `Agents.md`: data model and data manipulation API compatibility for a 4.0-derived server, including security-relevant command authorization and client helper behavior where this repository contains it.  Systems features such as sharding, replication, transactions, tenant migration, and serverless commands are classified as ignore/defer unless they directly change user-facing data APIs.

## Documents

- `v4.0-api-surface.md`: baseline 4.0 command/parser surface and client helper baseline.
- `v5.0-api-surface.md`: 5.0 additions and backport evaluation.
- `v6.0-api-surface.md`: 6.0 additions and backport evaluation.
- `v7.0-api-surface.md`: 7.0 additions and backport evaluation.
- `v8.0-api-surface.md`: 8.0 additions and backport evaluation.
- `curated-data-command-matrix.md`: public data-command availability after filtering old parser quirks and system/test commands.
- `mongosh-client-surface.md`: addendum from the sibling `../mongosh` checkout and its Node-driver-backed helper surface.
- `feature-backport-evaluation.md`: feature-by-feature implementation classification.
- `final-comparison.md`: cross-version differences, gaps, and recommended 4.0 forward-compatibility roadmap.
- `raw/`: mechanically extracted command, aggregation, expression, accumulator, and diff data.  The raw extractor is useful evidence but not authoritative for older hand-written commands; the narrative docs correct for those misses.

## Important Source Notes

- MongoDB 4.0 uses more hand-written parsers and command classes.  Later versions move many request shapes to IDL, so mechanical IDL diffs undercount 4.0 commands such as `createIndexes` and `dbStats`.
- Modern `mongosh` is not present inside the server repo, but this workspace has a sibling `../mongosh` checkout.  See `mongosh-client-surface.md`; it updates the client-side conclusions.
- Public data commands that exist in every version, even when not IDL-generated in 4.0, include `find`, `aggregate`, `insert`, `update`, `delete`, `findAndModify`, `create`, `createIndexes`, `collMod`, `listCollections`, `listIndexes`, `drop`, `dropIndexes`, `count`, `distinct`, `mapReduce`, `renameCollection`, and `validate`.
