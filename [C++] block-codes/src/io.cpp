#include "io.hpp"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

void throwError(const std::string& message) {
    std::cerr << "Error: " << message << "\n";
    std::exit(1);
}

InputData parseInput(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throwError("cannot open file " + filename);
    }

    nlohmann::json input = nlohmann::json::parse(file);
    InputData parameters;

    if (!input.contains("mode")) {
        throwError("missing field \"mode\"");
    }
    if (!input["mode"].is_string()) {
        throwError("field \"mode\" must be a string");
    }
    std::string mode = input["mode"];

    if (!input.contains("num_of_pucch_f2_bits")) {
        throwError("missing field \"num_of_pucch_f2_bits\"");
    }
    if (!input["num_of_pucch_f2_bits"].is_number_integer()) {
        throwError("field \"num_of_pucch_f2_bits\" must be an integer");
    }
    size_t n = input["num_of_pucch_f2_bits"];

    if (n != 2 && n != 4 && n != 6 && n != 8 && n != 11) {
        throwError("\"num_of_pucch_f2_bits\" must be one of: 2, 4, 6, 8, 11");
    }

    if (mode == "coding") {
        if (!input.contains("pucch_f2_bits")) {
            throwError("missing field \"pucch_f2_bits\"");
        }
        if (!input["pucch_f2_bits"].is_array()) {
            throwError("field \"pucch_f2_bits\" must be an array");
        }
        bitvec bits = input["pucch_f2_bits"].get<bitvec>();

        if (bits.size() != n) {
            throwError("\"pucch_f2_bits\" length must match \"num_of_pucch_f2_bits\"");
        }
        parameters.pucch_f2_bits = bits;

    } else if (mode == "decoding") {
        if (!input.contains("qpsk_symbols")) {
            throwError("missing field \"qpsk_symbols\"");
        }
        if (!input["qpsk_symbols"].is_array()) {
            throwError("field \"qpsk_symbols\" must be an array");
        }
        if (input["qpsk_symbols"].size() != 10) {
            throwError("\"qpsk_symbols\" must contain exactly 10 symbols");
        }
        for (size_t i = 0; i < 10; i++) {
            if (!input["qpsk_symbols"][i].is_string()) {
                throwError("each element of \"qpsk_symbols\" must be a string");
            }
            parameters.qpsk_symbols[i] = stringToComplex(input["qpsk_symbols"][i]);
        }

    } else if (mode == "channel simulation") {
        if (!input.contains("iterations")) {
            throwError("missing field \"iterations\"");
        }
        if (!input["iterations"].is_number_integer()) {
            throwError("field \"iterations\" must be an integer");
        }
        parameters.iterations = input["iterations"];

        if (!input.contains("noise_level")) {
            throwError("missing field \"noise_level\"");
        }
        if (!input["noise_level"].is_number()) {
            throwError("field \"noise_level\" must be a float");
        }
        parameters.noise_level = input["noise_level"];

    } else {
        throwError("unknown mode \"" + mode + "\"");
    }

    parameters.mode = mode;
    parameters.num_of_pucch_f2_bits = n;
    return parameters;
}

void writeOutputCoding(const signal& symbols) {
    std::ofstream file("result.json");
    if (!file) {
        throwError("failed to open result.json");
    }
    nlohmann::json output;
    output["mode"] = "coding";

    std::vector<std::string> qpsk_symbols;
    for (const point& p : symbols) {
        qpsk_symbols.push_back(complexToString(p));
    }
    output["qpsk_symbols"] = qpsk_symbols;

    file << output;
}

void writeOutputDecoding(size_t num_bits, const bitvec& bits) {
    std::ofstream file("result.json");
    nlohmann::json output;

    output["mode"] = "decoding";
    output["num_of_pucch_f2_bits"] = num_bits;
    output["pucch_f2_bits"] = bits;

    file << output;
}

void writeOutputSimulation(size_t num_bits, double bler, size_t success, size_t failed) {
    std::ofstream file("result.json");
    nlohmann::json output;

    output["mode"] = "channel simulation";
    output["num_of_pucch_f2_bits"] = num_bits;
    output["bler"] = bler;
    output["success"] = success;
    output["failed"] = failed;

    file << output;
}

std::string complexToString(const point& p) {
    std::ostringstream oss;
    const double real = p.real();
    const double imag = p.imag();

    const char* imag_sign = "";
    if (imag >= 0) {
        imag_sign = "+";
    }
    oss << real << imag_sign << imag << "j";
    return oss.str();
}

point stringToComplex(const std::string& s) {
    double real, imag;
    char sign, j;

    std::stringstream ss(s);
    ss >> real >> sign >> imag >> j;

    if (sign == '-') {
        imag = -imag;
    }
    return point(real, imag);
}
