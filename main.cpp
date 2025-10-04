#include <bits/stdc++.h>
using namespace std;

//ANSI
namespace Color {
    const string RESET  = "\033[0m";
    const string RED    = "\033[31m";
    const string GREEN  = "\033[32m";
    const string YELLOW = "\033[33m";
    const string BLUE   = "\033[34m";
    const string MAGENTA= "\033[35m";
    const string CYAN   = "\033[36m";
    const string GRAY   = "\033[90m";
}

enum class TokenType {
    Number,
    NumberHex,
    NumberFloat,
    StringLiteral,
    Directive,
    Comment,
    Keyword,
    Operator,
    Separator,
    Identifier,
    Unknown,
    EndOfFile,
    Error
};

struct Token {
    TokenType type;
    string lexeme;
    int line;
    int col;
};

static string tokenTypeName(TokenType t) {
    switch(t) {
        case TokenType::Number: return "Number";
        case TokenType::NumberHex: return "NumberHex";
        case TokenType::NumberFloat: return "NumberFloat";
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::Directive: return "Directive";
        case TokenType::Comment: return "Comment";
        case TokenType::Keyword: return "Keyword";
        case TokenType::Operator: return "Operator";
        case TokenType::Separator: return "Separator";
        case TokenType::Identifier: return "Identifier";
        case TokenType::Unknown: return "Unknown";
        case TokenType::EndOfFile: return "EOF";
        case TokenType::Error: return "Error";
        default: return "Other";
    }
}

class Lexer {
    string src;
    size_t pos = 0;
    int line = 1;
    int col = 1;
    vector<Token> tokens;
    unordered_set<string> keywords;

public:
    Lexer(const string &s): src(s) { initKeywords(); }

    vector<Token> tokenize() {
        while (peek() != '\0') {
            skipWhitespace();
            if (peek() == '\0') break;
            int tline = line, tcol = col;

            if (peek() == '/' && peek(1) == '/') {
                string c = scanLineComment();
                tokens.push_back({TokenType::Comment, c, tline, tcol});
            } else if (peek() == '/' && peek(1) == '*') {
                string c = scanBlockComment();
                tokens.push_back({c.empty() ? TokenType::Error : TokenType::Comment, c.empty() ? "Unterminated block comment" : c, tline, tcol});
            } else if (peek() == '#') {
                string d = scanDirective();
                tokens.push_back({TokenType::Directive, d, tline, tcol});
            } else if (peek() == '"') {
                string s = scanStringLiteral();
                tokens.push_back({s.empty() ? TokenType::Error : TokenType::StringLiteral, s.empty() ? "Unterminated string literal" : s, tline, tcol});
            } else if (isDigitStart(peek())) {
                string n = scanNumber();
                TokenType tt = classifyNumber(n);
                tokens.push_back({tt, n, tline, tcol});
            } else if (isIdentifierStart(peek())) {
                string id = scanIdentifier();
                tokens.push_back({keywords.count(id) ? TokenType::Keyword : TokenType::Identifier, id, tline, tcol});
            } else if (isOperatorStart(peek())) {
                string op = scanOperator();
                tokens.push_back({TokenType::Operator, op, tline, tcol});
            } else if (isSeparator(peek())) {
                string s(1, advance());
                tokens.push_back({TokenType::Separator, s, tline, tcol});
            } else {
                string s(1, advance());
                tokens.push_back({TokenType::Unknown, s, tline, tcol});
            }
        }
        tokens.push_back({TokenType::EndOfFile, "", line, col});
        return tokens;
    }

private:
    void initKeywords() {
        vector<string> ks = {
            "let","var","func","class","struct","enum","import","if","else","switch","case","default","for","while","repeat",
            "return","break","continue","guard","defer","do","try","catch","throw","protocol","extension","operator","precedencegroup",
            "inout","deinit","init","final","public","private","internal","open","fileprivate","static","lazy","weak","unowned",
            "associatedtype","typealias","where","as","is","nil","true","false","fallthrough"
        };
        for (auto &k: ks) keywords.insert(k);
    }

