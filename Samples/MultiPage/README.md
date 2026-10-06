# Multi-page fixture

`A` exists only on page 0; `B` and the replacement character exist only on page 1.
The corresponding rectangles on the other page are transparent, so always selecting
either page cannot accidentally pass the packaged `AB` rendering probe.

These original, MIT-licensed test patterns are generated deterministically by the
repository-only `Scripts/GenerateMultiPageFixtures.py`. Use `--check` to verify the
committed PNGs without modifying them. The fixtures test import ownership, normal
dependency cooking and both page selections in a single rendered string.
