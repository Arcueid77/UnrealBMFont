# Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

from __future__ import annotations

import json
import re
import struct
import subprocess
import sys
from pathlib import Path
from urllib.parse import unquote, urlsplit


ROOT = Path(__file__).resolve().parent.parent
TEXT_SUFFIXES = {
    ".cpp",
    ".cs",
    ".h",
    ".ini",
    ".json",
    ".md",
    ".ps1",
    ".py",
    ".yml",
}
IGNORED_PARTS = {
    ".build",
    ".codegraph",
    ".git",
    ".idea",
    ".vs",
    ".vscode",
    "Artifacts",
    "Binaries",
    "DerivedDataCache",
    "Intermediate",
    "Saved",
    "__pycache__",
}
REQUIRED_FILES = {
    ".editorconfig",
    ".gitattributes",
    ".github/workflows/ci.yml",
    ".gitignore",
    "LICENSE",
    "LICENSE.zh-CN.md",
    "README.md",
    "README.zh-CN.md",
    "CHANGELOG.md",
    "CODE_OF_CONDUCT.md",
    "CONTRIBUTING.md",
    "ROADMAP.md",
    "SECURITY.md",
    "SUPPORT.md",
    "THIRD_PARTY_NOTICES.md",
    "Config/FilterPlugin.ini",
    "Docs/Architecture.md",
    "Docs/API.md",
    "Docs/Upgrading.md",
    "Docs/Compatibility.md",
    "Docs/FormatSupport.md",
    "Docs/ReleaseChecklist.md",
    "Docs/RichText.md",
    "Docs/Testing.md",
    "Docs/Images/showcase-runtime.png",
    "Resources/Icon128.png",
    "Samples/GroundTruth/PackedAtlas.png",
    "Samples/GroundTruth/PlainGlyphs.png",
    "Samples/GroundTruth/RichTextEllipsis.png",
    "Samples/GroundTruth/TintShadowWrap.png",
    "Samples/Minimal/Minimal.fnt",
    "Samples/Minimal/Minimal.png",
    "Samples/Packed/Packed.fnt",
    "Samples/Packed/Packed.png",
    "Samples/Showcase/Showcase.fnt",
    "Samples/Showcase/Showcase.png",
    "Scripts/BuildPlugin.ps1",
    "Scripts/TestPackagedRuntime.ps1",
    "Scripts/TestPlugin.ps1",
    "Scripts/TestRelease.ps1",
    "Scripts/CreateExampleProject.ps1",
    "Scripts/GenerateMultiPageFixtures.py",
    "Scripts/PrepareExampleAssets.py",
    "Scripts/TestAssetUpgrade.py",
    "Scripts/TestScriptGuards.ps1",
    ".github/workflows/release-validation.yml",
    "Examples/Showcase/BMFontExample.uproject",
    "Samples/MultiPage/MultiPage.fnt",
    "Samples/MultiPage/Page0.png",
    "Samples/MultiPage/Page1.png",
    "UnrealBMFont.uplugin",
}
INLINE_BILINGUAL_DOCUMENTS = {".github/PULL_REQUEST_TEMPLATE.md"}
LOCAL_EVIDENCE_NAMES = {
    "MANUAL_ACCEPTANCE.md",
    "VALIDATION_SUMMARY.md",
    "ReleaseReadiness.md",
    "release-evidence.json",
    "distribution-evidence.json",
    "image-integrity-evidence.json",
    "source-archive-manifest.json",
}
LOCAL_EVIDENCE_NAMES_CASEFOLD = {name.casefold() for name in LOCAL_EVIDENCE_NAMES}


def fail(errors: list[str], message: str) -> None:
    errors.append(message)


def read_text_if_file(errors: list[str], relative_path: str) -> str | None:
    path = ROOT / relative_path
    if not path.is_file():
        return None
    try:
        return path.read_text(encoding="utf-8")
    except (OSError, UnicodeError) as error:
        fail(errors, f"{relative_path}: cannot read as UTF-8 ({error})")
        return None


def png_dimensions(path: Path) -> tuple[int, int]:
    data = path.read_bytes()
    if len(data) < 24 or data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        raise ValueError("not a PNG with an IHDR header")
    return struct.unpack(">II", data[16:24])


def iter_repository_files() -> list[Path]:
    return [
        path
        for path in ROOT.rglob("*")
        if path.is_file() and not any(part in IGNORED_PARTS for part in path.relative_to(ROOT).parts)
    ]


