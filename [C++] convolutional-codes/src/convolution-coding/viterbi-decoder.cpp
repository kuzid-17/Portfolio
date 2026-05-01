#include "convolution-coding.hpp"

std::vector<int> hard_viterbi_decode(const Trellis &T, std::vector<int>& encoded) {
    const int R = T.n;
    const int N = T.numStates;
    size_t length = encoded.size() / R;

    std::vector<int> output(length);

    std::vector<int> state_metric(N, INT_MAX);
    state_metric[0] = 0;
    
    std::vector<std::vector<int>> tb_state(N, std::vector<int>(length));
    std::vector<std::vector<int>> tb_input(N, std::vector<int>(length));

    for (int i = 0; i < length; i++) {
        int received_symbol = 0;

        for (int c = 0; c < R; c++) {
            received_symbol |= (encoded[R * i + c] << (R - c - 1));
        }
        std::vector<int> temp_metric(N, INT_MAX);

        for (int curr_state = 0; curr_state < N; curr_state++) {
            int curr_metric = state_metric[curr_state];

            if (curr_metric == INT_MAX) {
                continue;
            }

            for (int input = 0; input < 2; input++) {
                int next_state = T.nextStates[curr_state][input];
                int output_symbol = T.outputs[curr_state][input];

                int new_metric = curr_metric + __builtin_popcount(received_symbol ^ output_symbol);
                
                if (new_metric < temp_metric[next_state]) {
                    temp_metric[next_state] = new_metric;
                    
                    tb_state[next_state][i] = curr_state;
                    tb_input[next_state][i] = input;
                }
            }
        }

        state_metric = temp_metric;
    }

    int state = 0;
    for (int i = length - 1; i >= 0; i--) {
        output[i] = tb_input[state][i];
        state = tb_state[state][i];
    }
    
    return output;
}

std::vector<int> soft_viterbi_decode(const Trellis &T, std::vector<double>& encoded) {
    const int R = T.n;
    const int N = T.numStates;
    size_t length = encoded.size() / R;

    std::vector<int> output(length);

    std::vector<double> state_metric(N, std::numeric_limits<double>::infinity());
    state_metric[0] = 0.0;

    std::vector<std::vector<int>> tb_state(N, std::vector<int>(length));
    std::vector<std::vector<int>> tb_input(N, std::vector<int>(length));

    for (int i = 0; i < length; i++) {
        std::vector<double> temp_metric(N, std::numeric_limits<double>::infinity());

        for (int curr_state = 0; curr_state < N; curr_state++) {
            double curr_metric = state_metric[curr_state];

            if (curr_metric == std::numeric_limits<double>::infinity()) continue;

            for (int input = 0; input < 2; input++) {
                int next_state = T.nextStates[curr_state][input];
                int output_symbol = T.outputs[curr_state][input];

                double correlation = 0.0;
                for (int c = 0; c < R; c++) {
                    int bit = (output_symbol >> (R - c - 1)) & 1;
                    correlation += (1 - 2 * bit) * encoded[R * i + c];
                }
                double new_metric = curr_metric - correlation;
                
                if (new_metric < temp_metric[next_state]) {
                    temp_metric[next_state] = new_metric;
                    tb_state[next_state][i] = curr_state;
                    tb_input[next_state][i] = input;
                }
            }
        }
    
        state_metric = temp_metric;
    }
    
    int state = 0;
    for (int i = length - 1; i >= 0; i--) {
        output[i] = tb_input[state][i];
        state = tb_state[state][i];
    }
    
    return output;
}
