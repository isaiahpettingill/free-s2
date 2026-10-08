      SUBROUTINE S2QR(X,Y,N,P,TOL,COEF,RES,STATUS)
C     Independent pivoted, twice-reorthogonalized Gram-Schmidt QR.
C     X is column-major workspace, overwritten by the orthogonal basis.
C     In-place workspace keeps DOS data below one 64K segment.
      INTEGER N,P,STATUS,I,J,K,L,BEST,PASS,IP(32),IT
      DOUBLE PRECISION X(*),Y(N),TOL,COEF(P),RES(N)
      DOUBLE PRECISION R(32,32),B(32),VN(32)
      DOUBLE PRECISION Z,SCALE,T
      STATUS=0
      IF (N*P.GT.2048.OR.P.GT.32.OR.P.GT.N.OR.P.LT.1) THEN
        STATUS=2
        RETURN
      ENDIF
      SCALE=0D0
      DO 20 J=1,P
        IP(J)=J
        VN(J)=0D0
        B(J)=0D0
        DO 10 I=1,N
          VN(J)=VN(J)+X((J-1)*N+I)**2
   10   CONTINUE
        SCALE=DMAX1(SCALE,VN(J))
        DO 15 I=1,P
          R(I,J)=0D0
   15   CONTINUE
   20 CONTINUE
      DO 90 K=1,P
        BEST=K
        DO 30 J=K+1,P
          IF (VN(J).GT.VN(BEST)) BEST=J
   30   CONTINUE
        IF (BEST.NE.K) THEN
          DO 40 I=1,N
            T=X((K-1)*N+I)
            X((K-1)*N+I)=X((BEST-1)*N+I)
            X((BEST-1)*N+I)=T
   40     CONTINUE
          DO 45 I=1,K-1
            T=R(I,K)
            R(I,K)=R(I,BEST)
            R(I,BEST)=T
   45     CONTINUE
          IT=IP(K)
          IP(K)=IP(BEST)
          IP(BEST)=IT
          T=VN(K)
          VN(K)=VN(BEST)
          VN(BEST)=T
        ENDIF
        Z=0D0
        DO 50 I=1,N
          Z=Z+X((K-1)*N+I)**2
   50   CONTINUE
        Z=DSQRT(Z)
        IF (Z.LE.TOL*DSQRT(SCALE)) THEN
          STATUS=1
          RETURN
        ENDIF
        R(K,K)=Z
        DO 55 I=1,N
          X((K-1)*N+I)=X((K-1)*N+I)/Z
          B(K)=B(K)+X((K-1)*N+I)*Y(I)
   55   CONTINUE
        DO 80 J=K+1,P
          DO 70 PASS=1,2
            Z=0D0
            DO 60 I=1,N
              Z=Z+X((K-1)*N+I)*X((J-1)*N+I)
   60       CONTINUE
            R(K,J)=R(K,J)+Z
            DO 65 I=1,N
              X((J-1)*N+I)=X((J-1)*N+I)-Z*X((K-1)*N+I)
   65       CONTINUE
   70     CONTINUE
          VN(J)=0D0
          DO 75 I=1,N
            VN(J)=VN(J)+X((J-1)*N+I)**2
   75     CONTINUE
   80   CONTINUE
   90 CONTINUE
C     Residual is y minus its projection onto the orthonormal columns.
      DO 130 I=1,N
        Z=Y(I)
        DO 120 J=1,P
          Z=Z-X((J-1)*N+I)*B(J)
  120   CONTINUE
        RES(I)=Z
  130 CONTINUE
      DO 110 L=1,P
        K=P+1-L
        DO 100 J=K+1,P
          B(K)=B(K)-R(K,J)*B(J)
  100   CONTINUE
        B(K)=B(K)/R(K,K)
        COEF(IP(K))=B(K)
  110 CONTINUE
      END
