      PROGRAM S2NUM
C     Standalone numeric worker avoids mixed Watcom/Microsoft ABI.
      INTEGER N,I,IOS
      DOUBLE PRECISION X(2048),S,M,V
      READ(5,*,IOSTAT=IOS) N
      IF (IOS.NE.0) STOP 1
      IF (N.LT.0.OR.N.GT.2048) STOP 2
      DO 10 I=1,N
        READ(5,*,IOSTAT=IOS) X(I)
        IF (IOS.NE.0) STOP 3
   10 CONTINUE
      CALL S2STAT(X,N,S,M,V)
      WRITE(6,100) S,M,V
  100 FORMAT(3(1X,E25.16E3))
      END
