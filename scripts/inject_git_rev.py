"""PlatformIO pre-build hook: inject the current git short SHA (with a
``-dirty`` suffix when the working tree has uncommitted changes) as the
``GIT_REV`` build define. Falls back to ``unknown`` outside a git work
tree (e.g. CI building a tarball)."""

import subprocess

Import("env")  # noqa: F821 - provided by PlatformIO


def _git(args):
    try:
        out = subprocess.check_output(
            ["git"] + args,
            stderr=subprocess.DEVNULL,
        )
        return out.decode("utf-8").strip()
    except (subprocess.CalledProcessError, FileNotFoundError):
        return ""


def _git_rev():
    sha = _git(["rev-parse", "--short", "HEAD"])
    if not sha:
        return "unknown"
    dirty = _git(["status", "--porcelain"])
    return sha + ("-dirty" if dirty else "")


rev = _git_rev()
env.Append(CPPDEFINES=[("GIT_REV", env.StringifyMacro(rev))])  # noqa: F821
print("inject_git_rev: GIT_REV=" + rev)
