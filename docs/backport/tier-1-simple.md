# Tier 1 — Simple

**Definition:** Parser-only changes, pure expression additions, stage aliases, and other features that fit one of the seven extension patterns without touching the planner, executor, or storage layer. Each task is single-PR sized — typically a few hundred lines of new C++ plus jstests. One engineer, days each.

**Total tasks:** 16. Aggregate effort: ~6–10 engineer-weeks if done sequentially; can be parallelized 4–6 ways with no significant conflicts.

## Recommended phasing

### Phase 1.A — "Compatibility week" (1 engineer-week)
Get the modern driver handshake to succeed and quiet down `apiStrict` failures.

1. [`stable-api-envelope.md`](./tasks/stable-api-envelope.md) — accept `apiVersion`/`apiStrict`/`apiDeprecationErrors` permissively
2. [`hello-command.md`](./tasks/hello-command.md) — alias of `isMaster`
3. [`common-command-fields.md`](./tasks/common-command-fields.md) — accept-and-ignore `comment`, `collectionUUID`, etc.
4. [`rotate-certificates.md`](./tasks/rotate-certificates.md) — `rotateCertificates` command (TLS reload)

### Phase 1.B — "Expressions sprint" (~3 engineer-weeks, parallelizable)
Mass addition of pure aggregation expressions. Each is independent — split across engineers.

5. [`pure-expressions-batch.md`](./tasks/pure-expressions-batch.md) — trig (14), regex (5), trim (3), conversions (10), bit-ops (4), `$round`/`$trunc`, `$function`, `$accumulator`, `$rand`, `$binarySize`, `$bsonSize`, `$toHashedIndexKey`
6. [`date-arithmetic-expressions.md`](./tasks/date-arithmetic-expressions.md) — `$dateAdd`, `$dateDiff`, `$dateSubtract`, `$dateTrunc` (parser-easy, semantics-tricky)
7. [`getfield-setfield-unsetfield.md`](./tasks/getfield-setfield-unsetfield.md) — field-name access expressions
8. [`sortarray-firstn-lastn-expressions.md`](./tasks/sortarray-firstn-lastn-expressions.md) — `$sortArray`, N-value expressions

### Phase 1.C — "Stage aliases & cheap stages" (~1 engineer-week)
9. [`stage-aliases-set-unset-replacewith.md`](./tasks/stage-aliases-set-unset-replacewith.md) — `$set`/`$unset`/`$replaceWith`
10. [`document-source-documents.md`](./tasks/document-source-documents.md) — `$documents` literal source
11. [`document-source-plan-cache-stats.md`](./tasks/document-source-plan-cache-stats.md) — `$planCacheStats`

### Phase 1.D — "Cheap accumulator + flag" (~3 engineer-days)
12. [`accumulator-count.md`](./tasks/accumulator-count.md) — `$count` accumulator (group-context)
13. [`distinct-hint.md`](./tasks/distinct-hint.md) — `distinct.hint`
14. [`commitquorum-parsing.md`](./tasks/commitquorum-parsing.md) — parser-only stub (escalates to Tier 2 if real semantics required)

## Index of all Tier 1 tasks

| # | Task | Pattern | Effort | Spec |
| - | ---- | ------- | ------ | ---- |
| 1 | Stable API envelope | A (command) | 2–3 d | [stable-api-envelope.md](./tasks/stable-api-envelope.md) |
| 2 | `hello` command | A | 1 d | [hello-command.md](./tasks/hello-command.md) |
| 3 | Common command fields | command framework | 2 d | [common-command-fields.md](./tasks/common-command-fields.md) |
| 4 | `rotateCertificates` | A | 1–2 d | [rotate-certificates.md](./tasks/rotate-certificates.md) |
| 5 | Pure expressions batch (~36 expressions) | C (expression) | 2–3 w | [pure-expressions-batch.md](./tasks/pure-expressions-batch.md) |
| 6 | Date arithmetic expressions | C | 1 w | [date-arithmetic-expressions.md](./tasks/date-arithmetic-expressions.md) |
| 7 | `$getField`/`$setField`/`$unsetField` | C | 3 d | [getfield-setfield-unsetfield.md](./tasks/getfield-setfield-unsetfield.md) |
| 8 | `$sortArray`, N-value expressions | C | 3 d | [sortarray-firstn-lastn-expressions.md](./tasks/sortarray-firstn-lastn-expressions.md) |
| 9 | `$set`/`$unset`/`$replaceWith` | B (stage alias) | 1–2 d | [stage-aliases-set-unset-replacewith.md](./tasks/stage-aliases-set-unset-replacewith.md) |
| 10 | `$documents` | B | 2 d | [document-source-documents.md](./tasks/document-source-documents.md) |
| 11 | `$planCacheStats` | B | 2–3 d | [document-source-plan-cache-stats.md](./tasks/document-source-plan-cache-stats.md) |
| 12 | `$count` accumulator | D (accumulator) | 1 d | [accumulator-count.md](./tasks/accumulator-count.md) |
| 13 | `distinct.hint` | command field | 1–2 d | [distinct-hint.md](./tasks/distinct-hint.md) |
| 14 | `commitQuorum` (parser stub) | command field | 1 d | [commitquorum-parsing.md](./tasks/commitquorum-parsing.md) |

> **Sequencing tip:** Phase 1.A is a strict prerequisite — without it, modern drivers fail at the handshake and you cannot reproduce later-version client behavior in tests. Do 1.A first.
