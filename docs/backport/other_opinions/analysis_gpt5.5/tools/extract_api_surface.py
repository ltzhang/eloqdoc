#!/usr/bin/env python3
"""Extract MongoDB API-surface signals from multiple git refs.

This intentionally keeps parsing lightweight.  The output is a set of markdown
and JSON files used as evidence for the version-by-version writeups.
"""

from __future__ import annotations

import json
import re
import subprocess
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "analysis_gpt5.5" / "raw"
REFS = {
    "v4.0": "origin/v4.0",
    "v5.0": "origin/v5.0",
    "v6.0": "origin/v6.0",
    "v7.0": "origin/v7.0",
    "v8.0": "origin/v8.0",
}

DATA_RELEVANT_COMMANDS = {
    "aggregate",
    "analyze",
    "bulkWrite",
    "cleanupStructuredEncryptionData",
    "collMod",
    "collStats",
    "compactStructuredEncryptionData",
    "convertToCapped",
    "count",
    "create",
    "createIndexes",
    "createSearchIndexes",
    "dataSize",
    "dbStats",
    "delete",
    "distinct",
    "drop",
    "dropDatabase",
    "dropIndexes",
    "dropSearchIndex",
    "explain",
    "find",
    "findAndModify",
    "getMore",
    "getQueryableEncryptionCountInfo",
    "insert",
    "killCursors",
    "listCollections",
    "listDatabases",
    "listIndexes",
    "listSearchIndexes",
    "mapReduce",
    "planCacheClear",
    "planCacheClearFilters",
    "planCacheListFilters",
    "planCacheSetFilter",
    "renameCollection",
    "setQuerySettings",
    "removeQuerySettings",
    "update",
    "updateSearchIndex",
    "validate",
}

IDL_COMMAND_RE = re.compile(r"^\s*command_name:\s*[\"']?([^\"'\n#]+)[\"']?\s*$")
FIELD_RE = re.compile(r"^\s{8,}([A-Za-z_][A-Za-z0-9_]*):\s*$")
REGISTER_RE = re.compile(
    r"(REGISTER_(?:DOCUMENT_SOURCE|STABLE_EXPRESSION|EXPRESSION|ACCUMULATOR|WINDOW_FUNCTION)|"
    r"MONGO_REGISTER_COMMAND|Command::testCommandsEnabled)"
)
DOC_SOURCE_RE = re.compile(r"REGISTER_DOCUMENT_SOURCE(?:_CONDITIONALLY)?\(\s*([^,\s\)]+)")
EXPR_RE = re.compile(r"REGISTER_(?:STABLE_EXPRESSION|EXPRESSION)\(\s*([^,\s\)]+)")
ACC_RE = re.compile(r"REGISTER_(?:ACCUMULATOR|STABLE_ACCUMULATOR)\(\s*([^,\s\)]+)")
BASIC_COMMAND_CTOR_RE = re.compile(r"(?:BasicCommand|Command)\(\s*\"([A-Za-z0-9_.$-]+)\"")


def git(args: list[str]) -> str:
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True, errors="replace")


def ls(ref: str, *paths: str) -> list[str]:
    try:
        out = git(["ls-tree", "-r", "--name-only", ref, *paths])
    except subprocess.CalledProcessError:
        return []
    return [line for line in out.splitlines() if line]


def show(ref: str, path: str) -> str:
    try:
        return git(["show", f"{ref}:{path}"])
    except subprocess.CalledProcessError:
        return ""


def grep(ref: str, pattern: str, *paths: str) -> list[str]:
    try:
        out = git(["grep", "-n", "-E", pattern, ref, "--", *paths])
    except subprocess.CalledProcessError:
        return []
    return out.splitlines()


def extract_idl_commands(ref: str) -> dict[str, dict[str, object]]:
    commands: dict[str, dict[str, object]] = {}
    for path in ls(ref, "src/mongo"):
        if not path.endswith(".idl"):
            continue
        text = show(ref, path)
        lines = text.splitlines()
        for i, line in enumerate(lines):
            m = IDL_COMMAND_RE.match(line)
            if not m:
                continue
            name = m.group(1).strip()
            fields: list[str] = []
            for j in range(i + 1, min(i + 180, len(lines))):
                if IDL_COMMAND_RE.match(lines[j]):
                    break
                fm = FIELD_RE.match(lines[j])
                if fm and fm.group(1) not in {"description", "type", "optional", "default"}:
                    fields.append(fm.group(1))
            commands[name] = {"path": path, "line": i + 1, "fields": sorted(set(fields))}
    return commands


def extract_basic_command_names(ref: str) -> dict[str, list[str]]:
    found: dict[str, list[str]] = defaultdict(list)
    for path in ls(ref, "src/mongo/db", "src/mongo/s"):
        if not path.endswith((".cpp", ".h")):
            continue
        text = show(ref, path)
        for i, line in enumerate(text.splitlines(), start=1):
            for m in BASIC_COMMAND_CTOR_RE.finditer(line):
                found[m.group(1)].append(f"{path}:{i}")
    return dict(sorted(found.items()))


