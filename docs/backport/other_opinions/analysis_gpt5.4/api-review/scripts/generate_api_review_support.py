#!/usr/bin/env python3

import json
import re
import subprocess
from pathlib import Path

import yaml


ROOT = Path("/home/lintaoz/work/mongo")
GEN = ROOT / "analysis" / "api-review" / "generated"
FULL = ROOT / "analysis" / "api-review" / "full"
MONGOSH = Path("/home/lintaoz/work/mongosh")

VERSIONS = [
    ("v4", "origin/v4.4"),
    ("v5", "origin/v5.0"),
    ("v6", "origin/v6.0"),
    ("v7", "origin/v7.0"),
    ("v8", "origin/v8.0"),
]


def git_show(ref: str, path: str) -> str:
    return subprocess.check_output(
        ["git", "show", f"{ref}:{path}"], cwd=ROOT, text=True
    )


def parse_shell_options(ref: str):
    data = yaml.safe_load(git_show(ref, "src/mongo/shell/shell_options.idl"))
    configs = data.get("configs", {})
    items = []
    for name, cfg in configs.items():
        items.append(
            {
                "name": name,
                "section": cfg.get("section"),
                "short_name": cfg.get("short_name"),
                "single_name": cfg.get("single_name"),
                "description": cfg.get("description"),
                "arg_vartype": cfg.get("arg_vartype"),
                "default": cfg.get("default"),
                "hidden": bool(cfg.get("hidden")),
                "conflicts": cfg.get("conflicts"),
                "cpp_varname": cfg.get("cpp_varname"),
                "positional": cfg.get("positional"),
                "source": cfg.get("source"),
                "duplicate_behavior": cfg.get("duplicate_behavior"),
            }
        )
    return sorted(items, key=lambda x: x["name"])


def md_escape(text):
    if text is None:
        return ""
    return str(text).replace("|", "\\|").replace("\n", " ")


def render_shell_flags(version: str, flags):
    lines = [f"# {version} Legacy Shell Flags", ""]
    lines.append(f"- Total flags/options: `{len(flags)}`")
    lines.append("")
    lines.append(
        "| Flag | Type | Short | Single | Section | Default | Hidden | Positional | Conflicts | Description |"
    )
    lines.append("|---|---|---|---|---|---|---:|---|---|---|")
    for flag in flags:
        lines.append(
            "| `{name}` | `{type}` | `{short}` | `{single}` | `{section}` | `{default}` | `{hidden}` | `{positional}` | `{conflicts}` | {desc} |".format(
                name=md_escape(flag["name"]),
                type=md_escape(flag["arg_vartype"]),
                short=md_escape(flag["short_name"]),
                single=md_escape(flag["single_name"]),
                section=md_escape(flag["section"]),
                default=md_escape(flag["default"]),
                hidden="yes" if flag["hidden"] else "",
                positional=md_escape(flag["positional"]),
                conflicts=md_escape(flag["conflicts"]),
                desc=md_escape(flag["description"]),
            )
        )
    return "\n".join(lines) + "\n"


def diff_flags(old_flags, new_flags):
    old = {f["name"]: f for f in old_flags}
    new = {f["name"]: f for f in new_flags}
    added = sorted(set(new) - set(old))
    removed = sorted(set(old) - set(new))
    changed = []
    for name in sorted(set(old) & set(new)):
        o = old[name]
        n = new[name]
        fields = [
            key
            for key in (
                "section",
                "short_name",
                "single_name",
                "description",
                "arg_vartype",
                "default",
                "hidden",
                "conflicts",
                "cpp_varname",
                "positional",
                "source",
                "duplicate_behavior",
            )
            if o.get(key) != n.get(key)
        ]
        if fields:
            changed.append((name, fields))
    return added, removed, changed


def render_flag_diff(old_version, new_version, old_flags, new_flags):
    added, removed, changed = diff_flags(old_flags, new_flags)
    lines = [f"# Shell Flag Diff {old_version} -> {new_version}", ""]
    lines.append(f"- Added: `{len(added)}`")
    lines.append(f"- Removed: `{len(removed)}`")
    lines.append(f"- Changed: `{len(changed)}`")
    lines.append("")
    if added:
        lines.append("## Added")
        lines.append("")
        for name in added:
            lines.append(f"- `{name}`")
        lines.append("")
    if removed:
        lines.append("## Removed")
        lines.append("")
        for name in removed:
            lines.append(f"- `{name}`")
        lines.append("")
    if changed:
        lines.append("## Changed")
        lines.append("")
        for name, fields in changed:
            lines.append(f"- `{name}`: {', '.join(f'`{f}`' for f in fields)}")
        lines.append("")
    return "\n".join(lines)