    char peek(int offset = 0) const { size_t p = pos + offset; return p < src.size() ? src[p] : '\0'; }
    char advance() { char c = peek(); pos++; if (c == '\n') { line++; col = 1; } else col++; return c; }

    void skipWhitespace() {
        while (isspace((unsigned char)peek())) advance();
    }

    bool isDigitStart(char c) { return isdigit((unsigned char)c) || (c == '.' && isdigit((unsigned char)peek(1))); }
    bool isIdentifierStart(char c) { return isalpha((unsigned char)c) || c == '_'; }
    bool isIdentifierPart(char c) { return isalnum((unsigned char)c) || c == '_'; }
    bool isOperatorStart(char c) { return string("+-*/%=&|<>!~^?.:@#").find(c) != string::npos; }
    bool isSeparator(char c) { return string("(),;{}[]").find(c) != string::npos; }

    string scanLineComment() {
        string out; out += advance(); out += advance();
        while (peek() != '\0' && peek() != '\n') out += advance();
        return out;
    }

    string scanBlockComment() {
        string out; out += advance(); out += advance();
        int depth = 1;
        while (peek() != '\0') {
            if (peek() == '/' && peek(1) == '*') { out += advance(); out += advance(); depth++; }
            else if (peek() == '*' && peek(1) == '/') { out += advance(); out += advance(); depth--; if (depth == 0) break; }
            else { out += advance(); }
        }
        return depth ? "" : out;
    }

    string scanStringLiteral() {
        string out; out += advance();
        while (peek() != '\0' && peek() != '"') {
            if (peek() == '\\') { out += advance(); if (peek() != '\0') out += advance(); }
            else out += advance();
        }
        if (peek() == '"') out += advance();
        return out;
    }

    string scanDirective() {
        string out; out += advance();
        while (isIdentifierPart(peek())) out += advance();
        return out;
    }

    string scanNumber() {
        string out; bool seenDot = false;
        while (isdigit((unsigned char)peek()) || (peek() == '.' && !seenDot)) {
            if (peek() == '.') seenDot = true;
            out += advance();
        }
        return out;
    }

    TokenType classifyNumber(const string &s) {
        return s.find('.') != string::npos ? TokenType::NumberFloat : TokenType::Number;
    }

    string scanIdentifier() {
        string out; while (isIdentifierPart(peek())) out += advance(); return out;
    }

    string scanOperator() {
        string out; out += advance(); return out;
    }
};

// Функція для кольорового виводу токенів
string colorize(const Token &t) {
    using namespace Color;
    switch (t.type) {
        case TokenType::Keyword: return BLUE + t.lexeme + RESET;
        case TokenType::StringLiteral: return GREEN + t.lexeme + RESET;
        case TokenType::Number:
        case TokenType::NumberFloat:
        case TokenType::NumberHex: return YELLOW + t.lexeme + RESET;
        case TokenType::Comment: return GRAY + t.lexeme + RESET;
        case TokenType::Directive: return MAGENTA + t.lexeme + RESET;
        case TokenType::Operator: return CYAN + t.lexeme + RESET;
        case TokenType::Identifier: return RESET + t.lexeme + RESET;
        case TokenType::Error: return RED + t.lexeme + RESET;
        default: return RESET + t.lexeme + RESET;
    }
}

int main() {
    string swiftCode = R"swift(
import Foundation


let name = "Phone"
var age = 5
if age > 3 {
    print("Hello, \(name)!")
} else {
    print("Too young!")
}
)swift";

    Lexer lexer(swiftCode);
    auto tokens = lexer.tokenize();

    cout << "Swift Lexical Analysis\n\n";
    for (auto &t : tokens) {
        if (t.type == TokenType::EndOfFile) break;
        cout << "<" << colorize(t) << ", " << tokenTypeName(t.type) << ">\n";
    }

    cout << "\nEnd of Analysis\n";
    return 0;
}