def validate_documentation(errors: list[str], files: list[Path]) -> None:
    chinese = re.compile(r"[\u3400-\u9fff]")
    markdown_links = re.compile(r"!?\[[^\]\n]*\]\((<[^>\n]+>|[^)\n]+)\)")
    fenced_blocks = re.compile(r"^```[^\n]*\n.*?^```\s*$", re.MULTILINE | re.DOTALL)
    documents = [path for path in files if path.suffix.lower() == ".md"]
    for path in files:
        relative = path.relative_to(ROOT).as_posix()
        if path.name.casefold() in LOCAL_EVIDENCE_NAMES_CASEFOLD or re.fullmatch(r"CC-.*review.*\.md", path.name, re.IGNORECASE):
            fail(errors, f"{relative}: local acceptance/review evidence must stay outside the repository")
    for path in documents:
        relative = path.relative_to(ROOT).as_posix()
        content = read_text_if_file(errors, relative)
        if content is None:
            continue
        if relative in INLINE_BILINGUAL_DOCUMENTS:
            if chinese.search(content) is None or re.search(r"[A-Za-z]", content) is None:
                fail(errors, f"{relative}: contribution template must contain both English and Chinese")
        else:
            is_chinese = path.name.endswith(".zh-CN.md")
            if is_chinese:
                stem = path.name.removesuffix(".zh-CN.md")
                counterpart = path.with_name("LICENSE" if stem == "LICENSE" else stem + ".md")
                if chinese.search(content) is None:
                    fail(errors, f"{relative}: Chinese document has no Chinese content")
            else:
                counterpart = path.with_name(path.stem + ".zh-CN.md")
            if not counterpart.is_file():
                fail(errors, f"{relative}: missing language counterpart {counterpart.name}")
            if f"]({counterpart.name})" not in content:
                fail(errors, f"{relative}: missing language-switch link to {counterpart.name}")
        for match in markdown_links.finditer(fenced_blocks.sub("", content)):
            target = match.group(1).strip().removeprefix("<").removesuffix(">")
            try:
                parsed = urlsplit(target)
            except ValueError:
                fail(errors, f"{relative}: invalid link {target}")
                continue
            if parsed.scheme or parsed.netloc or not parsed.path:
                continue
            destination = (path.parent / unquote(parsed.path)).resolve()
            if not destination.is_relative_to(ROOT) or not destination.exists():
                fail(errors, f"{relative}: broken or out-of-repository link {target}")


