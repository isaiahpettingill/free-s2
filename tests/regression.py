"""Independent exact-fit and rank-deficiency checks for the F77 worker."""
import pathlib, subprocess, sys, tempfile
exe=str(pathlib.Path(sys.argv[1]).resolve())
for x,y,want in [([1,1,1,1,1,1,2,3,4,5],[3,5,7,9,11],[1,2]),([1,1,1,1,0,1,2,3],[4,3,2,1],[4,-1])]:
 n=len(y)
 data='\n'.join(map(str,[f'{n} 2 1e-7',*x,*y]))+'\n'
 r=subprocess.run([exe],input=data,text=True,capture_output=True,check=True)
 values=[float(v.replace('D','E')) for v in r.stdout.split()]
 assert values[0]==0,r.stdout
 assert all(abs(a-b)<1e-10 for a,b in zip(values[1:3],want)),values
 assert all(abs(v)<1e-10 for v in values[3:]),values
r=subprocess.run([exe],input='3 2 1e-7\n1\n1\n1\n1\n1\n1\n1\n2\n3\n',text=True,capture_output=True,check=True)
assert int(r.stdout.split()[0])!=0,r.stdout
print('3 independent QR regression cases passed')
