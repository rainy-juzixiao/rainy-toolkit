#!/usr/bin/env python3
"""Generate C++ API reference pages with MuZiYan and wire them into the site.

Each project is described by its own ``sleepy.yaml`` living next to the
library sources. The script runs MuZiYan in ``--partial`` mode, moves the
generated reference tree into ``docs/api/<project>/`` and rewrites the single
fixed entry page ``docs/api/index.md`` that links every generated module.

``docs/api/`` and the intermediate ``misc/docs/`` output are ignored by git:
the script never adds anything to version control. The VitePress config only
mounts the ``/api/`` sidebar when ``docs/api/index.md`` exists, so a plain
``npm run docs:build`` without running this script keeps working.

Usage:
    python3 scripts/ci/gen-api-docs.py
    python3 scripts/ci/gen-api-docs.py --project core
    python3 scripts/ci/gen-api-docs.py --allow-cycles

Set ``MUZIYAN_BIN`` to use a prebuilt MuZiYan binary instead of the
``scripts/muziyan`` launcher (which runs ``cargo run``).
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
BINARY = Path(os.environ.get("MUZIYAN_BIN") or REPO_ROOT / "scripts" / "muziyan")
if not BINARY.is_absolute():
    BINARY = (REPO_ROOT / BINARY).resolve()
SITE_DIR = REPO_ROOT / "docs"
OUT_ROOT = SITE_DIR / "api"
INDEX_NAME = "index.md"

PROJECTS = [
    {
        "id": "core",
        "title": "核心库",
        "config": REPO_ROOT / "tektite" / "library" / "core" / "sleepy.yaml",
    },
]


def parse_output_dir(config_path: Path) -> Path:
    """Read the top-level ``output_dir`` from a sleepy config."""
    for line in config_path.read_text(encoding="utf-8").splitlines():
        match = re.match(r"^output_dir:\s*(.+?)\s*$", line)
        if match:
            raw = match.group(1).strip().strip("'\"")
            return (config_path.parent / raw).resolve()
    raise SystemExit(f"[gen-api-docs] missing output_dir in {config_path}")


def ensure_binary() -> Path:
    if not BINARY.exists():
        raise SystemExit(f"[gen-api-docs] MuZiYan launcher not found at {BINARY}")
    return BINARY


def run_muziyan(config_path: Path, allow_cycles: bool) -> Path:
    output_dir = parse_output_dir(config_path)
    if output_dir.exists():
        shutil.rmtree(output_dir)

    cmd = [
        str(ensure_binary()),
        "-c",
        str(config_path),
        "--partial",
        "--no-graph",
    ]
    if allow_cycles:
        cmd.append("--allow-cycles")

    print(f"[gen-api-docs] {config_path.relative_to(REPO_ROOT)}")
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)
    return output_dir


def move_reference(output_dir: Path, dest: Path) -> int:
    """Move ``<output_dir>/reference/<source>/...`` into ``dest``."""
    reference = output_dir / "reference"
    if not reference.is_dir():
        raise SystemExit(f"[gen-api-docs] no reference pages in {output_dir}")

    if dest.exists():
        shutil.rmtree(dest)
    dest.mkdir(parents=True)

    sources = sorted(p for p in reference.iterdir() if p.is_dir())
    moved = 0
    if len(sources) == 1:
        shutil.copytree(sources[0], dest, dirs_exist_ok=True)
    else:
        for source in sources:
            shutil.copytree(source, dest / source.name, dirs_exist_ok=True)

    for page in dest.rglob("*.md"):
        moved += 1
    return moved


def read_frontmatter(page: Path) -> dict[str, str]:
    fields: dict[str, str] = {}
    text = page.read_text(encoding="utf-8")
    if not text.startswith("---"):
        return fields
    end = text.find("\n---", 3)
    if end == -1:
        return fields
    for line in text[3:end].splitlines():
        key, sep, value = line.partition(":")
        if sep:
            fields[key.strip()] = value.strip().strip("'\"")
    return fields


def common_prefix(groups: list[str]) -> str:
    """Longest shared directory prefix of the generated module groups."""
    parts = [group.split("/") for group in groups]
    prefix: list[str] = []
    for segments in zip(*parts):
        if all(segment == segments[0] for segment in segments):
            prefix.append(segments[0])
        else:
            break
    return "/".join(prefix)


def write_index(projects: list[tuple[str, str, Path]]) -> None:
    """Rewrite the fixed entry page that wires the generated pages together."""
    lines = [
        "---",
        "title: API 参考",
        "---",
        "",
        "# API 参考",
        "",
        "rainy-toolkit 的公开 API，按头文件组织。",
        "",
    ]

    for project_id, title, dest in projects:
        groups: dict[str, list[Path]] = {}
        for page in sorted(dest.rglob("*.md")):
            rel = page.relative_to(dest)
            groups.setdefault(rel.parent.as_posix(), []).append(page)

        if not groups:
            continue

        prefix = common_prefix(list(groups))

        def render(pages: list[Path]) -> str:
            links = []
            for page in pages:
                rel = page.relative_to(dest).with_suffix("")
                link = f"/api/{project_id}/{rel.as_posix()}"
                name = read_frontmatter(page).get("title") or page.stem
                links.append(f"[`{name}`]({link})")
            return " · ".join(links)

        lines.append(f"## {title}")
        lines.append("")

        # Pages sitting directly in the common prefix come first, without a
        # subheading; everything else is grouped by its first path segment.
        root_pages = groups.pop(prefix, [])
        if root_pages:
            lines.append(render(root_pages))
            lines.append("")

        sections: dict[str, list[tuple[str, list[Path]]]] = {}
        for group in sorted(groups):
            relative = group[len(prefix) :].strip("/")
            head, _, rest = relative.partition("/")
            sections.setdefault(head, []).append((rest, groups[group]))

        for head in sorted(sections):
            lines.append(f"### {head}")
            lines.append("")
            for rest, pages in sections[head]:
                if rest:
                    label = " / ".join(rest.split("/"))
                    lines.append(f"**{label}**")
                    lines.append("")
                lines.append(render(pages))
                lines.append("")

    (OUT_ROOT / INDEX_NAME).write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--project",
        action="append",
        choices=[p["id"] for p in PROJECTS],
        help="only generate the given project (repeatable)",
    )
    parser.add_argument(
        "--allow-cycles",
        action="store_true",
        help="pass --allow-cycles to MuZiYan",
    )
    args = parser.parse_args()

    selected = [
        p for p in PROJECTS if not args.project or p["id"] in args.project
    ]

    OUT_ROOT.mkdir(parents=True, exist_ok=True)

    wired: list[tuple[str, str, Path]] = []
    for project in selected:
        config_path: Path = project["config"]
        if not config_path.exists():
            print(
                f"[gen-api-docs] skip {project['id']}: missing {config_path}",
                file=sys.stderr,
            )
            continue

        output_dir = run_muziyan(config_path, args.allow_cycles)
        dest = OUT_ROOT / project["id"]
        moved = move_reference(output_dir, dest)
        shutil.rmtree(output_dir, ignore_errors=True)
        print(f"[gen-api-docs] {project['id']}: {moved} pages -> {dest.relative_to(REPO_ROOT)}")
        wired.append((project["id"], project["title"], dest))

    if not wired:
        raise SystemExit("[gen-api-docs] nothing generated")

    write_index(wired)
    print(f"[gen-api-docs] index -> {(OUT_ROOT / INDEX_NAME).relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
