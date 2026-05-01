#include "convolution-coding.hpp"

std::vector<int> tail_biting_encode(const Trellis& T, std::vector<int>& input_bits) {
    std::vector<int> output;
    uint32_t state = 0;

    const int memory = T.K - 1;

    size_t end = input_bits.size() - 1;
    for (size_t i = 0; i < memory; i++) {
        state |= (input_bits[end - i] << i);
    }

    for (int u : input_bits) {
        uint32_t out_symbol = T.outputs[state][u];
        uint32_t next_state = T.nextStates[state][u];

        for (int i = T.n - 1; i >= 0; i--) {
            output.push_back((out_symbol >> i) & 1);
        }

        state = next_state;
    }

    return output;
}

std::vector<int> hard_wava_decode(const Trellis &T, std::vector<int>& encoded, int maxIter) {
    const int R = T.n;
    const int N = T.numStates;
    const size_t length = encoded.size() / R;

    std::vector<int> received(length);
    for (int i = 0; i < length; i++) {
        int received_symbol = 0;
        for (int c = 0; c < R; c++) {
            received_symbol |= (encoded[R * i + c] << (R - c - 1));
        }
        received[i] = received_symbol;
    }

    std::vector<std::vector<int>> tb_state(N, std::vector<int>(length));
    std::vector<std::vector<int>> tb_input(N, std::vector<int>(length));

    std::vector<int> best_tb_bits;
    std::vector<int> state_metric(N, 0);

    for (int iter = 0; iter < maxIter; iter++) {
        std::vector<int> temp_metric(N, INT_MAX);

        for (int i = 0; i < length; i++) {
            std::fill(temp_metric.begin(), temp_metric.end(), INT_MAX);

            for (int curr_state = 0; curr_state < N; curr_state++) {
                if (state_metric[curr_state] == INT_MAX) {
                    continue;
                }

                for (int input = 0; input < 2; input++) {
                    int next_state = T.nextStates[curr_state][input];
                    int output_symbol = T.outputs[curr_state][input];

                    int new_metric = state_metric[curr_state] + __builtin_popcount(received[i] ^ output_symbol);

                    if (new_metric < temp_metric[next_state]) {
                        temp_metric[next_state] = new_metric;
                        tb_state[next_state][i] = curr_state;
                        tb_input[next_state][i] = input;
                    }
                }
            }

            state_metric = temp_metric;
        }

        int best_overall = (int)(std::min_element(state_metric.begin(), state_metric.end()) - state_metric.begin());

        int iter_best_tb_metric = INT_MAX;
        std::vector<int> iter_best_tb_bits;

        for (int s = 0; s < N; s++) {
            if (state_metric[s] == INT_MAX) {
                continue;
            }

            int cur = s;
            for (int i = (int)length - 1; i >= 0; i--) {
                cur = tb_state[cur][i];
            }

            if (cur != s) {
                continue;
            }

            if (state_metric[s] < iter_best_tb_metric) {
                iter_best_tb_metric = state_metric[s];

                iter_best_tb_bits.resize(length);
                int st = s;
                for (int i = (int)length - 1; i >= 0; i--) {
                    iter_best_tb_bits[i] = tb_input[st][i];
                    st = tb_state[st][i];
                }
            }
        }

        if (!iter_best_tb_bits.empty()) {
            best_tb_bits = iter_best_tb_bits;
        }

        if (!iter_best_tb_bits.empty() && state_metric[best_overall] == iter_best_tb_metric) {
            break;
        }
    }

    if (!best_tb_bits.empty()) {
        return best_tb_bits;
    }

    int best = (int)(std::min_element(state_metric.begin(), state_metric.end()) - state_metric.begin());
    std::vector<int> output(length);
    for (int i = (int)length - 1; i >= 0; i--) {
        output[i] = tb_input[best][i];
        best = tb_state[best][i];
    }
    
    return output;
}

std::vector<int> soft_wava_decode(const Trellis &T, std::vector<double>& encoded, int maxIter) {
    const int R = T.n;
    const int N = T.numStates;
    const size_t length = encoded.size() / R;

    std::vector<std::vector<int>> tb_state(N, std::vector<int>(length));
    std::vector<std::vector<int>> tb_input(N, std::vector<int>(length));

    std::vector<int> best_tb_bits;
    std::vector<double> state_metric(N, 0.0);

    for (int iter = 0; iter < maxIter; iter++) {
        std::vector<double> temp_metric(N, std::numeric_limits<double>::infinity());

        for (int i = 0; i < length; i++) {
            std::fill(temp_metric.begin(), temp_metric.end(), std::numeric_limits<double>::infinity());

            for (int curr_state = 0; curr_state < N; curr_state++) {
                if (state_metric[curr_state] == std::numeric_limits<double>::infinity()) {
                    continue;
                }

                for (int input = 0; input < 2; input++) {
                    int next_state = T.nextStates[curr_state][input];
                    int output_symbol = T.outputs[curr_state][input];

                    double correlation = 0.0;
                    for (int c = 0; c < R; c++) {
                        int bit = (output_symbol >> (R - c - 1)) & 1;
                        correlation += (1.0 - 2.0 * bit) * encoded[R * i + c];
                    }
                    double new_metric = state_metric[curr_state] - correlation;

                    if (new_metric < temp_metric[next_state]) {
                        temp_metric[next_state] = new_metric;
                        tb_state[next_state][i] = curr_state;
                        tb_input[next_state][i] = input;
                    }
                }
            }

            state_metric = temp_metric;
        }

        int best_overall = (int)(std::min_element(state_metric.begin(), state_metric.end()) - state_metric.begin());

        double iter_best_tb_metric = std::numeric_limits<double>::infinity();
        std::vector<int> iter_best_tb_bits;

        for (int s = 0; s < N; s++) {
            if (state_metric[s] == std::numeric_limits<double>::infinity()) {
                continue;
            }

            int cur = s;
            for (int i = (int)length - 1; i >= 0; i--) {
                cur = tb_state[cur][i];
            }

            if (cur != s) {
                continue;
            }

            if (state_metric[s] < iter_best_tb_metric) {
                iter_best_tb_metric = state_metric[s];

                iter_best_tb_bits.resize(length);
                int st = s;
                for (int i = (int)length - 1; i >= 0; i--) {
                    iter_best_tb_bits[i] = tb_input[st][i];
                    st = tb_state[st][i];
                }
            }
        }

        if (!iter_best_tb_bits.empty()) {
            best_tb_bits = iter_best_tb_bits;
        }

        if (!iter_best_tb_bits.empty() && state_metric[best_overall] == iter_best_tb_metric) {
            break;
        }
    }

    if (!best_tb_bits.empty()) {
        return best_tb_bits;
    }

    int best = (int)(std::min_element(state_metric.begin(), state_metric.end()) - state_metric.begin());
    std::vector<int> output(length);
    for (int i = (int)length - 1; i >= 0; i--) {
        output[i] = tb_input[best][i];
        best = tb_state[best][i];
    }

    return output;
}
