#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// ============ Token ============

enum class TokenType
{
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    // One or two character tokens
    BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Keywords
    AND, CLASS, ELSE, FALSE, FOR, FUN, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

    EOF_TOKEN
};

static const std::map<TokenType, std::string> tokenTypeNames = {
    {TokenType::LEFT_PAREN, "LEFT_PAREN"}, {TokenType::RIGHT_PAREN, "RIGHT_PAREN"},
    {TokenType::LEFT_BRACE, "LEFT_BRACE"}, {TokenType::RIGHT_BRACE, "RIGHT_BRACE"},
    {TokenType::COMMA, "COMMA"}, {TokenType::DOT, "DOT"},
    {TokenType::MINUS, "MINUS"}, {TokenType::PLUS, "PLUS"},
    {TokenType::SEMICOLON, "SEMICOLON"}, {TokenType::SLASH, "SLASH"},
    {TokenType::STAR, "STAR"}, {TokenType::BANG, "BANG"},
    {TokenType::BANG_EQUAL, "BANG_EQUAL"}, {TokenType::EQUAL, "EQUAL"},
    {TokenType::EQUAL_EQUAL, "EQUAL_EQUAL"}, {TokenType::GREATER, "GREATER"},
    {TokenType::GREATER_EQUAL, "GREATER_EQUAL"}, {TokenType::LESS, "LESS"},
    {TokenType::LESS_EQUAL, "LESS_EQUAL"}, {TokenType::IDENTIFIER, "IDENTIFIER"},
    {TokenType::STRING, "STRING"}, {TokenType::NUMBER, "NUMBER"},
    {TokenType::AND, "AND"}, {TokenType::CLASS, "CLASS"},
    {TokenType::ELSE, "ELSE"}, {TokenType::FALSE, "FALSE"},
    {TokenType::FOR, "FOR"}, {TokenType::FUN, "FUN"},
    {TokenType::IF, "IF"}, {TokenType::NIL, "NIL"},
    {TokenType::OR, "OR"}, {TokenType::PRINT, "PRINT"},
    {TokenType::RETURN, "RETURN"}, {TokenType::SUPER, "SUPER"},
    {TokenType::THIS, "THIS"}, {TokenType::TRUE, "TRUE"},
    {TokenType::VAR, "VAR"}, {TokenType::WHILE, "WHILE"},
    {TokenType::EOF_TOKEN, "EOF"}
};

static const std::map<std::string, TokenType> keywordTypes = {
    {"and", TokenType::AND}, {"class", TokenType::CLASS}, {"else", TokenType::ELSE},
    {"false", TokenType::FALSE}, {"for", TokenType::FOR}, {"fun", TokenType::FUN},
    {"if", TokenType::IF}, {"nil", TokenType::NIL}, {"or", TokenType::OR},
    {"print", TokenType::PRINT}, {"return", TokenType::RETURN}, {"super", TokenType::SUPER},
    {"this", TokenType::THIS}, {"true", TokenType::TRUE}, {"var", TokenType::VAR},
    {"while", TokenType::WHILE}
};

struct Token
{
    TokenType type;
    std::string lexeme;
    std::string literal;
    int line;

    std::string toString() const
    {
        return tokenTypeNames.at(type) + " " + lexeme + " " + literal;
    }
};

// ============ Scanner ============

class Scanner
{
public:
    Scanner(const std::string& source) : source_(source) {}

    std::vector<Token> scanTokens()
    {
        while (current_ < source_.size())
        {
            scanToken();
        }
        tokens_.push_back({TokenType::EOF_TOKEN, "", "null", line_});
        return tokens_;
    }

    bool hasError() const { return hasError_; }

private:
    std::string source_;
    std::vector<Token> tokens_;
    size_t current_ = 0;
    int line_ = 1;
    bool hasError_ = false;

    char advance() { return source_[current_++]; }
    char peek() const { return current_ < source_.size() ? source_[current_] : '\0'; }
    char peekNext() const { return current_ + 1 < source_.size() ? source_[current_ + 1] : '\0'; }

    bool match(char expected)
    {
        if (current_ >= source_.size() || source_[current_] != expected) return false;
        current_++;
        return true;
    }

    void addToken(TokenType type, const std::string& lexeme, const std::string& literal)
    {
        tokens_.push_back({type, lexeme, literal, line_});
    }

