# Change-stream pre/post-image full integration

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 3 | 5.0 / 6.0 | 1–2 months |

## Scope

Captures pre-images (and post-images, 6.0) of documents on update/delete, so that change-stream consumers can see what was there before and after each write.

Builds on the parser-only [`pre-post-images-collection-option.md`](./pre-post-images-collection-option.md) (Tier 2). This task adds the actual semantics:

1. **Capture.** On update/delete to a collection with the option enabled, capture the pre-image (and post-image for updates).
2. **Storage.** Persist images in `config.system.preimages` (or equivalent in EloqDoc).
3. **TTL on the pre-image collection.** Old pre-images deleted after a configurable interval.
4. **Change-stream output.** `$changeStream` events include `fullDocumentBeforeChange` and `fullDocument` from the pre/post image collection.

## Why it's Tier 3 in EloqDoc

EloqDoc's change-stream implementation depends on Data Substrate's event-notification model (no MongoDB oplog). Stock MongoDB hooks into the OpObserver pipeline, which fires on every write. EloqDoc must re-create the equivalent — capture writes at the storage-engine boundary or at the command-handler boundary.

**Key architectural decisions:**
- Where to hook (write command, storage engine, both)?
- How to store images efficiently (avoid per-write overhead for collections that don't have the option)?
- Cleanup TTL strategy.
- Backpressure when the change-stream consumer is slow.

## Dependencies

- **Blocked by:**
  - Working `$changeStream` semantics in EloqDoc. Verify; without that, pre/post images have no consumer.
  - [`pre-post-images-collection-option.md`](./pre-post-images-collection-option.md) (Tier 2) — option parsing.

## Phasing

1. **Pre-image side collection:** create `config.system.preimages` and write pre-images on update/delete. ~3 weeks.
2. **Post-image (6.0):** also write post-images for updates. ~2 weeks.
3. **TTL:** background cleanup. ~1 week.
4. **Change-stream integration:** wire pre/post images into `$changeStream` events. ~3 weeks.
5. **Performance:** make the capture cheap when the option is off. ~1 week.

## Recommendation

Don't start until there's clear product demand for pre/post images. Most change-stream consumers (CDC pipelines) work fine with just the post-image (which is the document's current state and doesn't require this feature).

## Notes from source analyses

`analysis_gpt5.5/feature-backport-evaluation.md`: "Catalog option alone is easy; real semantics require pre-image storage and change stream integration." `analysis_cc` rates Tier 3 explicitly. `analysis_gpt5.4` notes this is most useful for FLE2 audit and CDC use cases — both of which the team should evaluate before committing.
