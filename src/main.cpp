#include <chrono>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct RuntimeError : std::runtime_error
{
    int line;
    RuntimeError(const std::string& msg, int line)
        : std::runtime_error(msg), line(line) {}
};

struct ParseError : std::runtime_error
{
    ParseError() : std::runtime_error("parse error") {}
};

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

enum class ValueType { NIL, BOOL, NUMBER, STRING, CALLABLE, INSTANCE };

struct LoxValue;
struct LoxInstance;

struct LoxCallable
{
    virtual ~LoxCallable() = default;
    virtual int arity() const = 0;
    virtual LoxValue call(const std::vector<LoxValue>& args) const = 0;
    virtual std::string name() const = 0;
    virtual std::string toString() const { return "<fn " + name() + ">"; }
};

struct LoxClass;

struct LoxInstance
{
    std::shared_ptr<LoxClass> klass;
    std::map<std::string, LoxValue> fields;
    LoxInstance(std::shared_ptr<LoxClass> klass) : klass(klass) {}
    std::string toString() const;

    LoxValue get(const std::string& name, int line) const
    {
        auto it = fields.find(name);
        if (it != fields.end()) return it->second;
        throw RuntimeError("Undefined property '" + name + "'.", line);
    }

    void set(const std::string& name, const LoxValue& value)
    {
        fields[name] = value;
    }
};

struct LoxValue
{
    ValueType type;
    bool boolVal = false;
    double numVal = 0.0;
    std::string strVal;
    std::shared_ptr<LoxCallable> callableVal;
    std::shared_ptr<LoxInstance> instanceVal;

    static LoxValue Nil() { return {ValueType::NIL}; }
    static LoxValue Bool(bool b) { return {ValueType::BOOL, b, 0.0, "", nullptr, nullptr}; }
    static LoxValue Number(double n) { return {ValueType::NUMBER, false, n, "", nullptr, nullptr}; }
    static LoxValue String(const std::string& s) { return {ValueType::STRING, false, 0.0, s, nullptr, nullptr}; }
    static LoxValue Callable(std::shared_ptr<LoxCallable> c) { return {ValueType::CALLABLE, false, 0.0, "", c, nullptr}; }
    static LoxValue Instance(std::shared_ptr<LoxInstance> i) { return {ValueType::INSTANCE, false, 0.0, "", nullptr, i}; }

    std::string toString() const
    {
        switch (type)
        {
            case ValueType::NIL: return "nil";
            case ValueType::BOOL: return boolVal ? "true" : "false";
            case ValueType::NUMBER:
            {
                std::ostringstream oss;
                oss << std::setprecision(15) << numVal;
                std::string s = oss.str();
                // 如果是整数值，去掉尾部的 .0
                if (s.find('.') != std::string::npos)
                {
                    // 去掉尾部多余的0
                    size_t dot = s.find('.');
                    std::string afterDot = s.substr(dot + 1);
                    bool allZero = true;
                    for (char c : afterDot)
                    {
                        if (c != '0') { allZero = false; break; }
                    }
                    if (allZero) return s.substr(0, dot);
                }
                return s;
            }
            case ValueType::STRING: return strVal;
            case ValueType::CALLABLE: return callableVal->toString();
            case ValueType::INSTANCE: return instanceVal->toString();
        }
        return "nil";
    }
};

struct ReturnValue
{
    LoxValue value;
};

// ============ Environment ============

class Environment : public std::enable_shared_from_this<Environment>
{
public:
    Environment() : enclosing_(nullptr) {}
    Environment(std::shared_ptr<Environment> enclosing) : enclosing_(enclosing) {}

    void define(const std::string& name, const LoxValue& value)
    {
        values_[name] = value;
    }

    LoxValue get(const std::string& name, int line) const
    {
        auto it = values_.find(name);
        if (it != values_.end()) return it->second;
        if (enclosing_) return enclosing_->get(name, line);
        throw RuntimeError("Undefined variable '" + name + "'.", line);
    }

    LoxValue getAt(int distance, const std::string& name) const
    {
        const Environment* env = this;
        for (int i = 0; i < distance; i++)
        {
            env = env->enclosing_.get();
        }
        auto it = env->values_.find(name);
        if (it != env->values_.end()) return it->second;
        return LoxValue::Nil();
    }

    void assign(const std::string& name, const LoxValue& value, int line)
    {
        auto it = values_.find(name);
        if (it != values_.end())
        {
            it->second = value;
            return;
        }
        if (enclosing_)
        {
            enclosing_->assign(name, value, line);
            return;
        }
        throw RuntimeError("Undefined variable '" + name + "'.", line);
    }

    void assignAt(int distance, const std::string& name, const LoxValue& value)
    {
        Environment* env = this;
        for (int i = 0; i < distance; i++)
        {
            env = env->enclosing_.get();
        }
        env->values_[name] = value;
    }

private:
    std::shared_ptr<Environment> enclosing_;
    std::map<std::string, LoxValue> values_;
};

// Global environment pointer for unresolved (global) variable lookups
static std::shared_ptr<Environment> globalEnvironment;

struct Expr
{
    virtual ~Expr() = default;
    virtual std::string print() const = 0;
    virtual LoxValue evaluate(std::shared_ptr<Environment> env) const = 0;
};

