#pragma once

#include "types.hpp"

signal QPSKmod(const codebits& data);
softbits QPSKdemod(const signal& modulated);
