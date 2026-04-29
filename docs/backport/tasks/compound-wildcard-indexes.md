# Compound wildcard indexes

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 3 | 6.0 (experimental) / 7.0 (GA) | 1–2 months |

## Scope

Extends [`wildcard-indexes.md`](./wildcard-indexes.md) (Tier 2) to support compound indexes where one component is a wildcard expansion:

```js
db.c.createIndex({ "a.$**": 1, b: 1 })           // wildcard component first
db.c.createIndex({ tenantId: 1, "metadata.$**": 1 })   // wildcard component second
```

This is significantly harder than single-field wildcard because the planner must reason about which sub-predicates each component satisfies.

## Why it's Tier 3

`analysis_cc/forward_compat_eval.md` calls this out: "The interaction between wildcard semantics and compound index planning is deeply complex. The planner needs major new logic to reason about which sub-predicates are satisfiable by which component."

Specifically:
1. **Key generation.** A document with `metadata: {a: 1, b: 2}` and `tenantId: 5` produces multiple keys: `[5, "metadata.a", 1]`, `[5, "metadata.b", 2]`. The encoding must distinguish wildcard expansion from concrete components.
2. **Multikey + wildcard.** A wildcard component on an array path is multikey AND wildcard simultaneously.
3. **Bound calculation.** Given a query `{tenantId: 5, "metadata.a": {$gt: 0}}`, the planner must compose bounds across the concrete component (tenantId=5) and the wildcard component (path="metadata.a", value > 0).
4. **Index intersection.** Compound wildcard sometimes interacts with index intersection in non-obvious ways.

## Dependencies

- **Blocked by:** [`wildcard-indexes.md`](./wildcard-indexes.md) (Tier 2) — single-field wildcard must work first.

## Phasing

1. **Storage (key generation):** ~2 weeks.
2. **Single-direction compound (wildcard last):** ~3 weeks.
3. **Wildcard-first compound:** ~3 weeks.
4. **Multikey + wildcard interactions:** ~2 weeks.
5. **Production hardening + jstest port:** ~2 weeks.

## Recommendation

Do not start until single-field wildcard is shipped and stable. If the team has limited Tier 3 capacity, defer this — most use cases for wildcard are well-served by the single-field form.

## Notes from source analyses

`analysis_cc/forward_compat_eval.md` Section A2 has the most detail. `analysis_gpt5.5` rates Tier 3. `analysis_gpt5.4` agrees and notes 8.0 deprecated columnstore in favor of compound wildcard for many use cases — making this strategically valuable, but only after the simpler wildcard ships.