    void scanToken()
    {
        char c = advance();
        switch (c)
        {
            case '(': addToken(TokenType::LEFT_PAREN, "(", "null"); break;
            case ')': addToken(TokenType::RIGHT_PAREN, ")", "null"); break;
            case '{': addToken(TokenType::LEFT_BRACE, "{", "null"); break;
            case '}': addToken(TokenType::RIGHT_BRACE, "}", "null"); break;
            case ',': addToken(TokenType::COMMA, ",", "null"); break;
            case '.': addToken(TokenType::DOT, ".", "null"); break;
            case '-': addToken(TokenType::MINUS, "-", "null"); break;
            case '+': addToken(TokenType::PLUS, "+", "null"); break;
            case ';': addToken(TokenType::SEMICOLON, ";", "null"); break;
            case '*': addToken(TokenType::STAR, "*", "null"); break;
            case '=':
                if (match('=')) addToken(TokenType::EQUAL_EQUAL, "==", "null");
                else addToken(TokenType::EQUAL, "=", "null");
                break;
            case '!':
                if (match('=')) addToken(TokenType::BANG_EQUAL, "!=", "null");
                else addToken(TokenType::BANG, "!", "null");
                break;
            case '<':
                if (match('=')) addToken(TokenType::LESS_EQUAL, "<=", "null");
                else addToken(TokenType::LESS, "<", "null");
                break;
            case '>':
                if (match('=')) addToken(TokenType::GREATER_EQUAL, ">=", "null");
                else addToken(TokenType::GREATER, ">", "null");
                break;
            case '/':
                if (match('/'))
                {
                    while (current_ < source_.size() && source_[current_] != '\n') current_++;
                }
                else
                {
                    addToken(TokenType::SLASH, "/", "null");
                }
                break;
            case '"': scanString(); break;
            case '\n': line_++; break;
            case ' ': case '\r': case '\t': break;
            default:
                if (std::isdigit(c)) scanNumber(c);
                else if (std::isalpha(c) || c == '_') scanIdentifier(c);
                else
                {
                    std::cerr << "[line " << line_ << "] Error: Unexpected character: " << c << std::endl;
                    hasError_ = true;
                }
                break;
        }
    }

    void scanString()
    {
        size_t start = current_ - 1;
        while (current_ < source_.size() && source_[current_] != '"')
        {
            if (source_[current_] == '\n') line_++;
            current_++;
        }
        if (current_ >= source_.size())
        {
            std::cerr << "[line " << line_ << "] Error: Unterminated string." << std::endl;
            hasError_ = true;
            return;
        }
        current_++;
        std::string lexeme = source_.substr(start, current_ - start);
        std::string literal = source_.substr(start + 1, current_ - start - 2);
        addToken(TokenType::STRING, lexeme, literal);
    }

    void scanNumber(char first)
    {
        size_t start = current_ - 1;
        while (current_ < source_.size() && std::isdigit(source_[current_])) current_++;
        if (current_ < source_.size() && source_[current_] == '.' &&
            current_ + 1 < source_.size() && std::isdigit(source_[current_ + 1]))
        {
            current_++;
            while (current_ < source_.size() && std::isdigit(source_[current_])) current_++;
        }
        std::string lexeme = source_.substr(start, current_ - start);
        double value = std::stod(lexeme);
        std::string literal;
        if (lexeme.find('.') == std::string::npos)
        {
            literal = std::to_string(static_cast<long long>(value)) + ".0";
        }
        else
        {
            std::ostringstream oss;
            oss << std::setprecision(15) << value;
            literal = oss.str();
            if (literal.find('.') == std::string::npos)
            {
                literal += ".0";
            }
        }
        addToken(TokenType::NUMBER, lexeme, literal);
    }

    void scanIdentifier(char first)
    {
        size_t start = current_ - 1;
        while (current_ < source_.size() && (std::isalnum(source_[current_]) || source_[current_] == '_')) current_++;
        std::string lexeme = source_.substr(start, current_ - start);
        auto it = keywordTypes.find(lexeme);
        if (it != keywordTypes.end())
        {
            addToken(it->second, lexeme, "null");
        }
        else
        {
            addToken(TokenType::IDENTIFIER, lexeme, "null");
        }
    }
};

// ============ AST ============

struct Expr
{
    virtual ~Expr() = default;
    virtual std::string print() const = 0;
};

struct LiteralExpr : Expr
{
    std::string value;
    LiteralExpr(const std::string& v) : value(v) {}
    std::string print() const override { return value; }
};