struct LiteralExpr : Expr
{
    std::string value;
    ValueType litType;
    LiteralExpr(const std::string& v, ValueType t = ValueType::NIL) : value(v), litType(t) {}
    std::string print() const override { return value; }
    LoxValue evaluate(std::shared_ptr<Environment> env) const override
    {
        switch (litType)
        {
            case ValueType::NIL: return LoxValue::Nil();
            case ValueType::BOOL: return LoxValue::Bool(value == "true");
            case ValueType::NUMBER: return LoxValue::Number(std::stod(value));
            case ValueType::STRING: return LoxValue::String(value);
        }
        return LoxValue::Nil();
    }
};

struct GroupExpr : Expr
{
    std::unique_ptr<Expr> expr;
    GroupExpr(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    std::string print() const override { return "(group " + expr->print() + ")"; }
    LoxValue evaluate(std::shared_ptr<Environment> env) const override { return expr->evaluate(env); }
};

struct UnaryExpr : Expr
{
    std::string op;
    std::unique_ptr<Expr> right;
    int line;
    UnaryExpr(const std::string& op, std::unique_ptr<Expr> r, int line)
        : op(op), right(std::move(r)), line(line) {}
    std::string print() const override { return "(" + op + " " + right->print() + ")"; }
    LoxValue evaluate(std::shared_ptr<Environment> env) const override
    {
        LoxValue val = right->evaluate(env);
        if (op == "-")
        {
            if (val.type != ValueType::NUMBER)
            {
                throw RuntimeError("Operand must be a number.", line);
            }
            return LoxValue::Number(-val.numVal);
        }
        if (op == "!")
        {
            // falsey: nil and false
            bool truthy = true;
            if (val.type == ValueType::NIL) truthy = false;
            else if (val.type == ValueType::BOOL) truthy = val.boolVal;
            return LoxValue::Bool(!truthy);
        }
        return LoxValue::Nil();
    }
};

struct BinaryExpr : Expr
{
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
    int line;
    BinaryExpr(std::unique_ptr<Expr> l, const std::string& op, std::unique_ptr<Expr> r, int line)
        : left(std::move(l)), op(op), right(std::move(r)), line(line) {}
    std::string print() const override
    {
        return "(" + op + " " + left->print() + " " + right->print() + ")";
    }
    LoxValue evaluate(std::shared_ptr<Environment> env) const override
    {
        LoxValue l = left->evaluate(env);
        LoxValue r = right->evaluate(env);
        if (op == "+")
        {
            if (l.type == ValueType::STRING && r.type == ValueType::STRING)
                return LoxValue::String(l.strVal + r.strVal);
            if (l.type == ValueType::NUMBER && r.type == ValueType::NUMBER)
                return LoxValue::Number(l.numVal + r.numVal);
            throw RuntimeError("Operands must be two numbers or two strings.", line);
        }
        if (op == "-")
        {
            if (l.type != ValueType::NUMBER || r.type != ValueType::NUMBER)
                throw RuntimeError("Operands must be numbers.", line);
            return LoxValue::Number(l.numVal - r.numVal);
        }
        if (op == "*")
        {
            if (l.type != ValueType::NUMBER || r.type != ValueType::NUMBER)
                throw RuntimeError("Operands must be numbers.", line);
            return LoxValue::Number(l.numVal * r.numVal);
        }
        if (op == "/")
        {
            if (l.type != ValueType::NUMBER || r.type != ValueType::NUMBER)
                throw RuntimeError("Operands must be numbers.", line);
            return LoxValue::Number(l.numVal / r.numVal);
        }
        if (op == ">")
        {
            if (l.type != ValueType::NUMBER || r.type != ValueType::NUMBER)
                throw RuntimeError("Operands must be numbers.", line);
            return LoxValue::Bool(l.numVal > r.numVal);
        }
        if (op == ">=")
        {
            if (l.type != ValueType::NUMBER || r.type != ValueType::NUMBER)
                throw RuntimeError("Operands must be numbers.", line);
            return LoxValue::Bool(l.numVal >= r.numVal);
        }
        if (op == "<")
        {
            if (l.type != ValueType::NUMBER || r.type != ValueType::NUMBER)
                throw RuntimeError("Operands must be numbers.", line);
            return LoxValue::Bool(l.numVal < r.numVal);
        }
        if (op == "<=")
        {
            if (l.type != ValueType::NUMBER || r.type != ValueType::NUMBER)
                throw RuntimeError("Operands must be numbers.", line);
            return LoxValue::Bool(l.numVal <= r.numVal);
        }
        if (op == "==")
        {
            if (l.type != r.type) return LoxValue::Bool(false);
            if (l.type == ValueType::NIL) return LoxValue::Bool(true);
            if (l.type == ValueType::BOOL) return LoxValue::Bool(l.boolVal == r.boolVal);
            if (l.type == ValueType::NUMBER) return LoxValue::Bool(l.numVal == r.numVal);
            if (l.type == ValueType::STRING) return LoxValue::Bool(l.strVal == r.strVal);
        }
        if (op == "!=")
        {
            if (l.type != r.type) return LoxValue::Bool(true);
            if (l.type == ValueType::NIL) return LoxValue::Bool(false);
            if (l.type == ValueType::BOOL) return LoxValue::Bool(l.boolVal != r.boolVal);
            if (l.type == ValueType::NUMBER) return LoxValue::Bool(l.numVal != r.numVal);
            if (l.type == ValueType::STRING) return LoxValue::Bool(l.strVal != r.strVal);
        }
        return LoxValue::Nil();
    }
};

struct VariableExpr : Expr
{
    std::string name;
    int line;
    mutable int resolvedDistance = -1; // -1 means not resolved (global)
    VariableExpr(const std::string& name, int line) : name(name), line(line) {}
    std::string print() const override { return name; }
    LoxValue evaluate(std::shared_ptr<Environment> env) const override
    {
        if (resolvedDistance >= 0)
        {
            return env->getAt(resolvedDistance, name);
        }
        return globalEnvironment->get(name, line);
    }
};

struct AssignExpr : Expr
{
    std::string name;
    std::unique_ptr<Expr> value;
    int line;
    mutable int resolvedDistance = -1; // -1 means not resolved (global)
    AssignExpr(const std::string& name, std::unique_ptr<Expr> value, int line)
        : name(name), value(std::move(value)), line(line) {}
    std::string print() const override { return "(= " + name + " " + value->print() + ")"; }
    LoxValue evaluate(std::shared_ptr<Environment> env) const override
    {
        LoxValue val = value->evaluate(env);
        if (resolvedDistance >= 0)
        {
            env->assignAt(resolvedDistance, name, val);
        }
        else
        {
            globalEnvironment->assign(name, val, line);
        }
        return val;
    }
};

static bool isTruthy(const LoxValue& val);

struct LogicalExpr : Expr
{
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
    LogicalExpr(std::unique_ptr<Expr> l, const std::string& op, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(op), right(std::move(r)) {}
    std::string print() const override
    {
        return "(" + op + " " + left->print() + " " + right->print() + ")";
    }
    LoxValue evaluate(std::shared_ptr<Environment> env) const override
    {
        LoxValue l = left->evaluate(env);
        if (op == "or")
        {
            if (isTruthy(l)) return l;
        }
        else
        {
            if (!isTruthy(l)) return l;
        }
        return right->evaluate(env);
    }
};

// ============ Native Functions ============

struct ClockNative : LoxCallable
{
    int arity() const override { return 0; }
    std::string name() const override { return "clock"; }
    LoxValue call(const std::vector<LoxValue>& args) const override
    {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0;
        return LoxValue::Number(seconds);
    }
};

// ============ Call Expression ============

struct CallExpr : Expr
{
    std::unique_ptr<Expr> callee;
    Token paren;
    std::vector<std::unique_ptr<Expr>> arguments;
    CallExpr(std::unique_ptr<Expr> callee, Token paren, std::vector<std::unique_ptr<Expr>> args)
        : callee(std::move(callee)), paren(paren), arguments(std::move(args)) {}
    std::string print() const override
    {
        // Not typically used for parse output, but provide something reasonable
        std::string result = callee->print() + "(";
        for (size_t i = 0; i < arguments.size(); i++)
        {
            if (i > 0) result += ", ";
            result += arguments[i]->print();
        }
        result += ")";
        return result;
    }
    LoxValue evaluate(std::shared_ptr<Environment> env) const override
    {
        LoxValue calleeVal = callee->evaluate(env);
        if (calleeVal.type != ValueType::CALLABLE || !calleeVal.callableVal)
        {
            throw RuntimeError("Can only call functions and classes.", paren.line);
        }
        std::vector<LoxValue> args;
        for (const auto& arg : arguments)
        {
            args.push_back(arg->evaluate(env));
        }
        if (static_cast<int>(args.size()) != calleeVal.callableVal->arity())
        {
            throw RuntimeError(
                "Expected " + std::to_string(calleeVal.callableVal->arity()) +
                " arguments but got " + std::to_string(args.size()) + ".",
                paren.line);
        }
        return calleeVal.callableVal->call(args);
    }
};

// ============ Get/Set Expressions ============

struct GetExpr : Expr
{
    std::unique_ptr<Expr> object;
    Token name;
    GetExpr(std::unique_ptr<Expr> object, Token name)
        : object(std::move(object)), name(name) {}
    std::string print() const override
    {
        return object->print() + "." + name.lexeme;
    }
    LoxValue evaluate(std::shared_ptr<Environment> env) const override
    {
        LoxValue obj = object->evaluate(env);
        if (obj.type == ValueType::INSTANCE)
        {
            return obj.instanceVal->get(name.lexeme, name.line);
        }
        throw RuntimeError("Only instances have properties.", name.line);
    }
};

struct SetExpr : Expr
{
    std::unique_ptr<Expr> object;
    Token name;
    std::unique_ptr<Expr> value;
    SetExpr(std::unique_ptr<Expr> object, Token name, std::unique_ptr<Expr> value)
        : object(std::move(object)), name(name), value(std::move(value)) {}
    std::string print() const override
    {
        return object->print() + "." + name.lexeme + " = " + value->print();
    }
    LoxValue evaluate(std::shared_ptr<Environment> env) const override
    {
        LoxValue obj = object->evaluate(env);
        if (obj.type != ValueType::INSTANCE)
        {
            throw RuntimeError("Only instances have fields.", name.line);
        }
        LoxValue val = value->evaluate(env);
        obj.instanceVal->set(name.lexeme, val);
        return val;
    }
};

// ============ Statements ============

struct Stmt
{
    virtual ~Stmt() = default;
    virtual void execute(std::shared_ptr<Environment> env) const = 0;
};

struct PrintStmt : Stmt
{
    std::unique_ptr<Expr> expr;
    PrintStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void execute(std::shared_ptr<Environment> env) const override
    {
        LoxValue val = expr->evaluate(env);
        std::cout << val.toString() << std::endl;
    }
};

struct ExpressionStmt : Stmt
{
    std::unique_ptr<Expr> expr;
    ExpressionStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void execute(std::shared_ptr<Environment> env) const override
    {
        expr->evaluate(env);
    }
};

struct VarStmt : Stmt
{
    std::string name;
    int line;
    std::unique_ptr<Expr> initializer;
    VarStmt(const std::string& name, int line, std::unique_ptr<Expr> init)
        : name(name), line(line), initializer(std::move(init)) {}
    void execute(std::shared_ptr<Environment> env) const override
    {
        LoxValue value = LoxValue::Nil();
        if (initializer)
        {
            value = initializer->evaluate(env);
        }
        env->define(name, value);
    }
};

struct BlockStmt : Stmt
{
    std::vector<std::unique_ptr<Stmt>> statements;
    BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
        : statements(std::move(stmts)) {}
    void execute(std::shared_ptr<Environment> env) const override
    {
        auto blockEnv = std::make_shared<Environment>(env);
        for (const auto& stmt : statements)
        {
            stmt->execute(blockEnv);
        }
    }
};

static bool isTruthy(const LoxValue& val)
{
    if (val.type == ValueType::NIL) return false;
    if (val.type == ValueType::BOOL) return val.boolVal;
    return true;
}

struct IfStmt : Stmt
{
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> thenBr, std::unique_ptr<Stmt> elseBr)
        : condition(std::move(cond)), thenBranch(std::move(thenBr)), elseBranch(std::move(elseBr)) {}
    void execute(std::shared_ptr<Environment> env) const override
    {
        LoxValue val = condition->evaluate(env);
        if (isTruthy(val))
        {
            thenBranch->execute(env);
        }
        else if (elseBranch)
        {
            elseBranch->execute(env);
        }
    }
};

struct WhileStmt : Stmt
{
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> body)
        : condition(std::move(cond)), body(std::move(body)) {}
    void execute(std::shared_ptr<Environment> env) const override
    {
        while (isTruthy(condition->evaluate(env)))
        {
            body->execute(env);
        }
    }
};

