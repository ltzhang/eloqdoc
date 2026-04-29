# Queryable Encryption (FLE2)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 3 | 6.0 (equality) / 7.0 (range) / 8.0 (cleanup) | 4–6 months |

## Scope

Server-side support for **Queryable Encryption** — clients encrypt sensitive fields, but the server can still execute equality (and range, in 7.0) queries against ciphertexts without seeing plaintext. The server stores per-collection state collections (ESC, ECC, ECOC) that hold encrypted index entries.

This is the most ambitious data-API addition since the original sharding implementation. **All three source analyses agree it is Tier 3.**

## Components

1. **`encryptedFields` collection option (6.0)** — declares which fields are encrypted and with what schemas.
2. **State collections** — ESC (Encrypted State Collection — anti-spoofing), ECC (Encrypted Compaction Collection), ECOC (Encrypted Counter Collection). Auto-created with the parent encrypted collection.
3. **Encrypted insert/update.** Client-side library (CSFLE) generates encrypted-index "tokens" from plaintext; server stores both the ciphertext field value AND the tokens in the state collections.
4. **Encrypted query rewriting.** `find` with `{encryptedField: <ciphertext>}` looks up matching tokens in the state collections, then fetches the matching documents by `_id`.
5. **Compaction (`compactStructuredEncryptionData`, 6.0/8.0)** — periodic cleanup of the ECC collection, garbage-collecting stale tokens.
6. **Cleanup (`cleanupStructuredEncryptionData`, 8.0)** — delete-token resolution.
7. **Range queries (7.0)** — internal `queryable_encrypted_range` index type.
8. **`encryptionInformation` field** on `find`/`update`/`aggregate` — carries client-side encryption metadata to the server.

## Why it's Tier 3

- Cryptographic protocol implementation. Get it wrong → silent data leak.
- Three new system collections per encrypted collection. Significant catalog and storage work.
- New encrypted index access methods (range form).
- Query rewriting that bridges encrypted predicates to state-collection lookups.
- Multiple new commands (`compact`, `cleanup`, `getQueryableEncryptionCountInfo`).
- All of it needs to be wire-compatible with the upstream client libraries.

## Phasing

### Phase 1 — Equality (6.0 minimum) (~3 months)
1. `encryptedFields` parsing on `create`. ~1 week.
2. Auto-create ESC, ECC, ECOC collections. ~2 weeks.
3. Encrypted insert path (writes to data + state collections). ~3 weeks.
4. Encrypted equality query rewriting. ~3 weeks.
5. `compactStructuredEncryptionData` command. ~2 weeks.
6. End-to-end with upstream Node driver in QE mode. ~2 weeks.

### Phase 2 — Range (7.0) (~6 weeks)
- `queryable_encrypted_range` index type.
- Range query rewriting.

### Phase 3 — 8.0 enhancements (~3 weeks)
- `cleanupStructuredEncryptionData` command.
- Compaction enhancements (`anchorPaddingFactor`, `validator`).

## Recommendation

**Don't start without a security expert on the team.** The cryptographic protocol must be implemented exactly to spec — this is not a feature to "wing it" on. If the team lacks crypto expertise, partner with MongoDB's documentation on FLE2 wire protocol and use the existing libmongocrypt as a reference (it implements the client side; the server side mirrors a subset).

## Dependencies

- **Blocked by:** [`pre-post-images-collection-option.md`](./pre-post-images-collection-option.md) full semantics (Tier 3) — FLE2 audit and pre-image features can interact, but minimal FLE2 doesn't require pre-images.

## EloqDoc-specific considerations

- **State collection storage.** ESC/ECC/ECOC are regular collections from the storage engine's perspective. EloqStore handles them like any other.
- **Authorization.** FLE2 introduces new privileges; reuse the standard role-based system but ensure encrypted collections have separate read/write privileges.
- **Distributed semantics.** Stock MongoDB FLE2 has subtle interactions with sharding/transactions. Single-node EloqDoc avoids these — simpler.

## Notes from source analyses

All three analyses rate Tier 3 unequivocally. `analysis_cc/forward_compat_eval.md` Section G has the most detailed phasing. `analysis_gpt5.5/feature-backport-evaluation.md`: "Requires FLE2 state collections, encrypted query planning, token handling, compaction/cleanup commands."

**Implementation strategy:** before writing any code, produce a 10–20-page design doc covering: state-collection schemas, query-rewriting algorithm, threat model, and end-to-end test plan. Without this design doc, the project will fail.
