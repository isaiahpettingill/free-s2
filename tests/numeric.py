import math
import pathlib
import statistics
import subprocess
import sys

exe = str(pathlib.Path(sys.argv[1]).resolve())
for x in ([1., 2., 3., 4.], [1e9+1, 1e9+2, 1e9+3], [-4., 2., 9.], [7.], []):
    r = subprocess.run([exe], input=str(len(x))+'\n'+''.join(f'{v:.17g}\n' for v in x), capture_output=True, text=True, timeout=10, check=True)
    actual = [float(v.replace('D', 'E')) for v in r.stdout.split()]
    expected = [math.fsum(x), statistics.mean(x) if x else 0., statistics.variance(x) if len(x)>1 else 0.]
    assert len(actual) == 3
    assert all(math.isclose(a, e, rel_tol=1e-12, abs_tol=1e-12) for a, e in zip(actual,expected)), (x,actual,expected)
print('5 independent numeric worker cases passed')
