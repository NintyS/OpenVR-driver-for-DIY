#include <iostream>
#include <vector>

// Function will be splition the string into vector of strings when it finds the "|" character
std::vector<std::string> divideString(std::string str) {

    std::vector<std::string> dividedString;
    std::string tempString = "";

    for (int i = 0; i < str.length(); i++) {
        if (str[i] == '|') {
            dividedString.push_back(tempString);
            tempString = "";
        }
        else {
            tempString += str[i];
        }
    }

    return dividedString;

}

// template<class T>
// std::vector<T> ParseJSON(std::string str) {
//     std::vector<T> parsedJSON;

//     // Find JSON values


//     return parsedJSON;
// }

int main() {

    char buffer[1024] = "0|0|0|0|0|0|0|0|0|";
    
    std::vector<std::string> dividedString = divideString(buffer);

    for ( auto i : dividedString) {
        std::cout << i << std::endl;
    }
    
    return 0;
}