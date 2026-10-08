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
lines += ['echo S2ALLDONE']
(b / 'TESTDOS.BAT').write_bytes(('\r\n'.join(lines) + '\r\n').encode('ascii'))