struct ReturnStmt : Stmt
{
    Token keyword;
    std::unique_ptr<Expr> value;
    ReturnStmt(Token keyword, std::unique_ptr<Expr> value)
        : keyword(keyword), value(std::move(value)) {}
    void execute(std::shared_ptr<Environment> env) const override
    {
        LoxValue val = LoxValue::Nil();
        if (value)
        {
            val = value->evaluate(env);
        }
        throw ReturnValue{val};
    }
};

struct FunctionStmt : Stmt
{
    Token name;
    std::vector<Token> params;
    std::vector<std::unique_ptr<Stmt>> body;
    FunctionStmt(Token name, std::vector<Token> params, std::vector<std::unique_ptr<Stmt>> body)
        : name(name), params(std::move(params)), body(std::move(body)) {}
    void execute(std::shared_ptr<Environment> env) const override;
};

// ============ LoxFunction ============

struct LoxFunction : LoxCallable
{
    const FunctionStmt* declaration;
    std::shared_ptr<Environment> closure;
    LoxFunction(const FunctionStmt* decl, std::shared_ptr<Environment> closure)
        : declaration(decl), closure(closure) {}
    int arity() const override { return static_cast<int>(declaration->params.size()); }
    std::string name() const override { return declaration->name.lexeme; }
    LoxValue call(const std::vector<LoxValue>& args) const override
    {
        auto funcEnv = std::make_shared<Environment>(closure);
        for (size_t i = 0; i < declaration->params.size(); i++)
        {
            funcEnv->define(declaration->params[i].lexeme, args[i]);
        }
        try
        {
            for (const auto& stmt : declaration->body)
            {
                stmt->execute(funcEnv);
            }
        }
        catch (const ReturnValue& ret)
        {
            return ret.value;
        }
        return LoxValue::Nil();
    }
};

