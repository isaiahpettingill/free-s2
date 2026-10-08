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

import xml.etree.ElementTree as ET
for prefix in ['C','F']:
    for task in ['DATA','RNG']:
        rc=ROOT/'build'/f'{prefix}{task}.RC'
        assert not rc.exists() or rc.read_text().strip()!='1',(prefix,task)
    out=(ROOT/'build'/f'{prefix}DATA.OUT').read_text()
    assert '[1] 7\n' in out and '[1] 10\n' in out and '[1] 1 2\n' in out,out
    for name in ['FIT','HIST','QQ']:
        assert ET.parse(ROOT/'build'/f'{prefix}{name}.SVG').getroot().tag.endswith('svg')
    ps=(ROOT/'build'/f'{prefix}FIT.PS').read_text()
    assert ps.startswith('%!PS') and 'showpage' in ps and '%%EOF' in ps
    print(f'DOS data/regression/simulation and chart export passed: {prefix}')
