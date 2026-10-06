# 1.0 release readiness

Status: **1.0.0 release preparation; final acceptance in progress**.

The release scope is UE **5.8.0 source build / Win64**. Every claimed engine build
must pass validation; a single source build is sufficient for this scope. Launcher
builds, other UE minors and other platforms require their own evidence before they
are added to the supported matrix. The dated 0.3.0 and 1.0.0-rc.1 reports remain
historical evidence until the final 1.0.0 commit passes its fresh validation run.

## Candidate acceptance

- [x] Runtime/cache lifecycle regression tests pass without warnings on the reference engine.
- [x] Editor, Development and Shipping compile; package audit passes on the reference engine.
- [x] Full headless automation and strict GPU automation pass on the reference engine.
- [x] Real imported plain, packed and multi-page assets survive save/load and normal dependency cooking.
- [x] Packaged Development and Shipping pass all four GPU pixel probes on the reference engine.
- [x] A representative existing beta asset is loaded and resaved in a disposable project copy.
- [ ] Example project is manually inspected; API/upgrade notes are reviewed.
- [x] Independent code review accepts the final 1.0 changes.
- [ ] Release validation runs on a clean final commit with unchanged source fingerprint.

## Local candidate evidence - 2026-09-26

Reference environment: UE **5.8.0 source build, CL 0**, Windows / Win64. These are
development-tree results, not a published release or a Launcher compatibility claim.

| Check | Result |
| --- | --- |
| Plugin compilation and package audit | Editor Development, Game Development and Game Shipping passed; required files and path-leak audit passed |
| Strict headless automation | 40 passed, 0 warning results, 0 failed; screenshot cases skip under NullRHI |
| Separate strict GPU automation | 8 passed, 0 warning results, 0 failed; no NullRHI skips |
| Packaged Development and Shipping | Plain, packed, multi-page and Rich Text probes passed in each configuration; 0 mismatched pixels |
| Showcase capture | Both full 960 x 600 captures inspected; multilingual rows and atlas examples are visible |
| Beta upgrade | A 0.1.0 asset with 135 glyphs was resaved and verified in a fresh Editor process; data matched and the original asset SHA-256 was unchanged |
| Independent review | Read-only CC review and focused follow-up accepted; no outstanding actionable findings |
| Repository/script checks | Repository validator, PowerShell parsing, script guards and whitespace checks passed |

Local evidence is retained outside the source tree: run identifiers are
`Package-1.0-rc.1-final-c`, `Recheck-1.0-rc.1-final-c`,
`Upgrade-1.0-rc.1-c-{resave,verify}` and `CC-1.0-review-{a,b}`.
The recheck report records its input fingerprint, `DirtySource=true`,
`IncrementalHost=true` and `ReleaseReady=false`. It rebuilt the owned example host,
reimported the current fixtures, and created new cook/runtime/automation evidence.
It does not replace a fresh-host run of `TestRelease.ps1` for the claimed engine build on the final
clean commit. Offscreen screenshot inspection also does not replace interactive
Editor/Play and asset-inspector acceptance.

## Infrastructure requirements

The single-engine policy and final 1.0 metadata passed a focused read-only CC review
on 2026-10-07. Ten isolated orchestration scenarios confirmed source-build acceptance
and rejection of dirty final trees, candidate/beta runs, changed commits/inputs and
failed supplied engines; six workflow-configuration cases also passed. These checks
do not replace the full Unreal validation run or interactive Editor acceptance.

The `Release validation` workflow is manually dispatched. Configure a trusted Windows
GPU runner with labels `ue-release` and `gpu`, and `UE_RELEASE_ENGINE_ROOTS` as a
nonempty JSON array of complete engine build roots. One source build is sufficient;
supplying additional roots requires all of them to pass. The script records each
engine's version and installed-build marker without treating that marker as proof
of Launcher provenance. Do not expose this persistent
runner to untrusted PR code. Missing configuration fails before scheduling a runner;
the engine/GPU/package checks cannot be silently skipped.

Local equivalent:

```powershell
./Scripts/TestRelease.ps1 -EngineRoots '<UE 5.8 source build>' -OutputDirectory '<new external directory>'
```

Use `-CandidateOnly` for development-tree iterations. Its
report always records `ReleaseReady=false`, even when every available check passes.
Prerelease versions and descriptors still marked beta also remain non-release evidence;
`AutomatedGatesPassed` records their test outcome separately.
Successful final evidence is necessary, not sufficient: review this checklist and
manual/independent-review evidence before tagging. This workflow never publishes.

## Compatibility follow-up

Validate a second Unreal minor and an official Launcher installation in a later 1.x
release before extending the support matrix. They are follow-up work for broader
compatibility, not prerequisites for the current source-build / Win64 scope.
