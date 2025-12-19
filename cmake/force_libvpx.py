#!/usr/bin/env python3
import sys
from pathlib import Path


def read_defs(path):
    d = {}
    with path.open('r') as f:
        for l in f:
            l = l.strip()
            if l.startswith('#define'):
                parts = l.split()
                if len(parts) >= 3:
                    d[parts[1]] = parts[2]
    return d


def main():
    if len(sys.argv) < 2:
        print('usage: force_libvpx.py <meson-build-dir>', file=sys.stderr)
        return 2
    build_dir = Path(sys.argv[1])
    tmp = build_dir / 'tmpconfig.h'
    cfg = build_dir / 'config_components.h'
    if not tmp.exists():
        print('tmpconfig.h not found; skipping')
        return 0
    if not cfg.exists():
        print('config_components.h not found; skipping')
        return 0

    tdefs = read_defs(tmp)
    need = False
    for key in ('libvpx_vp8_encoder', 'libvpx_vp9_encoder', 'libvpx_vp8_decoder', 'libvpx_vp9_decoder'):
        if tdefs.get(key) == '1':
            need = True
            break

    if not need:
        print('no libvpx features detected in tmpconfig; nothing to do')
        return 0

    # Make backup
    bak = cfg.with_suffix('.h.bak')
    if not bak.exists():
        bak.write_bytes(cfg.read_bytes())

    text = cfg.read_text()

    # Force umbrella
    if 'CONFIG_LIBVPX' not in text:
        text += '\n#define CONFIG_LIBVPX 1\n'

    replacements = {
        'CONFIG_LIBVPX_VP8_ENCODER': '1',
        'CONFIG_LIBVPX_VP9_ENCODER': '1',
        'CONFIG_LIBVPX_VP8_DECODER': '1',
        'CONFIG_LIBVPX_VP9_DECODER': '1',
    }

    for k, v in replacements.items():
        if f'#define {k} ' in text:
            text = text.replace(f'#define {k} 0', f'#define {k} {v}')
        else:
            text += f'#define {k} {v}\n'

    cfg.write_text(text)
    print('patched config_components.h for libvpx')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
