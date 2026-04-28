# Wildcard indexes (`{ "$**": 1 }`, `wildcardProjection`)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 4.2 (4.0 catch-up — never in EloqDoc) | 4–6 weeks |

## Scope

Wildcard indexes are MongoDB's mechanism for indexing documents whose schema is unknown or polymorphic. A wildcard index `{"$**": 1}` indexes every field path in every document. The optional `wildcardProjection` lets users include/exclude paths.

```js
db.c.createIndex({"$**": 1})
db.c.createIndex({"$**": 1}, {wildcardProjection: {a: 1, b: 1}})       // only a, b
db.c.createIndex({"$**": 1}, {wildcardProjection: {secrets: 0}})       // exclude secrets
db.c.createIndex({"products.$**": 1})                                   // bounded subtree
```

The planner must recognize that any predicate on an indexed field path can use the wildcard index.

**In scope (Tier 2):**
- Single-field wildcard indexes (uncompounded).
- `wildcardProjection` (inclusion or exclusion mode).
- Subtree wildcards (`field.$**`).
- Planner integration for predicates on individual fields.

**Implementation status: storage plus first planner slice landed.** EloqDoc now accepts single-field
`{"$**": 1}` and subtree wildcard key patterns, accepts `wildcardProjection` inclusion/exclusion
documents, instantiates a wildcard access method, and maintains recursive leaf-path index entries
through insert/update/delete and validation. Query planning now selects wildcard indexes for
covered full, projected, and subtree paths, builds bounds over the internal `{path, value}`
wildcard key shape, and keeps a `FETCH` filter above the scan for correctness.

**Out of scope (Tier 3 — see [`compound-wildcard-indexes.md`](./compound-wildcard-indexes.md)):**
- Compound wildcard indexes (one component is wildcard, others are concrete).

## Extension pattern

Pattern E (add an index type). The infrastructure mirrors stock MongoDB 4.2's `wildcard_*` files (which never landed in EloqDoc).

```cpp
// wildcard_key_generator.cpp — recursively enumerate field paths in a document
class WildcardKeyGenerator {
public:
    void getKeys(const BSONObj& doc, BSONObjSet* keys, ...) const {
        // For each leaf path under the wildcardProjection, emit one key:
        // { fieldPath: <path-string>, value: <leaf-value> }
        recurse(doc, "", keys);
    }
};

// wildcard_access_method.cpp — IndexAccessMethod subclass
class WildcardAccessMethod final : public IndexAccessMethod {
    // delegates to WildcardKeyGenerator for getKeys, otherwise behaves like btree
};
```

## Files to create

- `src/mongo/db/index/wildcard_access_method.{h,cpp}`
- `src/mongo/db/index/wildcard_key_generator.{h,cpp}`
- `src/mongo/db/query/planner_wildcard_helpers.{h,cpp}`
- `src/mongo/db/query/wildcard_index_helpers.{h,cpp}`

## Files to modify

- `src/mongo/db/index_names.h` — add `WILDCARD` constant ("wildcard")
- `src/mongo/db/index/SConscript` — add new files
- `src/mongo/db/index/index_access_method_factory.cpp` (or wherever index types are dispatched) — recognize `$**` in keyPattern
- `src/mongo/db/index/index_descriptor.cpp` — accept `wildcardProjection` from spec
- `src/mongo/db/query/index_entry.h` — wildcard metadata (which paths are projected)
- `src/mongo/db/query/query_planner.cpp` — teach plan enumeration about wildcard candidate selection
- `src/mongo/db/query/index_bounds_builder.cpp` — bound generation for wildcard scans
- `src/mongo/db/query/planner_access.cpp` — convert wildcard match predicates to scans

## Dependencies

- **Blocks:** [`compound-wildcard-indexes.md`](./compound-wildcard-indexes.md) (Tier 3) — builds directly on this work.
- **Blocked by:** none.

## EloqDoc-specific considerations

- **Storage backend.** Wildcard index keys are written to the same B-tree-style index storage used by regular indexes. EloqDoc's `eloq_index.{h,cpp}` is the data-substrate side — verify it can store the multi-component key shape `{fieldPath, value}` without modification. Most likely yes, since the existing multikey infrastructure already stores multi-keyed entries per document.
- **Multikey tracking.** Each wildcard-indexed path may be multikey (array element); the existing multikey-path tracking in `IndexCatalog` must be extended. Stock MongoDB stores this in `system.indexes` metadata; EloqDoc's catalog may need a new field on `IndexDescriptor`.
- **Path normalization.** Field paths with embedded `.` or `$` are tricky. The key generator must escape these consistently — borrow the upstream encoding scheme from MongoDB 4.2 `wildcard_key_generator.cpp` (read-only reference).

## Acceptance criteria

- `db.c.createIndex({"$**":1})` succeeds and is reported in `listIndexes`.
- Inserting a document with arbitrary fields produces one index entry per leaf path.
- `db.c.find({some_field: 5}).explain()` shows the wildcard index used as IXSCAN.
- `wildcardProjection: {include: 1}` only indexes the listed paths.
- `wildcardProjection: {exclude: 0}` indexes everything except the listed paths.
- Subtree form `{"products.$**": 1}` only indexes paths under `products`.
- Multi-document equality, range, and `$exists: true` queries all use the wildcard index when appropriate.
- Update/delete maintains the index correctly (insert + later find returns updated state).
- Validate (`db.c.validate()`) reports the wildcard index as healthy.
- **Test entry point:** `tests/jstests/eloq_basic/wildcard_indexes/`. The upstream test suite (`jstests/core/wildcard_index_*.js`) is large — adapt at least the basic, projection, and update-delete tests.

### Remaining work after first planner slice

- Broaden planner coverage beyond single-predicate equality/range/`$exists:true` cases.

## Notes from source analyses

This is a **catch-up feature**: MongoDB 4.2 added wildcard indexes, but EloqDoc forked from 4.0.3 and skipped that. All three analyses flag it as "missing on 4.0 even if not new in v5."

`analysis_cc/forward_compat_eval.md` has the most concrete file list (above) and rates it Tier 2 with the caveat: "The planner integration is the hardest part but follows existing patterns (similar to how multikey index planning works)." `analysis_gpt5.5` agrees and notes that `wildcardProjection` parsing alone is straightforward — the cost is the planner work.

**Recommended sequencing:** ship the access method + key generator first (without planner integration; queries fall back to COLLSCAN). Test correctness of insert/delete/validate. Then add planner integration as a second PR. This de-risks the largest single piece (planner) by building on a known-correct storage layer.
