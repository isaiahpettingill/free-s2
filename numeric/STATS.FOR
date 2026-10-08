      SUBROUTINE S2STAT(X,N,S,M,V)
C     Fortran 77; compensated sum and Welford sample variance.
C     Caller checks missing values and N. No compiler extensions.
      INTEGER N,I
      DOUBLE PRECISION X(*),S,M,V,C,T,Y,D,SS
      S=0D0
      M=0D0
      C=0D0
      SS=0D0
      DO 10 I=1,N
        Y=X(I)-C
        T=S+Y
        C=(T-S)-Y
        S=T
        D=X(I)-M
        M=M+D/DBLE(I)
        SS=SS+D*(X(I)-M)
   10 CONTINUE
      V=0D0
      IF (N.GT.1) V=SS/DBLE(N-1)
      RETURN
      END
