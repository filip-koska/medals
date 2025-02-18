#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <algorithm>
#include <numeric>
#include <regex>
#include <cstdint>
#include <cstddef>

using llong = int64_t;
using ullong = uint64_t;

constexpr size_t MEDALS_NUM = 3;

// sort in descending order by sum of weighted medals
// and ascending order by country name lexicographically
bool cmp(const std::pair<std::array<ullong, MEDALS_NUM>, std::string> &A, 
            const std::pair<std::array<ullong, MEDALS_NUM>, std::string> &B) {
    ullong sum_A = std::accumulate(A.first.begin(), A.first.end(), 0ULL);
    ullong sum_B = std::accumulate(B.first.begin(), B.first.end(), 0ULL);
    if (sum_A != sum_B)
        return std::greater<ullong>()(sum_A, sum_B);
    return std::lexicographical_compare(A.second.begin(), A.second.end(), 
                                    B.second.begin(), B.second.end());
}

void award_medal(const std::string &country, size_t medal_type,
            std::unordered_map<std::string, 
                                std::array<ullong, MEDALS_NUM>> &medals) {
    std::array<ullong, MEDALS_NUM> new_record;
    std::fill(new_record.begin(), new_record.end(), 0ULL);
    // does nothing if the country is already in the database
    medals.emplace(country, new_record);
    if (medal_type > 0)
        medals.find(country)->second[medal_type - 1]++;
}

// returns false on failure of revoke attempt, otherwise returns true
bool revoke_medal(const std::string &country, size_t medal_type,
            std::unordered_map<std::string, std::array<ullong, 3>> &medals) {
    // country is not in the database
    if (medals.find(country) == medals.end())
        return false;

    ullong prev_medals = medals.find(country)->second[medal_type - 1];
    // attempt to revoke a medal that was not awarded
    if (prev_medals == 0)
        return false;
    medals.find(country)->second[medal_type - 1]--;
    return true;
}

void print_medals(const std::array<ullong, MEDALS_NUM> &weights,
            const std::unordered_map<std::string, 
                                    std::array<ullong, MEDALS_NUM>> &medals) {
    if (medals.empty())
        return;

    std::vector<std::pair<std::array<ullong, MEDALS_NUM>, std::string>> ranks;
    for (auto it = medals.begin(); it != medals.end(); ++it) {
        // store standings record as pair of weighted medal scores and country
        std::pair<std::array<ullong, MEDALS_NUM>, 
                    std::string> elem = {it->second, it->first};
        // fill vector with triplets of weighted medal scores
        // to pass correctly to comparator function
        for (size_t i = 0; i < MEDALS_NUM; ++i)
            elem.first[i] *= weights[i];
        ranks.push_back(elem);
    }
    std::sort(ranks.begin(), ranks.end(), cmp);
    // here we assume that the ranks vector is non-empty
    std::cout << "1. " << ranks[0].second << '\n';
    size_t i = 1;
    size_t prev_position = 0;
    ullong prev_result = std::accumulate(ranks[0].first.begin(), 
                                        ranks[0].first.end(), 0ULL);
    for (; i < ranks.size(); ++i) {
        // next country has weighted score strictly smaller than the previous
        if (std::accumulate(ranks[i].first.begin(), 
                                ranks[i].first.end(), 0ULL) < prev_result) {
            prev_position = i;
            prev_result = (ullong)std::accumulate(ranks[i].first.begin(), 
                                                ranks[i].first.end(), 0ULL);
        }
        std::cout << prev_position + 1 << ". " << ranks[i].second << '\n';
    }
}

inline void print_error(size_t line_num) {
    std::cerr << "ERROR " << line_num << '\n';
}

void process_input_line(const std::string &input, const size_t line_num, 
                        std::unordered_map<std::string, 
                            std::array<ullong, MEDALS_NUM>> &medals) {
    static std::regex is_award("[A-Z][a-zA-Z\\ ]*[A-Za-z]\\ [0-3]");
    static std::regex is_revoke("\\-[A-Z][a-zA-Z\\ ]*[a-zA-Z]\\ [1-3]");
    static std::regex is_print("=([1-9]\\d{0,5}\\ ){2}[1-9]\\d{0,5}");
    static std::regex is_number("[1-9]\\d*");

    if (std::regex_match(input, is_award)) {
        // 2 characters reserved for the space and digit at the end
        std::string country = input.substr(0, input.size() - 2);
        size_t medal_type = (size_t)(input[input.size() - 1] - '0');
        award_medal(country, medal_type, medals);
    }
    else if (std::regex_match(input, is_revoke)) {
        // three characters reserved for '-' at the beginning
        // and the space and digit at the end
        std::string country = input.substr(1, input.size() - 3);
        size_t medal_type = (size_t)(input[input.size() - 1] - '0');

        bool valid_medal = revoke_medal(country, medal_type, medals);
        if (!valid_medal)
            print_error(line_num);
    }
    else if (std::regex_match(input, is_print)) {
        size_t i = 0;
        std::array<ullong, MEDALS_NUM> weights;
        ullong tmp = 0;

        auto nums_begin = std::sregex_iterator(input.begin(), 
                                                input.end(), is_number);
        auto nums_end = std::sregex_iterator();
        for (std::sregex_iterator it = nums_begin; it != nums_end; ++it) {
            std::smatch match = *it;
            std::string match_str = match.str();
            ullong weight = std::stoull(match_str, &tmp);
            weights[i++] = weight;
        }
        print_medals(weights, medals);
    }
    else
        print_error(line_num);
}

int main() {
    std::string input;
    // lookup table of medals indexed by country
    std::unordered_map<std::string, std::array<ullong, MEDALS_NUM>> medals;
    size_t line_num = 0;

    while (std::getline(std::cin, input)) {
        ++line_num;
        process_input_line(input, line_num, medals);
    }
}
