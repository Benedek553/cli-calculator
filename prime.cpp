#include <iostream>
#include <vector>
#include <cmath>

bool isPrime(int num) {
    if (num <= 1) return false;
    for (int i = 2; i <= std::sqrt(num); i++) {
        if (num % i == 0) return false;
    }
    return true;
}

std::vector<int> getPrimesInRange(int start, int end) {
    std::vector<int> primes;
    for (int num = start; num <= end; num++) {
        if (isPrime(num)) {
            primes.push_back(num);
        }
    }
    return primes;
}

int main() {
    int start, end;

    std::cin >> start;
    std::cin >> end;

    if (std::cin.fail() || start > end) {
        std::cout << "Kerem ervenyes szamokat adjon meg!" << std::endl;
        return 1;
    }

    std::vector<int> result = getPrimesInRange(start, end);
    std::cout << "Primszamok " << start << " es " << end << " kozott: ";
    for (size_t i = 0; i < result.size(); i++) {
        std::cout << result[i];
        if (i < result.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    return 0;
}