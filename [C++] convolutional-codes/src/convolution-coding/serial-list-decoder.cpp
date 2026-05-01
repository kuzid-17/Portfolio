#include "convolution-coding.hpp"
#include <queue>
#include <functional>
#include <tuple>
#include <set>

struct PathInfo {
    std::vector<int> inputs;
    std::vector<int> states;
    double cost;
    int merge_time;
    int parent_index;
};

std::vector<std::vector<int>> soft_serial_lva(const Trellis &T, std::vector<double>& encoded, int L) {
    const int R = T.n;
    const int N = T.numStates;
    const size_t length = encoded.size() / R;
    
    if (L == 0) {
        return {};
    }
    
    std::vector<std::vector<double>> phi_best(N, std::vector<double>(length + 1, std::numeric_limits<double>::infinity()));
    phi_best[0][0] = 0.0;
    
    std::vector<std::vector<int>> tb_state(N, std::vector<int>(length, -1));
    std::vector<std::vector<int>> tb_input(N, std::vector<int>(length, -1));
    
    for (int i = 0; i < (int)length; i++) {
        std::vector<double> temp_metric(N, std::numeric_limits<double>::infinity());
        
        for (int curr_state = 0; curr_state < N; curr_state++) {
            if (phi_best[curr_state][i] == std::numeric_limits<double>::infinity()) {
                continue;
            }
            
            for (int input = 0; input < 2; input++) {
                int next_state = T.nextStates[curr_state][input];
                int output_symbol = T.outputs[curr_state][input];
                
                double correlation = 0.0;
                for (int c = 0; c < R; c++) {
                    int bit = (output_symbol >> (R - c - 1)) & 1;
                    correlation += (1 - 2 * bit) * encoded[R * i + c];
                }
                double new_metric = phi_best[curr_state][i] - correlation;
                
                if (new_metric < temp_metric[next_state]) {
                    temp_metric[next_state] = new_metric;
                    tb_state[next_state][i] = curr_state;
                    tb_input[next_state][i] = input;
                }
            }
        }
        
        for (int s = 0; s < N; s++) phi_best[s][i + 1] = temp_metric[s];
    }
    
    std::vector<int> best_states(length + 1);
    best_states[length] = 0;
    for (int i = (int)length - 1; i >= 0; i--) {
        best_states[i] = tb_state[best_states[i + 1]][i];
    }
    
    std::vector<int> best_inputs(length);
    for (int i = (int)length - 1; i >= 0; i--) {
        best_inputs[i] = tb_input[best_states[i + 1]][i];
    }
    
    std::vector<PathInfo> found_paths;
    found_paths.push_back({best_inputs, best_states, phi_best[0][length], (int)length, -1});
    
    if (L == 1) {
        return {best_inputs};
    }
    
    std::vector<double> cost_after(length + 1, 0.0);
    for (int i = (int)length - 1; i >= 0; i--) {
        int s = best_states[i];
        int s1 = best_states[i + 1];
        int inp = best_inputs[i];
        int out = T.outputs[s][inp];
        
        double correlation = 0.0;
        for (int c = 0; c < R; c++) {
            int bit = (out >> (R - c - 1)) & 1;
            correlation += (1 - 2 * bit) * encoded[R * i + c];
        }
        cost_after[i] = cost_after[i + 1] - correlation;
    }

    auto find_second_best_to_path = [&](const PathInfo& path, int max_time, 
                                        int& out_divergence_time, int& out_prev_state, 
                                        int& out_input, double& out_cost) -> bool {
        double best_alternative_cost = std::numeric_limits<double>::infinity();
        int best_divergence_time = -1;
        int best_prev_state = -1;
        int best_input = -1;
        
        for (int t = 1; t <= max_time; t++) {
            int curr_state = path.states[t];
            int prev_state_in_path = path.states[t - 1];
            int input_in_path = path.inputs[t - 1];

            for (int l = 0; l < N; l++) {
                if (l == prev_state_in_path) {
                    continue;
                }

                if (phi_best[l][t - 1] == std::numeric_limits<double>::infinity()) {
                    continue;
                }
                
                for (int input = 0; input < 2; input++) {
                    if (input == input_in_path && l == prev_state_in_path) {
                        continue;
                    }

                    if (T.nextStates[l][input] != curr_state) {
                        continue;
                    }
                    
                    int out = T.outputs[l][input];
                    double correlation = 0.0;
                    for (int c = 0; c < R; c++) {
                        int bit = (out >> (R - c - 1)) & 1;
                        correlation += (1 - 2 * bit) * encoded[R * (t - 1) + c];
                    }
                    double branch = -correlation;
                    
                    double remaining_cost = 0.0;
                    if (t <= (int)length) {
                        for (int tt = t; tt < (int)length; tt++) {
                            int s = path.states[tt];
                            int s1 = path.states[tt + 1];
                            int inpp = path.inputs[tt];
                            int outt = T.outputs[s][inpp];

                            double corr = 0.0;
                            for (int c = 0; c < R; c++) {
                                int bit = (outt >> (R - c - 1)) & 1;
                                corr += (1 - 2 * bit) * encoded[R * tt + c];
                            }
                            remaining_cost += -corr;
                        }
                    }
                    
                    double total = phi_best[l][t - 1] + branch + remaining_cost;
                    
                    if (total < best_alternative_cost - 1e-9) {
                        best_alternative_cost = total;
                        best_divergence_time = t;
                        best_prev_state = l;
                        best_input = input;
                    }
                }
            }
        }
        
        if (best_divergence_time != -1) {
            out_divergence_time = best_divergence_time;
            out_prev_state = best_prev_state;
            out_input = best_input;
            out_cost = best_alternative_cost;
            return true;
        }
        return false;
    };
    
    using Candidate = std::tuple<double, int, int, int, int>;
    auto cmp = [](const Candidate& a, const Candidate& b) {
        return std::get<0>(a) > std::get<0>(b);
    };
    std::priority_queue<Candidate, std::vector<Candidate>, decltype(cmp)> pq(cmp);
    
    for (int j = 1; j <= (int)length; j++) {
        int best_prev = best_states[j - 1];
        int best_curr = best_states[j];
        
        for (int l = 0; l < N; l++) {
            if (l == best_prev) {
                continue;
            }

            if (phi_best[l][j - 1] == std::numeric_limits<double>::infinity()) {
                continue;
            }
            
            for (int input = 0; input < 2; input++) {
                if (T.nextStates[l][input] != best_curr) {
                    continue;
                }
                
                int out = T.outputs[l][input];
                double correlation = 0.0;
                for (int c = 0; c < R; c++) {
                    int bit = (out >> (R - c - 1)) & 1;
                    correlation += (1 - 2 * bit) * encoded[R * (j - 1) + c];
                }
                double branch = -correlation;
                double total = phi_best[l][j - 1] + branch + cost_after[j];
                
                pq.push({total, 0, j, l, input});
            }
        }
    }
    
    std::vector<std::vector<int>> result(L);
    result[0] = best_inputs;
    
    std::set<std::vector<int>> unique_paths;
    unique_paths.insert(best_inputs);
    
    for (int k = 1; k < L && !pq.empty(); k++) {
        auto [cost, parent_idx, merge_t, pred_state, pred_input] = pq.top();
        pq.pop();
        
        const PathInfo& parent = found_paths[parent_idx];
        
        std::vector<int> path(length);
        
        for (int i = merge_t; i < (int)length; i++) {
            path[i] = parent.inputs[i];
        }
        
        path[merge_t - 1] = pred_input;
        
        {
            int state = pred_state;
            for (int i = merge_t - 2; i >= 0; i--) {
                path[i] = tb_input[state][i];
                state = tb_state[state][i];
            }
        }
        
        if (unique_paths.find(path) != unique_paths.end()) {
            k--;
            continue;
        }
        
        result[k] = path;
        unique_paths.insert(path);
        
        std::vector<int> path_states(length + 1);
        path_states[0] = 0;
        for (int i = 0; i < (int)length; i++) {
            path_states[i + 1] = T.nextStates[path_states[i]][path[i]];
        }
        
        int final_merge = length;
        for (int t = length; t >= 0; t--) {
            if (t <= merge_t && path_states[t] == parent.states[t]) {
                final_merge = t;
                break;
            } 
        }
        
        PathInfo new_path = {path, path_states, cost, final_merge, parent_idx};
        found_paths.push_back(new_path);
        int new_idx = found_paths.size() - 1;
        
        int div_time, alt_prev_state, alt_input;
        double alt_cost;
        
        if (find_second_best_to_path(new_path, final_merge, div_time, alt_prev_state, alt_input, alt_cost)) {
            double total_with_cost = alt_cost;
            pq.push({total_with_cost, new_idx, div_time, alt_prev_state, alt_input});
        }
        
        for (int j = 1; j <= final_merge; j++) {
            int curr_state_in_path = new_path.states[j];
            
            for (int l = 0; l < N; l++) {
                if (l == new_path.states[j - 1]) {
                    continue;
                }

                if (phi_best[l][j - 1] == std::numeric_limits<double>::infinity()) {
                    continue;
                }
                
                for (int input = 0; input < 2; input++) {
                    if (T.nextStates[l][input] != curr_state_in_path) {
                        continue;
                    }

                    if (input == new_path.inputs[j - 1] && l == new_path.states[j - 1]) {
                        continue;
                    }
                    
                    int out = T.outputs[l][input];
                    double correlation = 0.0;
                    for (int c = 0; c < R; c++) {
                        int bit = (out >> (R - c - 1)) & 1;
                        correlation += (1 - 2 * bit) * encoded[R * (j - 1) + c];
                    }
                    double branch = -correlation;
                    
                    double remaining = 0.0;
                    for (int tt = j; tt < (int)length; tt++) {
                        int s = new_path.states[tt];
                        int s1 = new_path.states[tt + 1];
                        int inpp = new_path.inputs[tt];
                        int outt = T.outputs[s][inpp];
                        double corr = 0.0;
                        for (int c = 0; c < R; c++) {
                            int bit = (outt >> (R - c - 1)) & 1;
                            corr += (1 - 2 * bit) * encoded[R * tt + c];
                        }
                        remaining += -corr;
                    }
                    
                    double total = phi_best[l][j - 1] + branch + remaining;
                    pq.push({total, new_idx, j, l, input});
                }
            }
        }
    }
    
    for (int k = (int)result.size(); k < L; k++) {
        result[k] = std::vector<int>(length, 0);
    }
    
    return result;
}
