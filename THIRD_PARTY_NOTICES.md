# Third-party notices and copyright

LeaderboardSystem includes original work and adapted third-party source code.
Copyright (C) 2024-2026 Alessandro Volpe applies to the author's original
contributions and modifications, not to the pre-existing third-party code.

The combined project is distributed under the **GNU General Public License,
version 3**, whose complete text is in [`LICENSE`](LICENSE). The original
notices and permissions below remain applicable to their respective upstream
components. The top-level license text has not been altered. In particular,
the upstream GPLv3-or-later grant for BOSTree is preserved independently of any
later-version permissions for new LeaderboardSystem contributions.

## Hashmap: David Leeds

- Original project: <https://github.com/DavidLeeds/hashmap>
- Copyright (c) 2016-2020 David Leeds
- Original license: **MIT License**
- Adapted files: `src/hashmap.c`, `include/hashmap.h`,
  `include/hashmap_base.h`
- Modified for LeaderboardSystem during 2024-2026 by Alessandro Volpe,
  including stabilization, API corrections, and regression testing.

The following reproduces the original MIT license, including its copyright
and permission notice:

```text
MIT License

Copyright (c) 2016-2020 David Leeds

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## BOSTree: Phillip Berndt

- Original project: <https://github.com/phillipberndt/bostree>
- Copyright (c) 2017, Phillip Berndt
- Original license: **GNU GPL version 3 or (at your option) any later version**
- Adapted files: `src/bostree.c`, `include/bostree.h`
- Modified for LeaderboardSystem during 2024-2026 by Alessandro Volpe,
  including AVL metadata, deletion, lifetime management, and tests.

The upstream source files contain Phillip Berndt's copyright and GPL notice.
The source headers in this repository preserve that notice, and `LICENSE`
contains the full terms of GNU GPL version 3.

## MTwister: Evan Sultanik

- Original project: <https://github.com/ESultanik/mtwister>
- Original author: Evan Sultanik
- Original license status: The upstream `README.md` declares the code
  **public domain** ("Public domain. Have fun!").
- Adapted files: `src/mtwister.c`, `include/mtwister.h`
- Modified for LeaderboardSystem during 2024-2026 by Alessandro Volpe,
  including fixed-width types, allocation changes, and correctness fixes.

This statement records the upstream author's public-domain declaration; it
is not a claim of copyright over the original implementation.

## Project-specific work

The remaining application code, tests, build configuration, and documentation
were developed or adapted for LeaderboardSystem. Copyright claims over those
contributions are limited to the material the contributor actually authored.
Modification history and dates are recorded in Git commits and pull requests.

For additional background and original acknowledgements, see `README.md`.