def extract_mongosh_methods():
    files = [
        MONGOSH / "packages" / "shell-api" / "src" / "database.ts",
        MONGOSH / "packages" / "shell-api" / "src" / "collection.ts",
        MONGOSH / "packages" / "shell-api" / "src" / "mongo.ts",
    ]
    method_re = re.compile(
        r"^\s*(?:public\s+)?(?:(async)\s+)?([A-Za-z_][A-Za-z0-9_]*)\((.*)\)\s*(?::\s*([^{]+))?\s*\{"
    )
    decorator_re = re.compile(r"^\s*@(.+)$")
    items = []
    for file in files:
        decorators = []
        for lineno, line in enumerate(file.read_text().splitlines(), start=1):
            dec = decorator_re.match(line)
            if dec:
                decorators.append("@" + dec.group(1).strip())
                continue
            m = method_re.match(line)
            if not m:
                decorators = []
                continue
            async_kw, name, params, ret = m.groups()
            if name.startswith("_"):
                decorators = []
                continue
            items.append(
                {
                    "file": file.name,
                    "line": lineno,
                    "method": name,
                    "signature": f"{'async ' if async_kw else ''}{name}({params})"
                    + (f": {ret.strip()}" if ret else ""),
                    "decorators": decorators,
                }
            )
            decorators = []
    return items


def render_mongosh_methods(methods):
    lines = ["# mongosh Shell API Signatures", ""]
    lines.append(f"- Total public methods captured: `{len(methods)}`")
    lines.append("")
    lines.append("| File | Line | Method | Signature | Decorators |")
    lines.append("|---|---:|---|---|---|")
    for item in methods:
        lines.append(
            "| `{file}` | {line} | `{method}` | `{sig}` | {decs} |".format(
                file=item["file"],
                line=item["line"],
                method=item["method"],
                sig=md_escape(item["signature"]),
                decs=md_escape(", ".join(item["decorators"])),
            )
        )
    return "\n".join(lines) + "\n"


def main():
    GEN.mkdir(parents=True, exist_ok=True)
    FULL.mkdir(parents=True, exist_ok=True)

    version_flags = {}
    for version, ref in VERSIONS:
        flags = parse_shell_options(ref)
        version_flags[version] = flags
        (GEN / f"{version}_shell_flags.json").write_text(json.dumps(flags, indent=2) + "\n")
        (GEN / f"{version}_shell_flags.md").write_text(render_shell_flags(version, flags))

    for (old_version, _), (new_version, _) in zip(VERSIONS, VERSIONS[1:]):
        (GEN / f"{old_version}_to_{new_version}_shell_flag_diff.md").write_text(
            render_flag_diff(
                old_version,
                new_version,
                version_flags[old_version],
                version_flags[new_version],
            )
        )

    methods = extract_mongosh_methods()
    (GEN / "mongosh_method_signatures.json").write_text(json.dumps(methods, indent=2) + "\n")
    (GEN / "mongosh_method_signatures.md").write_text(render_mongosh_methods(methods))

    bundles = {
        "v4": [
            ROOT / "analysis" / "api-review" / "v4.md",
            GEN / "v4_idl_commands.md",
            GEN / "v4_manual_commands.md",
            GEN / "v4_shell_flags.md",
        ],
        "v5": [
            ROOT / "analysis" / "api-review" / "v5.md",
            GEN / "v4_to_v5_idl_diff.md",
            GEN / "v4_to_v5_manual_diff.md",
            GEN / "manual_added_command_notes.md",
            GEN / "v5_idl_commands.md",
            GEN / "v5_manual_commands.md",
            GEN / "v5_shell_flags.md",
            GEN / "v4_to_v5_shell_flag_diff.md",
        ],
        "v6": [
            ROOT / "analysis" / "api-review" / "v6.md",
            GEN / "v5_to_v6_idl_diff.md",
            GEN / "v5_to_v6_manual_diff.md",
            GEN / "manual_added_command_notes.md",
            GEN / "v6_idl_commands.md",
            GEN / "v6_manual_commands.md",
            GEN / "v6_shell_flags.md",
            GEN / "v5_to_v6_shell_flag_diff.md",
        ],
        "v7": [
            ROOT / "analysis" / "api-review" / "v7.md",
            GEN / "v6_to_v7_idl_diff.md",
            GEN / "v6_to_v7_manual_diff.md",
            GEN / "manual_added_command_notes.md",
            GEN / "v7_idl_commands.md",
            GEN / "v7_manual_commands.md",
            GEN / "v7_shell_flags.md",
            GEN / "v6_to_v7_shell_flag_diff.md",
        ],
        "v8": [
            ROOT / "analysis" / "api-review" / "v8.md",
            GEN / "v7_to_v8_idl_diff.md",
            GEN / "v7_to_v8_manual_diff.md",
            GEN / "manual_added_command_notes.md",
            GEN / "v8_idl_commands.md",
            GEN / "v8_manual_commands.md",
            GEN / "v8_shell_flags.md",
            GEN / "v7_to_v8_shell_flag_diff.md",
            ROOT / "analysis" / "api-review" / "mongosh.md",
            GEN / "mongosh_shell_api_inventory.md",
            GEN / "mongosh_method_signatures.md",
        ],
    }

    for version, parts in bundles.items():
        out = [f"# {version.upper()} Full Reference", ""]
        for path in parts:
            out.append(f"<!-- BEGIN {path.name} -->")
            out.append(path.read_text().rstrip())
            out.append(f"<!-- END {path.name} -->")
            out.append("")
        (FULL / f"{version}_reference.md").write_text("\n".join(out))


if __name__ == "__main__":
    main()
