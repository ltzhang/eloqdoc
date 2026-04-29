# OIDC authentication (`MONGODB-OIDC`)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 3 | 7.0 | 2 months |

## Scope

OAuth 2.0 / OpenID Connect authentication mechanism. The client obtains an access token from an external IdP (Identity Provider, e.g. Okta, Auth0, Azure AD) and presents it to the server. The server verifies the token via the IdP's JWKS (JSON Web Key Set) endpoint and maps the token's claims to a MongoDB user.

```js
const conn = MongoClient.connect(uri, {
  authMechanism: "MONGODB-OIDC",
  authSource: "$external",
  authMechanismProperties: { ... }
})
```

Server requires:
- `oidcIdentityProviders` config array (URLs, client IDs, scopes).
- JWT verification using JWKS endpoint of each configured IdP.
- Mapping from JWT claims (`sub`, `groups`) to MongoDB roles.
- SASL conversation handling for the OIDC mechanism.

## Why it's Tier 3

1. **JWT verification** requires JWS/JWKS handling — public-key crypto with RS256/ES256/EdDSA support.
2. **JWKS caching and rotation.** Refresh keys periodically; handle key rollover.
3. **Claims-to-role mapping** is policy-rich; user-facing config.
4. **SASL mechanism wiring.** The OIDC SASL exchange is multi-step (initial token, refresh, audience-checks).
5. **Audit trail.** Failed authentications must be logged with sufficient context for ops.

## Extension pattern

`SaslMechanismRegistry` already exists in EloqDoc 4.0.3 (verified). Add a new mechanism subclass.

```cpp
// src/mongo/db/auth/sasl_mechanism_oidc.cpp
class OIDCServerMechanism : public SaslMechanismBase {
public:
    StatusWith<std::tuple<std::string, std::string>> step(StringData input) override {
        // 1. Receive presented JWT.
        // 2. Verify signature against cached JWKS.
        // 3. Validate `aud`, `iss`, `exp`.
        // 4. Map claims to user; return user.
    }
};
class OIDCMechanismFactory : public ServerFactory<OIDCServerMechanism> {
    // Register via REGISTER_SASL_MECHANISM("MONGODB-OIDC", ...)
};
```

## Files to create

- `src/mongo/db/auth/sasl_mechanism_oidc.{cpp,h}`
- `src/mongo/util/jwt/jwt_validator.{cpp,h}`
- `src/mongo/util/jwt/jwks_cache.{cpp,h}`
- `src/mongo/db/auth/oidc_identity_provider_config.{cpp,h}`

## Files to modify

- `src/mongo/db/auth/sasl_mechanism_registry.cpp` — register OIDC
- `src/mongo/db/auth/sasl_options.{cpp,h}` — config parsing for IdPs
- `src/mongo/db/server_options.h` — add OIDC server-config fields
- (third-party) — ensure a JWT/JWKS library is available (e.g. `jwt-cpp`)

## Dependencies

- **Blocked by:** none.
- **Blocks:** none.

## EloqDoc-specific considerations

- **Network egress to IdP.** EloqDoc deployments must allow outbound HTTPS to the IdP for JWKS fetches. Document this.
- **Time sync.** JWT validation depends on server clock being roughly aligned with IdP. Standard NTP is sufficient; document.
- **Caching across nodes.** In a multi-node EloqDoc deployment, JWKS cache is per-node. No coordination needed.

## Acceptance criteria

- Server accepts `MONGODB-OIDC` SASL handshake.
- JWT signature verified against configured IdP's JWKS.
- `aud`, `iss`, `exp` claims validated.
- Token's `sub` (or configured claim) maps to a MongoDB user.
- `groups` claim (if configured) maps to MongoDB roles.
- Expired tokens rejected.
- JWKS cache refreshes per IdP-published cache hints.
- Failed auth logs include IdP, claim summary (sanitized), reason.
- **Test entry point:** `tests/integration_tests/oidc_auth.js`. Requires a mock IdP fixture.

## Notes from source analyses

`analysis_cc/forward_compat_eval.md` rates Tier 3 ("very hard") primarily for the cryptographic infrastructure. `analysis_gpt5.5` agrees. The 2-month estimate assumes vendoring an existing JWT library; building from scratch would double it.

This is the most self-contained Tier 3 task — no storage-engine work, no planner work. A good first Tier 3 to attempt for a team learning the codebase.
