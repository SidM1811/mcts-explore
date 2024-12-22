template <typename DataT, int M, int N, int K>
void matmul(const DataT A[M][K], const DataT B[K][N], DataT C[M][N]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < K; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

template <typename DataT, int M, int N> // inplace ReLU
void ReLU(DataT A[M][N]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = std::max(static_cast<DataT>(0), A[i][j]);
        }
    }
}

template <typename DataT, int M, int N> 
void add(DataT A[M][N], DataT B[M][N]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] += B[i][j];
        }
    }
}

template <typename DataT, int M1, int M2, int N>
void concat(DataT A[M1][N], DataT B[M2][N], DataT C[M1 + M2][N]) {
    for (int i = 0; i < M1; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = A[i][j];
        }
    }
    for (int i = 0; i < M2; i++) {
        for (int j = 0; j < N; j++) {
            C[i + M1][j] = B[i][j];
        }
    }
}