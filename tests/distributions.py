"""Analytic identities independent of the implementation's algorithms."""
import math, pathlib, subprocess, sys
exe=str(pathlib.Path(sys.argv[1]).resolve())
checks=[('pnorm(1)',(1+math.erf(1/math.sqrt(2)))/2),('dnorm(0)',1/math.sqrt(2*math.pi)),('punif(.25)',.25),('pexp(1)',1-math.exp(-1)),('dbinom(2,4,.5)',.375),('pbinom(2,4,.5)',11/16),('dpois(0,1)',math.exp(-1)),('ppois(1,1)',2/math.e),('pt(1,1)',.75),('pchisq(2,2)',1-math.exp(-1)),('pgamma(1,1)',1-math.exp(-1)),('pbeta(.5,2,2)',.5),('pf(1,1,1)',.5),('pcauchy(1)',.75),('plogis(0)',.5),('pweibull(1,1)',1-math.exp(-1)),('qt(.75,1)',1),('qchisq(.5,2)',2*math.log(2)),('qgamma(.5,1)',math.log(2)),('qbeta(.5,2,2)',.5),('qf(.5,1,1)',1),('qbinom(.7,4,.5)',3),('qpois(.7,1)',1)]
for expr,want in checks:
 r=subprocess.run([exe,'-e',expr],capture_output=True,text=True,check=True)
 got=float(r.stdout.split()[-1]);assert abs(got-want)<2e-9*max(1,abs(want)),(expr,got,want)
print(f'{len(checks)} independent distribution checks passed')
