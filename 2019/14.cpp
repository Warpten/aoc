#include <cstdint>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <cmath>
#include <deque>
#include <algorithm>
#include <optional>
#include <functional>

using size_t = std::size_t;

const char* input[] = {
"10 ORE => 10 A",
"1 ORE => 1 B",
"7 A, 1 B => 1 C",
"7 A, 1 C => 1 D",
"7 A, 1 D => 1 E",
"7 A, 1 E => 1 FUEL"
};

struct reactant_t {
    std::string name;
    float count;
};

struct reaction_t {
    std::vector<reactant_t> inputs;
    reactant_t output;

    reaction_t(const char* line) {
        const char* start = line;
        const char* end = line;

        bool output_phase = false;
        while (*end != '\0') {
            while (*end != ' ' && *end != '\0') {
                ++end;
            }

            std::string token(start, end);
            if (token == "=>")
            {
                output_phase = true;
                start = end + 1;
                end = start;
            }
            else
            {
                int32_t count = std::atoi(token.c_str());
                if (count == 0)
                    return; // ??

                start = end + 1;
                end = start;
                while (*end != ' ' && *end != '\0') {
                    ++end;
                }

                std::string name(start, end);
                if (name.back() == ',')
                    name.pop_back();

                std::cout << count << " " << name << std::endl;

                if (output_phase) {
                    output.name = name;
                    output.count = count;
                }
                else
                    inputs.push_back({ name, float(count) });

                start = end + 1;
                end = start;
            }
        }

        //for (auto&& input : inputs)
        //   input.count /= output.count;

        //output.count = 1.0f;
    }
};

struct soup_t {
    std::vector<reaction_t> reactions;

    soup_t(const char** in, size_t sz) {
        for (size_t i = 0; i < sz; ++i)
        {
            std::cout << "==============" << std::endl;
            reactions.emplace_back(in[i]);
        }
    }

    std::unordered_map<std::string, int32_t> collect(std::string const& r) {
        std::unordered_map<std::string, int32_t> primes;
        for (auto&& ir : reactions) {
            if (ir.output.name == r)
                for (auto&& iri : ir.inputs)
                    primes[iri.name] += int32_t(std::ceil(float(iri.count) / ir.output.count));
        }
        return primes;
    }

    int32_t get_cost_of(std::string const& product, int32_t productCount, std::string const& base) {
        if (product == base)
            return std::ceil(productCount);

        std::unordered_map<std::string, int32_t> primes = collect(product);

        while (primes.size() != 1) {
            std::unordered_map<std::string, int32_t> adjustedPrimes;
            for (auto&& p : primes) {
                auto children = collect(p.first);

                for (auto&& c : children) {
                    adjustedPrimes[c.first] += c.second * productCount * p.second;
                }
            }

            primes.swap(adjustedPrimes);
        }

        return primes[base];
    }
};

int main() {
    soup_t soup(input, sizeof(input) / (sizeof(const char*)));

    std::cout << soup.get_cost_of("FUEL", 1, "ORE");
    return 0;
}
