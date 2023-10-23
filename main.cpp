#include <iostream>
#include <vector>
#include <regex>
#include <string>
#include <algorithm>
#include <map>
#include <fstream>
#include <iomanip>

// Enum to categorize tokens
enum TokenType {
    KEYWORD, IDENTIFIER, STRING_CONSTANT, NUMERIC_CONSTANT, OPERATOR,
    DELIMITER, COMMENT, UNKNOWN
};

// Token structure
struct Token {
    std::string value;
    TokenType type;
};

struct LexemeTableEntry {
    std::string lexeme;
    TokenType type;
};

using LexemeTable = std::vector<LexemeTableEntry>;

// Function declarations
bool isKeyword(const std::string& token);
bool isIdentifier(const std::string& token);
bool isStringConstant(const std::string& token);
bool isNumericConstant(const std::string& token);
bool isOperator(const std::string& token);
bool isDelimiter(const std::string& token);
std::vector<Token> tokenize(const std::string& code);
void analyze(const std::vector<Token>& tokens);
void displayTokensByClass(const std::vector<Token>& tokens);
LexemeTable createLexemeTable(const std::vector<Token>& tokens);
void displayLexemeTable(const LexemeTable& table);

Token determineTokenType(const std::string& value) {
    if (isKeyword(value)) return {value, KEYWORD};
    if (isIdentifier(value)) return {value, IDENTIFIER};
    if (isStringConstant(value)) return {value, STRING_CONSTANT};
    if (isNumericConstant(value)) return {value, NUMERIC_CONSTANT};
    if (isOperator(value)) return {value, OPERATOR};
    if (isDelimiter(value)) return {value, DELIMITER};
    return {value, UNKNOWN};
}

std::string readFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(1);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Main function
int main() {
    std::string code = readFromFile("testFile.txt");
    std::vector<Token> tokens = tokenize(code);
    LexemeTable table = createLexemeTable(tokens);
    displayLexemeTable(table);
    return 0;
}

LexemeTable createLexemeTable(const std::vector<Token>& tokens) {
    LexemeTable table;
    for (const auto& token : tokens) {
        table.push_back({token.value, token.type});
    }
    return table;
}

void displayLexemeTable(const LexemeTable& table) {
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "|   Lexeme   |            Type            |" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    for (const auto& entry : table) {
        std::cout << "| " << std::setw(10) << entry.lexeme << " | " << std::setw(25);
        switch (entry.type) {
            case KEYWORD: std::cout << "KEYWORD"; break;
            case IDENTIFIER: std::cout << "IDENTIFIER"; break;
            case STRING_CONSTANT: std::cout << "STRING_CONSTANT"; break;
            case NUMERIC_CONSTANT: std::cout << "NUMERIC_CONSTANT"; break;
            case OPERATOR: std::cout << "OPERATOR"; break;
            case DELIMITER: std::cout << "DELIMITER"; break;
            case COMMENT: std::cout << "COMMENT"; break;
            case UNKNOWN: std::cout << "UNKNOWN"; break;
        }
        std::cout << " |" << std::endl;
    }
    std::cout << "--------------------------------------------" << std::endl;
}

// Function implementations
bool isKeyword(const std::string& token) {
    // List of JavaScript keywords
    std::vector<std::string> keywords = {"var", "let", "const", "function", "return",
                                         "if", "else", "for", "while", "switch", "case",
                                         "default", "break", "continue", "try", "catch",
                                         "throw", "finally", "async", "await"};

    return std::find(keywords.begin(), keywords.end(), token) != keywords.end();
}

bool isIdentifier(const std::string& token) {
    std::regex identifier_regex("[_a-zA-Z][_a-zA-Z0-9]*");
    return std::regex_match(token, identifier_regex);
}

bool isStringConstant(const std::string& token) {
    std::regex string_regex("\"([^\"]*)\"|'([^']*)'");
    return std::regex_match(token, string_regex);
}

bool isNumericConstant(const std::string& token) {
    std::regex numeric_regex("([0-9]*\\.[0-9]+|[0-9]+)|0x[a-fA-F0-9]+");
    return std::regex_match(token, numeric_regex);
}

bool isOperator(const std::string& token) {
    std::vector<std::string> operators = {"+", "-", "*", "/", "%", "&&", "||", "==",
                                          "!=", "===", "!==", "<", ">", "<=", ">=",
                                          "!", "++", "--", "=", "+=", "-=", "*=",
                                          "/=", "%=", "&=", "|=", "^=", "<<", ">>",
                                          ">>>", "?.", "=>"};
    return std::find(operators.begin(), operators.end(), token) != operators.end();
}

bool isDelimiter(const std::string& token) {
    std::vector<std::string> delimiters = {",", ";", "{", "}", "(", ")", "[", "]"};
    return std::find(delimiters.begin(), delimiters.end(), token) != delimiters.end();
}

std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    std::string buffer;

    for (size_t i = 0; i < input.length(); ++i) {
        char ch = input[i];

        if (std::isspace(ch) || isOperator(std::string(1, ch)) || isDelimiter(std::string(1, ch))) {
            if (!buffer.empty()) {
                tokens.push_back(determineTokenType(buffer));
                buffer.clear();
            }

            if (isOperator(std::string(1, ch)) || isDelimiter(std::string(1, ch))) {
                tokens.push_back(determineTokenType(std::string(1, ch)));
            }

            // Handle multi-character operators like '=='
            if (i < input.length() - 1) {
                std::string potentialDoubleOp = std::string(1, ch) + std::string(1, input[i + 1]);
                if (isOperator(potentialDoubleOp)) {
                    tokens.push_back(determineTokenType(potentialDoubleOp));
                    i++;  // skip next character
                }
            }

            // Handling comments
            if (ch == '/' && i < input.length() - 1) {
                // Single-line comment
                if (input[i + 1] == '/') {
                    size_t commentEnd = input.find("\n", i + 2);
                    tokens.push_back({ input.substr(i, commentEnd - i), TokenType::COMMENT });
                    i = commentEnd;  // skip to the end of the comment
                }
                    // Multi-line comment
                else if (input[i + 1] == '*') {
                    size_t commentEnd = input.find("*/", i + 2);
                    tokens.push_back({ input.substr(i, commentEnd + 2 - i), TokenType::COMMENT });
                    i = commentEnd + 1;  // skip to the end of the comment
                }
            }
        } else {
            buffer += ch;
        }
    }

    // Handle any remaining characters in the buffer
    if (!buffer.empty()) {
        tokens.push_back(determineTokenType(buffer));
    }

    return tokens;
}


void displayTokensByClass(const std::vector<Token>& tokens) {
    std::map<TokenType, std::vector<std::string>> groupedTokens;

    for (const auto& token : tokens) {
        groupedTokens[token.type].push_back(token.value);
    }

    for (const auto& [type, values] : groupedTokens) {
        std::cout << "Type: ";
        switch (type) {
            case KEYWORD: std::cout << "KEYWORD"; break;
            case IDENTIFIER: std::cout << "IDENTIFIER"; break;
            case STRING_CONSTANT: std::cout << "STRING_CONSTANT"; break;
            case NUMERIC_CONSTANT: std::cout << "NUMERIC_CONSTANT"; break;
            case OPERATOR: std::cout << "OPERATOR"; break;
            case DELIMITER: std::cout << "DELIMITER"; break;
            case COMMENT: std::cout << "COMMENT"; break;
            case UNKNOWN: std::cout << "UNKNOWN"; break;
        }
        std::cout << ":\n";

        for (const auto& value : values) {
            std::cout << "  " << value << "\n";
        }
        std::cout << std::endl;
    }
}
