#include "convolution-coding.hpp"

std::vector<std::vector<int>> hard_parallel_lva(const Trellis &T, std::vector<int>& encoded, int L) {
    const int R = T.n;
    const int N = T.numStates;
    const size_t length = encoded.size() / R;

    std::vector<std::vector<int>> phi(N, std::vector<int>(L, INT_MAX));
    std::vector<std::vector<std::vector<int>>> tb_state(N, std::vector<std::vector<int>>(L, std::vector<int>(length, -1)));
    std::vector<std::vector<std::vector<int>>> tb_rank(N, std::vector<std::vector<int>>(L, std::vector<int>(length, -1)));
    std::vector<std::vector<std::vector<int>>> tb_input(N, std::vector<std::vector<int>>(L, std::vector<int>(length, -1)));

    for (int i = 0; i < length; i++) {
        int received_symbol = 0;
        for (int c = 0; c < R; c++) {
            received_symbol |= (encoded[R * i + c] << (R - c - 1));
        }

        std::vector<std::vector<int>> temp_phi(N, std::vector<int>(L, INT_MAX));
        std::vector<std::vector<int>> temp_tb_state(N, std::vector<int>(L, -1));
        std::vector<std::vector<int>> temp_tb_rank(N, std::vector<int>(L, -1));
        std::vector<std::vector<int>> temp_tb_input(N, std::vector<int>(L, -1));

        for (int curr_state = 0; curr_state < N; curr_state++) {
            for (int input = 0; input < 2; input++) {
                int next_state = T.nextStates[curr_state][input];
                int output_symbol = T.outputs[curr_state][input];
                int branch_metric = __builtin_popcount(received_symbol ^ output_symbol);

                for (int l = 0; l < L; l++) {
                    int curr_metric = phi[curr_state][l];
                    if (curr_metric == INT_MAX) {
                        continue;
                    }
                    int new_metric = curr_metric + branch_metric;

                    for (int k = 0; k < L; k++) {
                        if (new_metric < temp_phi[next_state][k]) {
                            for (int m = L - 1; m > k; m--) {
                                temp_phi[next_state][m] = temp_phi[next_state][m - 1];
                                temp_tb_state[next_state][m] = temp_tb_state[next_state][m - 1];
                                temp_tb_rank[next_state][m] = temp_tb_rank[next_state][m - 1];
                                temp_tb_input[next_state][m] = temp_tb_input[next_state][m - 1];
                            }
                            temp_phi[next_state][k] = new_metric;
                            temp_tb_state[next_state][k] = curr_state;
                            temp_tb_rank[next_state][k] = l;
                            temp_tb_input[next_state][k] = input;

                            break;
                        }
                    }
                }
            }
        }

        phi = temp_phi;

        for (int s = 0; s < N; s++) {
            for (int k = 0; k < L; k++) {
                tb_state[s][k][i] = temp_tb_state[s][k];
                tb_rank[s][k][i] = temp_tb_rank[s][k];
                tb_input[s][k][i] = temp_tb_input[s][k];
            }
        }
    }

    std::vector<std::vector<int>> result(L, std::vector<int>(length));

    for (int k = 0; k < L; k++) {
        int state = 0;
        int rank = k;

        for (int i = length - 1; i >= 0; i--) {
            result[k][i] = tb_input[state][rank][i];

            int prev_state = tb_state[state][rank][i];
            int prev_rank  = tb_rank[state][rank][i];
            state = prev_state;
            rank  = prev_rank;
        }
    }

    return result;
}

std::vector<std::vector<int>> soft_parallel_lva(const Trellis &T, std::vector<double>& encoded, int L) {
    const int R = T.n;
    const int N = T.numStates;
    size_t length = encoded.size() / R;

    std::vector<std::vector<double>> phi(N, std::vector<double>(L, std::numeric_limits<double>::infinity()));
    phi[0][0] = 0.0;

    std::vector<std::vector<std::vector<int>>> tb_state(N, std::vector<std::vector<int>>(L, std::vector<int>(length, -1)));
    std::vector<std::vector<std::vector<int>>> tb_rank(N, std::vector<std::vector<int>>(L, std::vector<int>(length, -1)));
    std::vector<std::vector<std::vector<int>>> tb_input(N, std::vector<std::vector<int>>(L, std::vector<int>(length, -1)));

    for (int i = 0; i < length; i++) {
        std::vector<std::vector<double>> temp_phi(N, std::vector<double>(L, std::numeric_limits<double>::infinity()));
        std::vector<std::vector<int>> temp_tb_state(N, std::vector<int>(L, -1));
        std::vector<std::vector<int>> temp_tb_rank(N, std::vector<int>(L, -1));
        std::vector<std::vector<int>> temp_tb_input(N, std::vector<int>(L, -1));

        for (int curr_state = 0; curr_state < N; curr_state++) {
            for (int input = 0; input < 2; input++) {
                int next_state = T.nextStates[curr_state][input];
                int output_symbol = T.outputs[curr_state][input];

                double correlation = 0.0;
                for (int c = 0; c < R; c++) {
                    int bit = (output_symbol >> (R - c - 1)) & 1;
                    correlation += (1 - 2 * bit) * encoded[R * i + c];
                }
                double branch_metric = -correlation;

                for (int l = 0; l < L; l++) {
                    double curr_metric = phi[curr_state][l];
                    if (curr_metric == std::numeric_limits<double>::infinity()) {
                        continue;
                    }
                    double new_metric = curr_metric + branch_metric;

                    for (int k = 0; k < L; k++) {
                        if (new_metric < temp_phi[next_state][k]) {
                            for (int m = L - 1; m > k; m--) {
                                temp_phi[next_state][m] = temp_phi[next_state][m - 1];
                                temp_tb_state[next_state][m] = temp_tb_state[next_state][m - 1];
                                temp_tb_rank[next_state][m] = temp_tb_rank[next_state][m - 1];
                                temp_tb_input[next_state][m] = temp_tb_input[next_state][m - 1];
                            }
                            temp_phi[next_state][k] = new_metric;
                            temp_tb_state[next_state][k] = curr_state;
                            temp_tb_rank[next_state][k] = l;
                            temp_tb_input[next_state][k] = input;

                            break;
                        }
                    }
                }
            }
        }

        phi = temp_phi;

        for (int s = 0; s < N; s++) {
            for (int k = 0; k < L; k++) {
                tb_state[s][k][i] = temp_tb_state[s][k];
                tb_rank[s][k][i] = temp_tb_rank[s][k];
                tb_input[s][k][i] = temp_tb_input[s][k];
            }
        }
    }

    std::vector<std::vector<int>> result(L, std::vector<int>(length));

    for (int k = 0; k < L; k++) {
        int state = 0;
        int rank = k;

        for (int i = length - 1; i >= 0; i--) {
            result[k][i] = tb_input[state][rank][i];

            int prev_state = tb_state[state][rank][i];
            int prev_rank  = tb_rank[state][rank][i];

            state = prev_state;
            rank  = prev_rank;
        }
    }
    
    return result;
}
