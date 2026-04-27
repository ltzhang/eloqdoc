# `rotateCertificates` command

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 | 4.4 / 5.0 | 1–2 days |

## Scope

Add the `rotateCertificates` admin command, which causes the running server to reload its TLS certificate (and CA bundle) from disk without restarting. This is a well-defined ops command; modern deployments (Kubernetes cert-manager, Let's Encrypt) rely on it.

**In scope:**
- Command accepts no arguments (or only the optional `message` field for log output).
- Re-reads `net.tls.certificateKeyFile` and related TLS options from the active config.
- Reinitializes the TLS context used by the listener.

**Out of scope:**
- Reloading other config (this command does TLS only).

## Extension pattern

Pattern A (add a command). The TLS-context reload helper likely already exists for the SIGUSR1 handler — `rotateCertificates` is a thin wrapper around it.

```cpp
// src/mongo/db/commands/rotate_certificates.cpp
class CmdRotateCertificates : public BasicCommand {
public:
    CmdRotateCertificates() : BasicCommand("rotateCertificates") {}
    bool adminOnly() const override { return true; }
    bool requiresAuth() const override { return true; }  // requires hostManager privilege
    bool run(OperationContext*, const std::string&,
             const BSONObj& cmdObj, BSONObjBuilder& result) override {
        auto status = SSLManager::get()->rotateCertificates();
        uassertStatusOK(status);
        if (auto msg = cmdObj["message"]) log() << "rotateCertificates: " << msg.toString();
        return true;
    }
};
static CmdRotateCertificates cmdRotateCertificates;
```

## Files to create

- `src/mongo/db/commands/rotate_certificates.cpp`

## Files to modify

- `src/mongo/util/net/ssl_manager.h` — expose `rotateCertificates()` if not already public. Look for the existing SIGUSR1 handler for reference.
- `src/mongo/db/commands/SConscript`
- `src/mongo/db/auth/action_types.txt` — add `rotateCertificates` action if absent (or reuse `hostManager` cluster action)

## Dependencies

- **Blocks:** none.
- **Blocked by:** none.

## EloqDoc-specific considerations

If EloqDoc has additional internal TLS contexts (for Data Substrate inter-node communication, log service mTLS, S3 client), this command should rotate those too — but only if the same on-disk paths are used. Audit `src/mongo/db/modules/eloq/` for additional SSL contexts before declaring done.

## Acceptance criteria

- `db.adminCommand({rotateCertificates: 1})` succeeds when run as a user with the `hostManager` role.
- An incoming TLS connection after the rotation uses the new certificate (verifiable with `openssl s_client`).
- The command fails with `Unauthorized` for unprivileged users.
- The command fails with a clear error if the cert file on disk is malformed.
- **Test entry point:** `tests/jstests/eloq_basic/rotate_certificates.js` (note: harder to test without certificate fixtures; an integration test under `tests/integration_tests/` may be more appropriate).

## Notes from source analyses

`analysis_cc/forward_compat_eval.md` rates this Category 1 ("Very Easily Doable") with a one-line note: "leverage existing SIGUSR1 reload code." `analysis_gpt5.5` agrees. The trap is forgetting that EloqDoc may have multiple SSL contexts — see EloqDoc-specific considerations above.
