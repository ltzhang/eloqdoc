# MongoDB API Review

This folder now contains an implementation-grade review of the server-side command surface, the in-tree legacy shell, and the standalone `mongosh` client.

Main documents:

- `implementation-index.md`: reading order, counts, appendices, and porting workflow
- `v4.md`: v4.4 baseline
- `v5.md`: v5.0 request-envelope and Stable API transition
- `v6.md`: v6.0 typed-surface expansion
- `v7.md`: v7.0 request-metadata ordering change
- `v8.md`: v8.0 `CommonRequestArgs` and explicit API validation
- `comparison.md`: cross-version matrix and recommended porting order
- `mongosh.md`: standalone `mongosh` workbook based on `/home/lintaoz/work/mongosh`
- `full/`: single-file per-version references that inline the inventories and diffs

Generated appendices live under `generated/` and include:

- per-version IDL command catalogs
- per-version manual/non-IDL command catalogs
- adjacent-version command diffs
- manual added-command notes
- per-version legacy shell flag inventories and diffs
- `mongosh` method inventories and signatures
- full per-version reference bundles

Important limitation:

- IDL command coverage is field-level exhaustive.
- Manual-command coverage is exhaustive for discovery, but not a universal field-level schema dump for every legacy/manual command.