def extract_registered_tokens(ref: str) -> dict[str, list[str]]:
    tokens = {
        "aggregation_stages": set(),
        "aggregation_expressions": set(),
        "accumulators": set(),
        "feature_flags": set(),
        "index_type_tokens": set(),
    }
    for line in grep(ref, "REGISTER_DOCUMENT_SOURCE|REGISTER_STABLE_EXPRESSION|REGISTER_EXPRESSION|REGISTER_ACCUMULATOR|feature_flags:", "src/mongo"):
        text = line.split(":", 3)[-1]
        for m in DOC_SOURCE_RE.finditer(text):
            tokens["aggregation_stages"].add(m.group(1).strip('"'))
        for m in EXPR_RE.finditer(text):
            tokens["aggregation_expressions"].add(m.group(1).strip('"'))
        for m in ACC_RE.finditer(text):
            tokens["accumulators"].add(m.group(1).strip('"'))
    for line in grep(ref, "MONGO_.*FEATURE_FLAG|feature_flag|feature_flags:", "src/mongo"):
        for name in re.findall(r"featureFlag[A-Za-z0-9_]+|gFeatureFlag[A-Za-z0-9_]+", line):
            tokens["feature_flags"].add(name)
    for line in grep(ref, "INDEX_NAME|INDEX_.*NAME|columnstore|wildcard|2dsphere|hashed|text", "src/mongo/db/index|src/mongo/db/catalog|src/mongo/db/query|src/mongo/client"):
        for tok in re.findall(r'"(2d|2dsphere|geoHaystack|hashed|text|wildcard|columnstore|btree)"', line):
            tokens["index_type_tokens"].add(tok)
    return {k: sorted(v) for k, v in tokens.items()}


def extract_selected_idl(ref: str, paths: list[str]) -> dict[str, str]:
    selected = {}
    for path in paths:
        text = show(ref, path)
        if text:
            selected[path] = text
    return selected


def markdown_table(rows: list[list[str]], headers: list[str]) -> str:
    def esc(s: str) -> str:
        return s.replace("|", "\\|").replace("\n", " ")

    out = ["| " + " | ".join(headers) + " |", "| " + " | ".join(["---"] * len(headers)) + " |"]
    out.extend("| " + " | ".join(esc(cell) for cell in row) + " |" for row in rows)
    return "\n".join(out) + "\n"


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    all_data = {}
    for version, ref in REFS.items():
        idl_commands = extract_idl_commands(ref)
        basic_commands = extract_basic_command_names(ref)
        tokens = extract_registered_tokens(ref)
        commands = sorted(set(idl_commands) | set(basic_commands))
        data_relevant = [c for c in commands if c in DATA_RELEVANT_COMMANDS]
        all_data[version] = {
            "ref": ref,
            "commands": commands,
            "data_relevant_commands": data_relevant,
            "idl_commands": idl_commands,
            "basic_commands": basic_commands,
            "tokens": tokens,
        }

        rows = []
        for cmd in data_relevant:
            source = "idl" if cmd in idl_commands else "basic/manual"
            path = idl_commands.get(cmd, {}).get("path", "; ".join(basic_commands.get(cmd, [])[:2]))
            fields = ", ".join(idl_commands.get(cmd, {}).get("fields", []))
            rows.append([cmd, source, str(path), fields])
        (OUT / f"{version}-data-commands.md").write_text(
            f"# {version} data-relevant commands\n\n"
            + markdown_table(rows, ["Command", "Parser", "Source", "Top-level fields observed in IDL"]),
            encoding="utf-8",
        )

        for name, values in tokens.items():
            (OUT / f"{version}-{name}.txt").write_text("\n".join(values) + "\n", encoding="utf-8")

    (OUT / "api-surface.json").write_text(json.dumps(all_data, indent=2, sort_keys=True), encoding="utf-8")

    versions = list(REFS)
    diff_lines = ["# Version command deltas\n"]
    for prev, cur in zip(versions, versions[1:]):
        p = set(all_data[prev]["commands"])
        c = set(all_data[cur]["commands"])
        diff_lines.append(f"\n## {prev} -> {cur}\n")
        diff_lines.append("### Added commands\n")
        diff_lines.extend(f"- `{name}`" for name in sorted(c - p))
        diff_lines.append("\n### Removed commands\n")
        diff_lines.extend(f"- `{name}`" for name in sorted(p - c))
        dp = set(all_data[prev]["data_relevant_commands"])
        dc = set(all_data[cur]["data_relevant_commands"])
        diff_lines.append("\n### Added data-relevant commands\n")
        diff_lines.extend(f"- `{name}`" for name in sorted(dc - dp))
        diff_lines.append("\n### Removed data-relevant commands\n")
        diff_lines.extend(f"- `{name}`" for name in sorted(dp - dc))
    (OUT / "command-deltas.md").write_text("\n".join(diff_lines) + "\n", encoding="utf-8")

    matrix_rows = []
    data_cmds = sorted(set().union(*(set(all_data[v]["data_relevant_commands"]) for v in versions)))
    for cmd in data_cmds:
        matrix_rows.append([cmd, *["yes" if cmd in all_data[v]["data_relevant_commands"] else "" for v in versions]])
    (OUT / "data-command-matrix.md").write_text(
        "# Data command matrix\n\n" + markdown_table(matrix_rows, ["Command", *versions]),
        encoding="utf-8",
    )


if __name__ == "__main__":
    main()