struct GroupExpr : Expr
{
    std::unique_ptr<Expr> expr;
    GroupExpr(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    std::string print() const override { return "(group " + expr->print() + ")"; }
};

struct UnaryExpr : Expr
{
    std::string op;
    std::unique_ptr<Expr> right;
    UnaryExpr(const std::string& op, std::unique_ptr<Expr> r) : op(op), right(std::move(r)) {}
    std::string print() const override { return "(" + op + " " + right->print() + ")"; }
};

struct BinaryExpr : Expr
{
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> l, const std::string& op, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(op), right(std::move(r)) {}
    std::string print() const override
    {
        return "(" + op + " " + left->print() + " " + right->print() + ")";
    }
};

// ============ Parser ============

class Parser
{
public:
    Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}

    std::unique_ptr<Expr> parse()
    {
        return expression();
    }

    bool hasError() const { return hasError_; }

private:
    std::vector<Token> tokens_;
    size_t current_ = 0;
    bool hasError_ = false;

    const Token& peek() const { return tokens_[current_]; }
    const Token& previous() const { return tokens_[current_ - 1]; }
    bool isAtEnd() const { return peek().type == TokenType::EOF_TOKEN; }

    Token advance()
    {
        if (!isAtEnd()) current_++;
        return previous();
    }

    bool check(TokenType type) const
    {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    std::unique_ptr<Expr> expression()
    {
        return addition();
    }

    std::unique_ptr<Expr> addition()
    {
        auto left = multiplication();
        while (check(TokenType::PLUS) || check(TokenType::MINUS))
        {
            Token op = advance();
            auto right = multiplication();
            left = std::make_unique<BinaryExpr>(std::move(left), op.lexeme, std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> multiplication()
    {
        auto left = unary();
        while (check(TokenType::STAR) || check(TokenType::SLASH))
        {
            Token op = advance();
            auto right = unary();
            left = std::make_unique<BinaryExpr>(std::move(left), op.lexeme, std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> unary()
    {
        if (check(TokenType::BANG) || check(TokenType::MINUS))
        {
            Token op = advance();
            auto right = unary();
            return std::make_unique<UnaryExpr>(op.lexeme, std::move(right));
        }
        return primary();
    }

    std::unique_ptr<Expr> primary()
    {
        if (check(TokenType::TRUE))
        {
            advance();
            return std::make_unique<LiteralExpr>("true");
        }
        if (check(TokenType::FALSE))
        {
            advance();
            return std::make_unique<LiteralExpr>("false");
        }
        if (check(TokenType::NIL))
        {
            advance();
            return std::make_unique<LiteralExpr>("nil");
        }
        if (check(TokenType::NUMBER))
        {
            Token tok = advance();
            return std::make_unique<LiteralExpr>(tok.literal);
        }
        if (check(TokenType::STRING))
        {
            Token tok = advance();
            return std::make_unique<LiteralExpr>(tok.literal);
        }

        if (check(TokenType::LEFT_PAREN))
        {
            advance();
            auto expr = expression();
            if (!check(TokenType::RIGHT_PAREN))
            {
                hasError_ = true;
                std::cerr << "[line " << peek().line << "] Error at '" << peek().lexeme << "': Expect ')' after expression." << std::endl;
                return nullptr;
            }
            advance();
            return std::make_unique<GroupExpr>(std::move(expr));
        }

        hasError_ = true;
        std::cerr << "[line " << peek().line << "] Error at '" << peek().lexeme << "': Expect expression." << std::endl;
        return nullptr;
    }
};

// ============ Helpers ============

std::string read_file_contents(const std::string& filename);

// ============ Main ============

int main(int argc, char *argv[])
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::cerr << "Logs from your program will appear here!" << std::endl;

    if (argc < 3)
    {
        std::cerr << "Usage: ./your_program tokenize <filename>" << std::endl;
        return 1;
    }

    const std::string command = argv[1];

    if (command == "tokenize")
    {
        std::string file_contents = read_file_contents(argv[2]);
        Scanner scanner(file_contents);
        auto tokens = scanner.scanTokens();
        for (const auto& token : tokens)
        {
            std::cout << token.toString() << std::endl;
        }
        if (scanner.hasError()) return 65;
    }
    else if (command == "parse")
    {
        std::string file_contents = read_file_contents(argv[2]);
        Scanner scanner(file_contents);
        auto tokens = scanner.scanTokens();
        if (scanner.hasError()) return 65;

        Parser parser(tokens);
        auto expr = parser.parse();
        if (parser.hasError()) return 65;
        if (expr)
        {
            std::cout << expr->print() << std::endl;
        }
    }
    else
    {
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}

std::string read_file_contents(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error reading file: " << filename << std::endl;
        std::exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}
