# Persistent desktop workspaces

This update composes the shared Framework notebook and presentation preferences.
It is one user-space building block for the graphical/persistent desktop roadmap,
not a completed graphical operating-system image.

## Run on an existing desktop

Build the Applications checkout and open Umicom Desk. Its new **Open persistent
desktop workspace** entry opens the shared GTK window. Storage opens only after
**Open workspace**. The current user's application-data path is displayed.

The existing System Centre, saved application groups, taskbar, context strip and
Linux session handoff remain unchanged. There is no automatic execution of a
saved application or stored note.

## Focused native tools

Configure `desktop/workspace` with `UMICOM_OS_FRAMEWORK_SOURCE` pointing to the
updated Framework checkout. This builds the same native services and tests,
without copying the implementation into this repository.

```
cmake -S desktop/workspace -B build/desktop-workspace -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/desktop-workspace --parallel 2
ctest --test-dir build/desktop-workspace --parallel 2 --no-tests=error --output-on-failure
```

## Guest boundary

The existing diskless foundation still has a RAM-only runtime. Installing these
source files does not create persistent user storage, accounts, display-server
support, a graphical login, a bootable desktop image or system update/rollback.
A qualified graphical OS profile must supply a persistent volume and an existing
graphical session before this workspace can preserve user work across guest boots.

Use the public beginner guide at
`framework/docs/learning/desktop-workspace.html`. Keep physical-media writing
disabled while the separate media writer remains unqualified.

Sammy Hegab · Umicom Foundation · MIT

## Review a checkpoint from the OS shell

The Umicom OS desktop shell also exposes **Open persistent desktop workspace**.
It uses Framework's shared workspace service. Opening its window does not open
the database; **Open workspace** is the separate action that opens storage.

1. Open the workspace, write a note and choose **Save checkpoint**.
2. Change the note or appearance and save again. Note the saved revision number.
3. Save or discard any remaining draft, enter an earlier retained checkpoint
   number and choose **Preview checkpoint**.
4. Compare the complete saved notes and preferences on the left and right.
   The right pane is the content that would be restored. Both panes are
   read-only and support selection and copying.
5. Choose **Cancel** to leave the current saved workspace unchanged, or
   **Restore reviewed checkpoint** to save the reviewed content as a new
   revision. Check the result reported in the workspace window.

The review includes note titles and bodies, the selected note, theme, font size
and note-list visibility. Difference navigation is available within the shared
text viewer's alignment limits. Larger content remains visible in full.

If the saved workspace advances or an unsaved draft appears, dismiss the old
review and prepare another. If a checkpoint is no longer among the eight
retained revisions, choose a retained one. A storage failure leaves the last
successfully saved workspace in place. This operation restores this notebook
and its appearance; it does not roll back the operating system or a product's
business transactions.

## Find a retained checkpoint

1. Open the persistent workspace. Its checkpoint list shows the newest retained revisions first, with note counts and the selected note title.
2. Select a usable row to fill the checkpoint number. Selecting does not restore it. The manual number field remains available.
3. Save or discard an unsaved draft, then choose **Preview checkpoint**. Read the complete comparison before choosing **Restore reviewed checkpoint**.
4. Use **Refresh checkpoints** to reread the list without discarding editor text. An unavailable row remains visible; another retained checkpoint may still be usable.

Restoration creates a new saved revision. Framework owns the history reader, worker and restore review; the OS shell supplies the entry point. For the full workflow and troubleshooting, read [Review and restore saved workspaces](../../framework/docs/learning/restore-saved-workspaces.html).
