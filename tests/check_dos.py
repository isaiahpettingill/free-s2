"""Verify host-visible output/errorlevel from the DOS batch, not R results."""
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
cases = json.loads((ROOT / 'tests/cases.json').read_text())
for prefix in ['TC', 'TF']:
    for i, c in enumerate(cases):
        stem = ROOT / 'build' / f'{prefix}{i:03d}'
        out = stem.with_suffix('.OUT').read_text()
        rc = stem.with_suffix('.RC')
        failed = rc.exists() and rc.read_text().strip() == '1'
        if 'error' in c:
            assert failed, (prefix, c['id'], out)
        else:
            assert not failed and out == c['out'], (prefix, c['id'], out, c['out'])
    print(f'{len(cases)} DOS language cases passed: {prefix}')
