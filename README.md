# Aimee module: memory

This is the independent `memory` source-ownership repository.

It builds `aimee-module-memory` as a separate Go process for the
server bus. The exported repository includes the
exact canonical Go bus client/runtime snapshot and its repository-owned handler;
the retained C adapter is a wire-parity fixture, not the production executable.

The daemon admits the process only when its installed absolute executable path,
UID, principal class, principal reference, and event-kind grants match the
installed `.grant` file. Copy that generated grant into each declared daemon
policy directory under `modules.d`.


The descriptor-owned production sources, headers, tests, and documentation are
preserved at their canonical paths so their migration history remains auditable.
