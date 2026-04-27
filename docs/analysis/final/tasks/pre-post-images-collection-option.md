# Pre/post-image collection option (parser+catalog only)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 (parser+catalog) / 3 (full semantics) | 5.0 / 6.0 | 1 week (parser+catalog) |

## Scope

Two related options that record document snapshots before/after writes for change-stream consumers:

- **`recordPreImages: true`** (5.0) — record pre-images in a side collection.
- **`changeStreamPreAndPostImages: { enabled: true }`** (6.0; supersedes the 5.0 form) — finer-grained control; affects change-stream output.

```js
db.createCollection("c", { changeStreamPreAndPostImages: { enabled: true } })
db.runCommand({ collMod: "c", changeStreamPreAndPostImages: { enabled: true } })
```

**In scope (Tier 2):**
- Parse the option in `create` and `collMod`.
- Persist on `CollectionOptions`.
- Surface in `listCollections` output.

**Out of scope (Tier 3, if pursued — see [`change-streams-pre-post-image.md`](./tasks/change-streams-pre-post-image.md)):**
- Actually capturing pre/post images.
- Wiring into change-stream output.
- The pre-image side collection storage.

## Extension pattern

Pattern G (collection option).

```cpp
// In CollectionOptions struct:
struct PreAndPostImagesConfig { bool enabled = false; };
boost::optional<PreAndPostImagesConfig> changeStreamPreAndPostImages;
bool recordPreImages = false;

// In CollectionOptions::parse:
if (auto el = obj["changeStreamPreAndPostImages"]) {
    PreAndPostImagesConfig cfg;
    cfg.enabled = el.Obj()["enabled"].trueValue();
    changeStreamPreAndPostImages = cfg;
}

// In appendBSON: serialize back if set.
```

## Files to modify

- `src/mongo/db/catalog/collection_options.h` and `.cpp` — add fields, parse, serialize
- `src/mongo/db/catalog/coll_mod.cpp` — accept option
- `src/mongo/db/catalog/collection_impl.cpp` (or wherever collection creation happens) — store
- `src/mongo/db/commands/list_collections.cpp` — surface in output

## Dependencies

- **Blocks:** [`change-streams-pre-post-image.md`](./change-streams-pre-post-image.md) (Tier 3).
- **Blocked by:** none.

## EloqDoc-specific considerations

- **Change streams in EloqDoc.** Verify the current state of change-stream support. EloqDoc has `$changeStream` registered as a multi-stage alias; whether the underlying invalidation/event mechanism is implemented depends on Data Substrate's notification capabilities. Without a working change stream, the pre/post-image option is metadata-only — which is what this Tier 2 task delivers.
- **Storage of pre-images.** Stock MongoDB stores pre-images in `config.system.preimages`. EloqDoc would need an equivalent. Not in scope for this Tier 2 task — defer to Tier 3.

## Acceptance criteria

- `create` with `changeStreamPreAndPostImages: {enabled: true}` succeeds; `listCollections` reflects it.
- `collMod` flips the value; `listCollections` reflects the new state.
- `create` with the older `recordPreImages: true` is accepted (deprecated path) and surfaces in `listCollections`.
- Validation: `enabled` must be boolean.
- **Test entry point:** `tests/jstests/eloq_basic/pre_post_images_options.js`.

## Notes from source analyses

`analysis_gpt5.5/feature-backport-evaluation.md`: "Catalog option alone is easy; real semantics require pre-image storage and change stream integration." `analysis_cc/forward_compat_eval.md` agrees and rates the parser-only Tier 2. The full semantics implementation is a Tier 3 project (depends on Data Substrate's event-notification model).