def main() -> int:
    errors: list[str] = []
    for relative_path in sorted(REQUIRED_FILES):
        if not (ROOT / relative_path).is_file():
            fail(errors, f"missing required file: {relative_path}")

    descriptor_path = ROOT / "UnrealBMFont.uplugin"
    try:
        descriptor = json.loads(descriptor_path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        fail(errors, f"invalid plugin descriptor: {error}")
        descriptor = {}

    if descriptor.get("FriendlyName") != "Unreal BMFont":
        fail(errors, "FriendlyName must be 'Unreal BMFont'")
    version = descriptor.get("Version")
    if not isinstance(version, int) or isinstance(version, bool) or version <= 0:
        fail(errors, "Version must be a positive integer")
    version_name = descriptor.get("VersionName")
    if not isinstance(version_name, str) or re.fullmatch(
        r"(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)(?:-[0-9A-Za-z]+(?:[.-][0-9A-Za-z]+)*)?", version_name
    ) is None:
        fail(errors, "VersionName must be a semantic version, optionally with a prerelease suffix")
    if isinstance(version_name, str) and "-" in version_name and descriptor.get("IsBetaVersion") is not True:
        fail(errors, "Prerelease candidates must keep IsBetaVersion=true")
    if descriptor.get("EnabledByDefault") is not False:
        fail(
            errors,
            "EnabledByDefault must be false so content-only projects generate a plugin-aware target",
        )
    if isinstance(version_name, str):
        for readme_name in ("README.md", "README.zh-CN.md"):
            readme_text = read_text_if_file(errors, readme_name)
            if readme_text is not None and f"`{version_name}`" not in readme_text:
                fail(errors, f"{readme_name}: does not identify the descriptor version {version_name}")
        changelog_text = read_text_if_file(errors, "CHANGELOG.md")
        release_heading = re.compile(
            rf"^## \[{re.escape(version_name)}\] - (?:\d{{4}}-\d{{2}}-\d{{2}}|Unreleased)$",
            re.MULTILINE,
        )
        if changelog_text is not None and release_heading.search(changelog_text) is None:
            fail(errors, f"CHANGELOG.md: missing a dated or Unreleased {version_name} section")
    expected_modules = [
        ("UnrealBMFont", "Runtime"),
        ("UnrealBMFontEditor", "Editor"),
        ("UnrealBMFontTests", "Editor"),
    ]
    actual_modules = [
        (module.get("Name"), module.get("Type")) for module in descriptor.get("Modules", [])
    ]
    if actual_modules != expected_modules:
        fail(errors, f"unexpected module topology: {actual_modules!r}")

    example_path = ROOT / "Examples/Showcase/BMFontExample.uproject"
    try:
        example = json.loads(example_path.read_text(encoding="utf-8"))
        example_plugins = {plugin.get("Name"): plugin for plugin in example.get("Plugins", [])}
        for editor_plugin in ("ContentBrowserAssetDataSource", "EngineAssetDefinitions"):
            plugin = example_plugins.get(editor_plugin, {})
            if plugin.get("Enabled") is not True or plugin.get("TargetAllowList") != ["Editor"]:
                fail(errors, f"showcase must enable {editor_plugin} for Editor targets only")
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        fail(errors, f"invalid showcase descriptor: {error}")

    package_filter = read_text_if_file(errors, "Config/FilterPlugin.ini")
    if package_filter is not None:
        package_filter_lines = package_filter.splitlines()
        for required_filter in ("/*.md", "/LICENSE", "/Docs/...", "/Samples/..."):
            if required_filter not in package_filter_lines:
                fail(errors, f"FilterPlugin.ini must retain release file {required_filter}")
        if "/Scripts/..." in package_filter_lines:
            fail(errors, "FilterPlugin.ini must not ship repository-only scripts")

    ci_text = read_text_if_file(errors, ".github/workflows/ci.yml")
    if ci_text is not None and "Scripts/TestPackagedRuntime.ps1" not in ci_text:
        fail(errors, "CI must invoke the packaged runtime smoke script")

    runtime_rules = read_text_if_file(errors, "Source/UnrealBMFont/UnrealBMFont.Build.cs")
    if runtime_rules is not None:
        for editor_dependency in ("UnrealEd", "AssetDefinition", "AssetTools"):
            if f'"{editor_dependency}"' in runtime_rules:
                fail(errors, f"Runtime module depends on Editor module {editor_dependency}")

    source_suffixes = {".cpp", ".cs", ".h", ".ps1", ".py"}
    repository_files = iter_repository_files()
    validate_documentation(errors, repository_files)
    for path in repository_files:
        relative_path = path.relative_to(ROOT).as_posix()
        content: str | None = None
        if path.suffix.lower() in TEXT_SUFFIXES or path.name == "LICENSE":
            try:
                content = path.read_text(encoding="utf-8")
            except (OSError, UnicodeError) as error:
                fail(errors, f"{relative_path}: cannot read as UTF-8 ({error})")
                continue
            if content and not content.endswith("\n"):
                fail(errors, f"{relative_path}: missing final newline")
            for line_number, line in enumerate(content.splitlines(), start=1):
                if line.endswith((" ", "\t")):
                    fail(errors, f"{relative_path}:{line_number}: trailing whitespace")
        if path.suffix.lower() in source_suffixes and content is not None:
            first_line = content.splitlines()[0] if content else ""
            if "Copyright (c) 2026 AvaloNero" not in first_line:
                fail(errors, f"{relative_path}: missing project copyright header")
        if path.name.startswith("AssetTypeActions_"):
            fail(errors, f"AssetTypeActions_ file naming is not allowed: {relative_path}")

    for relative_path, expected in (
        ("Resources/Icon128.png", (128, 128)),
        ("Samples/GroundTruth/PackedAtlas.png", (64, 32)),
        ("Samples/GroundTruth/PlainGlyphs.png", (64, 32)),
        ("Samples/GroundTruth/RichTextEllipsis.png", (24, 32)),
        ("Samples/GroundTruth/TintShadowWrap.png", (64, 48)),
        ("Samples/Minimal/Minimal.png", (64, 16)),
    ):
        try:
            actual = png_dimensions(ROOT / relative_path)
            if actual != expected:
                fail(errors, f"{relative_path}: expected {expected}, got {actual}")
        except (OSError, ValueError) as error:
            fail(errors, f"{relative_path}: {error}")

    fixture_generator = ROOT / "Scripts" / "GenerateMultiPageFixtures.py"
    if fixture_generator.is_file():
        fixture_check = subprocess.run([sys.executable, str(fixture_generator), "--check"],
                                       capture_output=True, text=True, check=False)
        if fixture_check.returncode != 0:
            fail(errors, fixture_check.stderr.strip() or fixture_check.stdout.strip())

    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    print("Repository validation passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
