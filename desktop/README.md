# Umicom Desk in a Linux graphical session

Sammy Hegab, Umicom Foundation. MIT.

This directory composes Framework-owned tools for an **already established**
Linux Wayland or local X11 session. It is not a compositor or a replacement
login manager. It does not change the diskless recovery profile from Batch 19.

Build with an explicit reviewed Framework checkout:

```sh
cmake -S /absolute/path/umicomOS/desktop -B "$HOME/umicom-builds/desktop-session" \
  -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/absolute/path/Umicom-Applications/framework
cmake --build "$HOME/umicom-builds/desktop-session" --parallel 2
ctest --test-dir "$HOME/umicom-builds/desktop-session" --no-tests=error --output-on-failure
```

Use `bin/umicom-desk-session --inspect --desk /absolute/path/to/umicom-desk`
as a normal user in a real graphical session. Inspection does not launch Desk.
Use `--run` only when ready to replace this launcher process with Desk.
Ordinary application startup and child-app management remain Desk's job.
Closing Desk returns to the host session; it does not log out the host user.

`tools/desktop_session.py --framework <absolute-framework-path> stage ...`
delegates packaging to Framework. It writes one new review directory, not an
autostart entry. Its manifest detects changes, but is neither signed nor an
execution authorisation token. Follow `docs/desk-system-centre.html` for the
complete workflow and current limitations.
