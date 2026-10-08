"""Independent implementation tests; evidence labels are not conformance claims."""
import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
CASES = json.loads((ROOT / 'tests/cases.json').read_text())

def check(case, result):
    out = result.stdout.replace('\r\n', '\n')
    if 'error' in case:
        assert result.returncode != 0 and case['error'] in result.stderr, (case['id'], result)
    else:
        assert result.returncode == 0, (case['id'], result.stderr)
        assert out == case['out'], (case['id'], out, case['out'])

if __name__ == '__main__':
    exe = str(pathlib.Path(sys.argv[1]).resolve())
    for case in CASES:
        r = subprocess.run([exe, '-e', case['source']], text=True, capture_output=True, timeout=10)
        check(case, r)
    # Genuine REPL: continuations and error recovery must preserve workspace.
    r = subprocess.run([exe], input='x <- 6\nmissing.name\nx+\n2\nq()\n', text=True, capture_output=True, timeout=10)
    assert '[1] 8' in r.stdout and 'object not found' in r.stderr, r
    r = subprocess.run([exe], input='x <- 6\n{x <- 99; stop()}\nx\nq()\n', text=True, capture_output=True, timeout=10)
    assert '[1] 6' in r.stdout and '[1] 99' not in r.stdout, r
    print(f'{len(CASES)} language cases + REPL recovery/rollback passed: {exe}')
