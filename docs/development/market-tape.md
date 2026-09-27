# Shared market-data observations

The host tools under `tools/market-tape` compose the same Framework source used by
Trader. There is no OS-local market-data engine. The source guide lives at
`framework/docs/learning/market-tape.html`, and the ownership contract is in
`framework/docs/development/market-tape.md`.

A practice source can demonstrate old prices, missing sequences and retention
without enabling financial connections. This is not a live feed or trading
application inside the bootable guest. The OS kernel, initialisation, recovery,
VM launch and physical-media defaults remain unchanged.
