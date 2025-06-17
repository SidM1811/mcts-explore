template <typename DataT, int M, int N>
class ActivationsT {
    public:
    DataT data[M][N];
};

template <typename DataT, int M, int N>
class LinearLayer {
    public:
    DataT weights[N][M];
    DataT bias[N];

    DataT weights_grad[N][M];
    DataT bias_grad[N];

    void forward(DataT input[M], DataT output[N]) {
        matmul<DataT, N, M, M>(weights, input, output);
        add<DataT, N, 1>(output, bias);
    }

    void backward(DataT input[M], DataT grad_in[N], DataT grad_out[M]) {
        matmul<DataT, M, N, N>(weights, grad_in, grad_out);
        add<DataT, N, 1>(grad_in, bias_grad);
        matmul<DataT, N, 1, M>(grad_in, input, weights_grad);
    }
};

template <typename DataT, int K1, int K2>
class Conv2D {
    public:
    DataT weights[K1][K2];
    DataT bias[K1][K2];

    void forward(int M, int N, DataT input[][], DataT output[][]) {
        // read once write many
        for (int i = 0; int i < M - K1; i++) {
            for (int j = 0; j < N - K1; j++) {
                for (int k = 0; k < K1; k++) {
                    for (int l = 0; l < K2; l++) {
                        output[i][j] += input[i + K1 - k][j + K2 - l] * weights[k][l];
                    }
                }
            }
        }
    }

    void backward(int M, int N, DataT input[][], DataT grad_in[][], DataT grad_out[][]) {
        for (int i = 0; int i < M - K1; i++) {
            for (int j = 0; j < N - K1; j++) {
                for (int k = 0; k < K1; k++) {
                    for (int l = 0; l < K2; l++) {
                        grad_out[i + K1 - k][j + K2 - l] += grad_in[i][j] * weights[k][l];
                    }
                }
            }
        }
    }
};

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