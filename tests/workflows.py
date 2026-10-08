"""End-to-end data, fitting, serialization and device checks."""
from pathlib import Path
import subprocess, sys, tempfile, shutil, re, xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[1]
exe=str(Path(sys.argv[1]).resolve())
def run(src,cwd):
 r=subprocess.run([exe,'-e',src],cwd=cwd,text=True,capture_output=True,timeout=20)
 assert r.returncode==0,r.stderr
 return r.stdout
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp)
 for f in (root/'examples').iterdir(): shutil.copy(f,p/f.name)
 assert run((p/'HELLO.S').read_text(),p)=='[1] 5\n[1] 6 8\n[1] -3 -1 1 3\n'
 r=subprocess.run([exe,'ANALYZE.S'],cwd=p,text=True,capture_output=True,timeout=30)
 assert r.returncode==0,r.stderr
 assert '[1] 7\n' in r.stdout and '[1] 10\n' in r.stdout and '[1] 1 2\n' in r.stdout,r.stdout
 assert ET.parse(p/'FIT.SVG').getroot().tag.endswith('svg')
 assert len(ET.parse(p/'FIT.SVG').getroot())>10
 ps=(p/'FIT.PS').read_text();assert ps.startswith('%!PS') and 'showpage' in ps and '%%EOF' in ps
 assert [float(x) for x in (p/'RESID.TXT').read_text().split()]==[0]*5 or all(abs(float(x))<1e-10 for x in (p/'RESID.TXT').read_text().split())
 assert run('restore("WORK.S");as.vector(fit$coef);d$region',p)=='[1] 1 2\n[1] "north" "south" "north" "south" "north"\n'
 assert run('f<-function(x,y=3)x+y;dump("f","FUN.S");rm("f");restore("FUN.S");f(2)',p)=='[1] 5\n'
 assert run('x<-structure(c(a=1,b=2),class="tag");dput(x,"OBJ.S");z<-dget("OBJ.S");names(z);class(z)',p)=='[1] "a" "b"\n[1] "tag"\n'
 r=subprocess.run([exe,'RANDOM.S'],cwd=p,text=True,capture_output=True,timeout=30);assert r.returncode==0,r.stderr
 for f in ['HIST.SVG','QQ.SVG']:ET.parse(p/f)
 assert run('as.vector(outer(c("a","b"),1:2,paste,sep=""))',p)=='[1] "a1" "b1" "a2" "b2"\n'
 blocks=re.findall(r'```s\n(.*?)```',(root/'docs/guide.md').read_text(),re.S)
 run('\n'.join(blocks),p)
print('Guide examples, data analysis, two graphics devices, simulation, and three serialization round trips passed')
