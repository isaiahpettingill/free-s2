"""Stage DOS 8.3 fixtures; run TESTDOS.BAT through dosbox-agent-tools."""
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
b = ROOT / 'build'
b.mkdir(exist_ok=True)
for p in b.glob('T*.RC'):
    p.unlink()
lines = ['@echo off']
for prefix, exe in [('TC', 'S2'), ('TF', 'S2F')]:
    for i, c in enumerate(json.loads((ROOT / 'tests/cases.json').read_text())):
        stem = f'{prefix}{i:03d}'
        (b / (stem + '.S')).write_text(c['source'])
        lines += [f'{exe} {stem}.S > {stem}.OUT',
                  f'if errorlevel 1 echo 1 > {stem}.RC']
for f in (ROOT / 'examples').iterdir():
    if f.is_file():
        (b / f.name).write_bytes(f.read_bytes().replace(b'\r\n',b'\n').replace(b'\n',b'\r\n'))
for prefix, exe in [('C', 'S2'), ('F', 'S2F')]:
    lines += [f'{exe} ANALYZE.S > {prefix}DATA.OUT',
              f'if errorlevel 1 echo 1 > {prefix}DATA.RC',
              f'copy FIT.SVG {prefix}FIT.SVG', f'copy FIT.PS {prefix}FIT.PS',
              f'{exe} RANDOM.S > {prefix}RNG.OUT',
              f'if errorlevel 1 echo 1 > {prefix}RNG.RC',
              f'copy HIST.SVG {prefix}HIST.SVG', f'copy QQ.SVG {prefix}QQ.SVG']
lines += ['echo S2ALLDONE']
(b / 'TESTDOS.BAT').write_bytes(('\r\n'.join(lines) + '\r\n').encode('ascii'))
