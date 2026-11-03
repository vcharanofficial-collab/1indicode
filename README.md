# indicode
ndiCode is an AI-native programming language built in India. It bridges traditional logic with machine learning, empowering developers and researchers to build high-performance AI and computational biology systems with unmatched efficiency. India’s gift to the global AI era.

## Compiler Front-End Status

As of 2026-11-03, the C++20 compiler front end is estimated at **30% complete** toward a validated typed representation that can later feed an LLVM backend. LLVM is not downloaded, vendored, or built as part of this project. The current implementation includes an initial lexer, parser/AST, core name and type checks, a small inspection CLI, and tests; a separate typed HIR and verifier are not implemented yet.

See [the front-end project](1.Indicode_Complete_frent_end/README.md) and its [detailed progress report](1.Indicode_Complete_frent_end/docs/PROGRESS.md) for the calculation, implemented scope, limitations, and next milestones.
