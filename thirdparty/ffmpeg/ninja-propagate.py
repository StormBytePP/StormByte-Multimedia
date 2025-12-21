#!/usr/bin/env python3
"""
Lightweight portable wrapper to propagate jobserver or -j parallelism
to nested `ninja` invocations when invoked from CMake custom commands.

Usage: ninja-propagate.py /path/to/ninja [ninja-args...]

Behavior:
- If the parent environment provides a GNU Make jobserver token in
  `MAKEFLAGS` ("--jobserver-auth=..." or a "-jN"), forward it to the
  nested `ninja` invocation (prefer `--jobserver-auth=` if present).
- Otherwise, if `JOBS` or `NPROC` environment variables exist, use them
  to pass `-jN` to `ninja`.
- Otherwise invoke `ninja` with the same args (no change).

This is best-effort and intentionally conservative — it never invents
file-descriptor based jobservers, only forwards auth tokens or -j counts
when present.
"""
import os
import re
import sys
import shutil


def parse_makeflags_jobserver(makeflags):
    # Try to find --jobserver-auth=FD,FD
    m = re.search(r'--jobserver-auth=([^\s]+)', makeflags)
    if m:
        return ('auth', m.group(1))
    # Try to find -jN
    m = re.search(r'(^|\s)-j(\d+)(\s|$)', makeflags)
    if m:
        return ('count', m.group(2))
    return (None, None)


def main(argv):
    if len(argv) < 2:
        print('Usage: ninja-propagate.py /path/to/ninja [args...]', file=sys.stderr)
        return 2

    ninja = argv[1]
    ninja_args = argv[2:]

    # Use absolute path to ninja if available
    ninja_path = shutil.which(ninja) or ninja

    extra = []

    makeflags = os.environ.get('MAKEFLAGS', '')
    kind, val = parse_makeflags_jobserver(makeflags)
    if kind == 'auth':
        extra.append('--jobserver-auth=%s' % val)
    elif kind == 'count':
        extra.append('-j%s' % val)
    else:
        # fallback to common env vars used by CI or wrappers
        jobs = os.environ.get('JOBS') or os.environ.get('NUM_JOBS') or os.environ.get('NPROC')
        if jobs and jobs.isdigit():
            extra.append('-j%s' % jobs)

    # If we still don't have a job count, walk up the ancestor chain and try
    # to find a `ninja` process (handles wrapper being invoked from a shell).
    if not extra:
        try:
            cur = os.getppid()
            hops = 0
            while cur and cur != 1 and hops < 12:
                proc_cmdline = '/proc/%d/cmdline' % cur
                if os.path.exists(proc_cmdline):
                    with open(proc_cmdline, 'rb') as f:
                        data = f.read()
                        parts = [p.decode('utf-8') for p in data.split(b'\0') if p]
                    if parts:
                        exe = parts[0]
                        # detect a ninja executable name
                        if exe.endswith('/ninja') or os.path.basename(exe) == 'ninja' or 'ninja' in os.path.basename(exe):
                            # Look for -jN, -j N, or --jobs=N patterns
                            for i, part in enumerate(parts):
                                m = re.match(r'-j(\d+)$', part)
                                if m:
                                    extra.append('-j%s' % m.group(1))
                                    break
                                if part == '-j' and i + 1 < len(parts) and parts[i + 1].isdigit():
                                    extra.append('-j%s' % parts[i + 1])
                                    break
                                m2 = re.match(r'--jobs=(\d+)$', part)
                                if m2:
                                    extra.append('-j%s' % m2.group(1))
                                    break
                            if extra:
                                break
                # climb to parent pid
                status_file = '/proc/%d/status' % cur
                if os.path.exists(status_file):
                    with open(status_file, 'r') as sf:
                        for line in sf:
                            if line.startswith('PPid:'):
                                try:
                                    cur = int(line.split()[1])
                                except Exception:
                                    cur = 0
                                break
                else:
                    break
                hops += 1
        except Exception:
            # Be conservative: if anything goes wrong, do not modify args.
            pass
    cmd = [ninja_path] + extra + ninja_args

    # Execute (replace) so jobserver fds, etc. are preserved if any.
    os.execvp(cmd[0], cmd)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
