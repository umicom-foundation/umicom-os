# Umicom desktop workspace

This entry point builds the native workspace tools and tests from the canonical
Framework checkout. It does not compile the complete OS, Desk or a compositor.

The persistent directory must be on storage that survives restart. The existing
diskless foundation image remains RAM-only: this source change does not add a
persistent volume or graphical guest profile to it.

Open `framework/docs/learning/desktop-workspace.html` for the public walkthrough,
and `docs/desktop/persistent-workspace.md` for the integration boundaries.

Sammy Hegab · Umicom Foundation · MIT