void FunctionStmt::execute(std::shared_ptr<Environment> env) const
{
    auto function = std::make_shared<LoxFunction>(this, env);
    env->define(name.lexeme, LoxValue::Callable(function));
}

// ============ LoxClass ============

struct ClassStmt : Stmt
{
    Token name;
    ClassStmt(Token name) : name(name) {}
    void execute(std::shared_ptr<Environment> env) const override;
};

struct LoxClass : LoxCallable, public std::enable_shared_from_this<LoxClass>
{
    std::string className;
    LoxClass(const std::string& name) : className(name) {}
    int arity() const override { return 0; }
    std::string name() const override { return className; }
    std::string toString() const override { return className; }
    LoxValue call(const std::vector<LoxValue>& args) const override
    {
        auto instance = std::make_shared<LoxInstance>(
            std::const_pointer_cast<LoxClass>(shared_from_this()));
        return LoxValue::Instance(instance);
    }
};

std::string LoxInstance::toString() const
{
    return klass->className + " instance";
}

void ClassStmt::execute(std::shared_ptr<Environment> env) const
{
    auto klass = std::make_shared<LoxClass>(name.lexeme);
    env->define(name.lexeme, LoxValue::Callable(klass));
}

// ============ Parser ============

class Parser
{
public:
    Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}

    std::unique_ptr<Expr> parse()
    {
        return expression();
    }

    std::vector<std::unique_ptr<Stmt>> parseStatements()
    {
        std::vector<std::unique_ptr<Stmt>> stmts;
        while (!isAtEnd())
        {
            auto s = declaration();
            if (s) stmts.push_back(std::move(s));
        }
        return stmts;
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

    ParseError error(const Token& token, const std::string& message)
    {
        hasError_ = true;
        if (token.type == TokenType::EOF_TOKEN)
        {
            std::cerr << "[line " << token.line << "] Error at end: " << message << std::endl;
        }
        else
        {
            std::cerr << "[line " << token.line << "] Error at '" << token.lexeme << "': " << message << std::endl;
        }
        return ParseError();
    }

    std::unique_ptr<Expr> expression()
    {
        return assignment();
    }

    std::unique_ptr<Expr> assignment()
    {
        auto expr = orExpression();

        if (check(TokenType::EQUAL))
        {
            Token equals = advance();
            auto value = assignment(); // 右结合，递归调用自身

            // 检查左侧是否为变量
            VariableExpr* varExpr = dynamic_cast<VariableExpr*>(expr.get());
            if (varExpr)
            {
                std::string name = varExpr->name;
                int line = varExpr->line;
                return std::make_unique<AssignExpr>(name, std::move(value), line);
            }

            // 检查左侧是否为属性访问 (GetExpr)
            GetExpr* getExpr = dynamic_cast<GetExpr*>(expr.get());
            if (getExpr)
            {
                auto setExpr = std::make_unique<SetExpr>(
                    std::move(getExpr->object), getExpr->name, std::move(value));
                return setExpr;
            }

            error(equals, "Invalid assignment target.");
            return nullptr;
        }

        return expr;
    }

    std::unique_ptr<Expr> orExpression()
    {
        auto expr = andExpression();
        while (check(TokenType::OR))
        {
            Token op = advance();
            auto right = andExpression();
            expr = std::make_unique<LogicalExpr>(std::move(expr), op.lexeme, std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expr> andExpression()
    {
        auto expr = equality();
        while (check(TokenType::AND))
        {
            Token op = advance();
            auto right = equality();
            expr = std::make_unique<LogicalExpr>(std::move(expr), op.lexeme, std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expr> equality()
    {
        auto left = comparison();
        while (check(TokenType::EQUAL_EQUAL) || check(TokenType::BANG_EQUAL))
        {
            Token op = advance();
            auto right = comparison();
            left = std::make_unique<BinaryExpr>(std::move(left), op.lexeme, std::move(right), op.line);
        }
        return left;
    }

    std::unique_ptr<Expr> comparison()
    {
        auto left = addition();
        while (check(TokenType::GREATER) || check(TokenType::GREATER_EQUAL) ||
               check(TokenType::LESS) || check(TokenType::LESS_EQUAL))
        {
            Token op = advance();
            auto right = addition();
            left = std::make_unique<BinaryExpr>(std::move(left), op.lexeme, std::move(right), op.line);
        }
        return left;
    }

    std::unique_ptr<Expr> addition()
    {
        auto left = multiplication();
        while (check(TokenType::PLUS) || check(TokenType::MINUS))
        {
            Token op = advance();
            auto right = multiplication();
            left = std::make_unique<BinaryExpr>(std::move(left), op.lexeme, std::move(right), op.line);
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
            left = std::make_unique<BinaryExpr>(std::move(left), op.lexeme, std::move(right), op.line);
        }
        return left;
    }

    std::unique_ptr<Expr> unary()
    {
        if (check(TokenType::BANG) || check(TokenType::MINUS))
        {
            Token op = advance();
            auto right = unary();
            return std::make_unique<UnaryExpr>(op.lexeme, std::move(right), op.line);
        }
        return call();
    }

    std::unique_ptr<Expr> call()
    {
        auto expr = primary();

        while (true)
        {
            if (check(TokenType::LEFT_PAREN))
            {
                Token paren = advance();
                std::vector<std::unique_ptr<Expr>> arguments;
                if (!check(TokenType::RIGHT_PAREN))
                {
                    do
                    {
                        if (arguments.size() >= 255)
                        {
                            error(peek(), "Can't have more than 255 arguments.");
                        }
                        arguments.push_back(expression());
                    } while (check(TokenType::COMMA) && (advance(), true));
                }
                if (!check(TokenType::RIGHT_PAREN))
                {
                    throw error(peek(), "Expect ')' after arguments.");
                }
                Token closeParen = advance();
                expr = std::make_unique<CallExpr>(std::move(expr), closeParen, std::move(arguments));
            }
            else if (check(TokenType::DOT))
            {
                advance();
                if (!check(TokenType::IDENTIFIER))
                {
                    throw error(peek(), "Expect property name after '.'.");
                }
                Token name = advance();
                expr = std::make_unique<GetExpr>(std::move(expr), name);
            }
            else
            {
                break;
            }
        }

        return expr;
    }

    std::unique_ptr<Expr> primary()
    {
        if (check(TokenType::TRUE))
        {
            advance();
            return std::make_unique<LiteralExpr>("true", ValueType::BOOL);
        }
        if (check(TokenType::FALSE))
        {
            advance();
            return std::make_unique<LiteralExpr>("false", ValueType::BOOL);
        }
        if (check(TokenType::NIL))
        {
            advance();
            return std::make_unique<LiteralExpr>("nil", ValueType::NIL);
        }
        if (check(TokenType::NUMBER))
        {
            Token tok = advance();
            return std::make_unique<LiteralExpr>(tok.literal, ValueType::NUMBER);
        }
        if (check(TokenType::STRING))
        {
            Token tok = advance();
            return std::make_unique<LiteralExpr>(tok.literal, ValueType::STRING);
        }
        if (check(TokenType::IDENTIFIER))
        {
            Token tok = advance();
            return std::make_unique<VariableExpr>(tok.lexeme, tok.line);
        }

        if (check(TokenType::LEFT_PAREN))
        {
            advance();
            auto expr = expression();
            if (!check(TokenType::RIGHT_PAREN))
            {
                throw error(peek(), "Expect ')' after expression.");
            }
            advance();
            return std::make_unique<GroupExpr>(std::move(expr));
        }

        throw error(peek(), "Expect expression.");
    }

    void synchronize()
    {
        while (!isAtEnd())
        {
            if (previous().type == TokenType::SEMICOLON) return;
            switch (peek().type)
            {
                case TokenType::CLASS:
                case TokenType::FUN:
                case TokenType::VAR:
                case TokenType::FOR:
                case TokenType::IF:
                case TokenType::WHILE:
                case TokenType::PRINT:
                case TokenType::RETURN:
                    return;
                default:
                    break;
            }
            advance();
        }
    }

    std::unique_ptr<Stmt> declaration()
    {
        try
        {
            if (check(TokenType::CLASS))
            {
                advance();
                return classDeclaration();
            }
            if (check(TokenType::FUN))
            {
                advance();
                return funDeclaration();
            }
            if (check(TokenType::VAR))
            {
                advance();
                return varDeclaration();
            }
            return statement();
        }
        catch (const std::runtime_error&)
        {
            synchronize();
            return nullptr;
        }
    }

    std::unique_ptr<Stmt> classDeclaration()
    {
        if (!check(TokenType::IDENTIFIER))
        {
            throw error(peek(), "Expect class name.");
        }
        Token name = advance();

        if (!check(TokenType::LEFT_BRACE))
        {
            throw error(peek(), "Expect '{' before class body.");
        }
        advance();

        // For now, class body is empty (methods will come in a later stage)
        if (!check(TokenType::RIGHT_BRACE))
        {
            throw error(peek(), "Expect '}' after class body.");
        }
        advance();

        return std::make_unique<ClassStmt>(name);
    }

    std::unique_ptr<Stmt> funDeclaration()
    {
        if (!check(TokenType::IDENTIFIER))
        {
            throw error(peek(), "Expect function name.");
        }
        Token name = advance();

        if (!check(TokenType::LEFT_PAREN))
        {
            throw error(peek(), "Expect '(' after function name.");
        }
        advance();

        std::vector<Token> params;
        if (!check(TokenType::RIGHT_PAREN))
        {
            do
            {
                if (params.size() >= 255)
                {
                    error(peek(), "Can't have more than 255 parameters.");
                }
                if (!check(TokenType::IDENTIFIER))
                {
                    throw error(peek(), "Expect parameter name.");
                }
                params.push_back(advance());
            } while (check(TokenType::COMMA) && (advance(), true));
        }

        if (!check(TokenType::RIGHT_PAREN))
        {
            throw error(peek(), "Expect ')' after parameters.");
        }
        advance();

        if (!check(TokenType::LEFT_BRACE))
        {
            throw error(peek(), "Expect '{' before function body.");
        }
        advance();

        // 复用 blockStatement 的内部逻辑来解析函数体
        std::vector<std::unique_ptr<Stmt>> body;
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
        {
            auto s = declaration();
            if (s) body.push_back(std::move(s));
        }
        if (!check(TokenType::RIGHT_BRACE))
        {
            throw error(peek(), "Expect '}' after function body.");
        }
        advance();

        return std::make_unique<FunctionStmt>(name, std::move(params), std::move(body));
    }

    std::unique_ptr<Stmt> varDeclaration()
    {
        if (!check(TokenType::IDENTIFIER))
        {
            throw error(peek(), "Expect variable name.");
        }
        Token name = advance();

        std::unique_ptr<Expr> initializer = nullptr;
        if (check(TokenType::EQUAL))
        {
            advance();
            initializer = expression();
        }

        if (!check(TokenType::SEMICOLON))
        {
            throw error(peek(), "Expect ';' after variable declaration.");
        }
        advance();
        return std::make_unique<VarStmt>(name.lexeme, name.line, std::move(initializer));
    }

    std::unique_ptr<Stmt> statement()
    {
        if (check(TokenType::FOR))
        {
            advance();
            return forStatement();
        }
        if (check(TokenType::IF))
        {
            advance();
            return ifStatement();
        }
        if (check(TokenType::RETURN))
        {
            return returnStatement();
        }
        if (check(TokenType::WHILE))
        {
            advance();
            return whileStatement();
        }
        if (check(TokenType::LEFT_BRACE))
        {
            advance();
            return blockStatement();
        }
        if (check(TokenType::PRINT))
        {
            advance();
            return printStatement();
        }
        return expressionStatement();
    }

    std::unique_ptr<Stmt> forStatement()
    {
        if (!check(TokenType::LEFT_PAREN))
        {
            throw error(peek(), "Expect '(' after 'for'.");
        }
        advance();

        // Initializer
        std::unique_ptr<Stmt> initializer;
        if (check(TokenType::SEMICOLON))
        {
            advance();
            initializer = nullptr;
        }
        else if (check(TokenType::VAR))
        {
            advance();
            initializer = varDeclaration();
        }
        else
        {
            initializer = expressionStatement();
        }

        // Condition
        std::unique_ptr<Expr> condition = nullptr;
        if (!check(TokenType::SEMICOLON))
        {
            condition = expression();
        }
        if (!check(TokenType::SEMICOLON))
        {
            throw error(peek(), "Expect ';' after loop condition.");
        }
        advance();

        // Increment
        std::unique_ptr<Expr> increment = nullptr;
        if (!check(TokenType::RIGHT_PAREN))
        {
            increment = expression();
        }
        if (!check(TokenType::RIGHT_PAREN))
        {
            throw error(peek(), "Expect ')' after for clauses.");
        }
        advance();

        // Body
        auto body = statement();

        // Desugar: attach increment after body
        if (increment)
        {
            std::vector<std::unique_ptr<Stmt>> stmts;
            stmts.push_back(std::move(body));
            stmts.push_back(std::make_unique<ExpressionStmt>(std::move(increment)));
            body = std::make_unique<BlockStmt>(std::move(stmts));
        }

        // Desugar: wrap in while
        if (!condition)
        {
            condition = std::make_unique<LiteralExpr>("true", ValueType::BOOL);
        }
        body = std::make_unique<WhileStmt>(std::move(condition), std::move(body));

        // Desugar: wrap with initializer
        if (initializer)
        {
            std::vector<std::unique_ptr<Stmt>> stmts;
            stmts.push_back(std::move(initializer));
            stmts.push_back(std::move(body));
            body = std::make_unique<BlockStmt>(std::move(stmts));
        }

        return body;
    }

    std::unique_ptr<Stmt> whileStatement()
    {
        if (!check(TokenType::LEFT_PAREN))
        {
            throw error(peek(), "Expect '(' after 'while'.");
        }
        advance();

        auto condition = expression();

        if (!check(TokenType::RIGHT_PAREN))
        {
            throw error(peek(), "Expect ')' after condition.");
        }
        advance();

        auto body = statement();
        return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
    }

    std::unique_ptr<Stmt> ifStatement()
    {
        if (!check(TokenType::LEFT_PAREN))
        {
            throw error(peek(), "Expect '(' after 'if'.");
        }
        advance();

        auto condition = expression();

        if (!check(TokenType::RIGHT_PAREN))
        {
            throw error(peek(), "Expect ')' after if condition.");
        }
        advance();

        auto thenBranch = statement();
        std::unique_ptr<Stmt> elseBranch = nullptr;
        if (check(TokenType::ELSE))
        {
            advance();
            elseBranch = statement();
        }

        return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
    }

    std::unique_ptr<Stmt> blockStatement()
    {
        std::vector<std::unique_ptr<Stmt>> stmts;
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
        {
            auto s = declaration();
            if (s) stmts.push_back(std::move(s));
        }
        if (!check(TokenType::RIGHT_BRACE))
        {
            throw error(peek(), "Expect '}' .");
        }
        advance();
        return std::make_unique<BlockStmt>(std::move(stmts));
    }

    std::unique_ptr<Stmt> printStatement()
    {
        auto expr = expression();
        if (!check(TokenType::SEMICOLON))
        {
            throw error(peek(), "Expect ';' after value.");
        }
        advance();
        return std::make_unique<PrintStmt>(std::move(expr));
    }

    std::unique_ptr<Stmt> expressionStatement()
    {
        auto expr = expression();
        if (!check(TokenType::SEMICOLON))
        {
            throw error(peek(), "Expect ';' after expression.");
        }
        advance();
        return std::make_unique<ExpressionStmt>(std::move(expr));
    }

    std::unique_ptr<Stmt> returnStatement()
    {
        Token keyword = advance(); // consume 'return'
        std::unique_ptr<Expr> value = nullptr;
        if (!check(TokenType::SEMICOLON))
        {
            value = expression();
        }
        if (!check(TokenType::SEMICOLON))
        {
            throw error(peek(), "Expect ';' after return value.");
        }
        advance();
        return std::make_unique<ReturnStmt>(keyword, std::move(value));
    }
};

// ============ Resolver ============

class Resolver
{
public:
    bool hasError() const { return hasError_; }

    void resolve(const std::vector<std::unique_ptr<Stmt>>& statements)
    {
        for (const auto& stmt : statements)
        {
            resolveStmt(stmt.get());
        }
    }

private:
    // Each scope maps variable name -> true (defined) / false (declared but not yet defined)
    std::vector<std::map<std::string, bool>> scopes_;
    bool hasError_ = false;
    int inFunction_ = 0;

    void beginScope()
    {
        scopes_.push_back(std::map<std::string, bool>());
    }

    void endScope()
    {
        scopes_.pop_back();
    }

    void declare(const std::string& name, int line)
    {
        if (scopes_.empty()) return;
        if (scopes_.back().find(name) != scopes_.back().end())
        {
            std::cerr << "[line " << line << "] Error at '" << name
                      << "': Already a variable with this name in this scope." << std::endl;
            hasError_ = true;
        }
        scopes_.back()[name] = false;
    }

    void define(const std::string& name)
    {
        if (scopes_.empty()) return;
        scopes_.back()[name] = true;
    }

    void resolveLocal(VariableExpr* expr)
    {
        for (int i = static_cast<int>(scopes_.size()) - 1; i >= 0; i--)
        {
            auto it = scopes_[i].find(expr->name);
            if (it != scopes_[i].end())
            {
                if (i == static_cast<int>(scopes_.size()) - 1 && it->second == false)
                {
                    std::cerr << "[line " << expr->line << "] Error at '" << expr->name
                              << "': Can't read local variable in its own initializer." << std::endl;
                    hasError_ = true;
                }
                expr->resolvedDistance = static_cast<int>(scopes_.size()) - 1 - i;
                return;
            }
        }
        // Not found in any scope => global variable, leave resolvedDistance = -1
    }

    void resolveLocalAssign(AssignExpr* expr)
    {
        for (int i = static_cast<int>(scopes_.size()) - 1; i >= 0; i--)
        {
            if (scopes_[i].find(expr->name) != scopes_[i].end())
            {
                expr->resolvedDistance = static_cast<int>(scopes_.size()) - 1 - i;
                return;
            }
        }
        // Not found in any scope => global variable, leave resolvedDistance = -1
    }

    void resolveStmt(const Stmt* stmt)
    {
        if (auto s = dynamic_cast<const PrintStmt*>(stmt))
        {
            resolveExpr(s->expr.get());
        }
        else if (auto s = dynamic_cast<const ExpressionStmt*>(stmt))
        {
            resolveExpr(s->expr.get());
        }
        else if (auto s = dynamic_cast<const VarStmt*>(stmt))
        {
            declare(s->name, s->line);
            if (s->initializer)
            {
                resolveExpr(s->initializer.get());
            }
            define(s->name);
        }
        else if (auto s = dynamic_cast<const BlockStmt*>(stmt))
        {
            beginScope();
            resolve(s->statements);
            endScope();
        }
        else if (auto s = dynamic_cast<const IfStmt*>(stmt))
        {
            resolveExpr(s->condition.get());
            resolveStmt(s->thenBranch.get());
            if (s->elseBranch) resolveStmt(s->elseBranch.get());
        }
        else if (auto s = dynamic_cast<const WhileStmt*>(stmt))
        {
            resolveExpr(s->condition.get());
            resolveStmt(s->body.get());
        }
        else if (auto s = dynamic_cast<const FunctionStmt*>(stmt))
        {
            declare(s->name.lexeme, s->name.line);
            define(s->name.lexeme);
            resolveFunction(s);
        }
        else if (auto s = dynamic_cast<const ClassStmt*>(stmt))
        {
            declare(s->name.lexeme, s->name.line);
            define(s->name.lexeme);
        }
        else if (auto s = dynamic_cast<const ReturnStmt*>(stmt))
        {
            if (inFunction_ == 0)
            {
                std::cerr << "[line " << s->keyword.line << "] Error at 'return': Can't return from top-level code." << std::endl;
                hasError_ = true;
            }
            if (s->value) resolveExpr(s->value.get());
        }
    }

    void resolveFunction(const FunctionStmt* func)
    {
        inFunction_++;
        beginScope();
        for (const auto& param : func->params)
        {
            declare(param.lexeme, param.line);
            define(param.lexeme);
        }
        resolve(func->body);
        endScope();
        inFunction_--;
    }

    void resolveExpr(Expr* expr)
    {
        if (auto e = dynamic_cast<VariableExpr*>(expr))
        {
            resolveLocal(e);
        }
        else if (auto e = dynamic_cast<AssignExpr*>(expr))
        {
            resolveExpr(e->value.get());
            resolveLocalAssign(e);
        }
        else if (auto e = dynamic_cast<BinaryExpr*>(expr))
        {
            resolveExpr(e->left.get());
            resolveExpr(e->right.get());
        }
        else if (auto e = dynamic_cast<UnaryExpr*>(expr))
        {
            resolveExpr(e->right.get());
        }
        else if (auto e = dynamic_cast<GroupExpr*>(expr))
        {
            resolveExpr(e->expr.get());
        }
        else if (auto e = dynamic_cast<LogicalExpr*>(expr))
        {
            resolveExpr(e->left.get());
            resolveExpr(e->right.get());
        }
        else if (auto e = dynamic_cast<CallExpr*>(expr))
        {
            resolveExpr(e->callee.get());
            for (const auto& arg : e->arguments)
            {
                resolveExpr(arg.get());
            }
        }
        else if (auto e = dynamic_cast<GetExpr*>(expr))
        {
            resolveExpr(e->object.get());
        }
        else if (auto e = dynamic_cast<SetExpr*>(expr))
        {
            resolveExpr(e->value.get());
            resolveExpr(e->object.get());
        }
        // LiteralExpr: nothing to resolve
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
        std::unique_ptr<Expr> expr;
        try
        {
            expr = parser.parse();
        }
        catch (const ParseError&)
        {
            // error already reported
        }
        if (parser.hasError()) return 65;
        if (expr)
        {
            std::cout << expr->print() << std::endl;
        }
    }
    else if (command == "evaluate")
    {
        std::string file_contents = read_file_contents(argv[2]);
        Scanner scanner(file_contents);
        auto tokens = scanner.scanTokens();
        if (scanner.hasError()) return 65;

        Parser parser(tokens);
        std::unique_ptr<Expr> expr;
        try
        {
            expr = parser.parse();
        }
        catch (const ParseError&)
        {
            // error already reported
        }
        if (parser.hasError()) return 65;
        if (expr)
        {
            try
            {
                auto env = std::make_shared<Environment>();
                globalEnvironment = env;
                LoxValue result = expr->evaluate(env);
                std::cout << result.toString() << std::endl;
            }
            catch (const RuntimeError& e)
            {
                std::cerr << e.what() << std::endl;
                std::cerr << "[line " << e.line << "]" << std::endl;
                return 70;
            }
        }
    }
    else if (command == "run")
    {
        std::string file_contents = read_file_contents(argv[2]);
        Scanner scanner(file_contents);
        auto tokens = scanner.scanTokens();
        if (scanner.hasError()) return 65;

        Parser parser(tokens);
        auto stmts = parser.parseStatements();
        if (parser.hasError()) return 65;

        // Resolve variable bindings
        Resolver resolver;
        resolver.resolve(stmts);
        if (resolver.hasError()) return 65;

        try
        {
            auto env = std::make_shared<Environment>();
            globalEnvironment = env;
            env->define("clock", LoxValue::Callable(std::make_shared<ClockNative>()));
            for (const auto& stmt : stmts)
            {
                stmt->execute(env);
            }
        }
        catch (const RuntimeError& e)
        {
            std::cerr << e.what() << std::endl;
            std::cerr << "[line " << e.line << "]" << std::endl;
            return 70;
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
