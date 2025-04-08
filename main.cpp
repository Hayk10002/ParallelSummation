#include <iostream>
#include <random>

int generateRandomNumber(int min = 1, int max = 100) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(min, max);
    return dis(gen);
}

int main(int argc, char* argv[])
{
    
    return 0;
}
