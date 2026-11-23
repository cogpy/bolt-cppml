#include "bolt/drawkern/yacc_grammar.hpp"

#include <algorithm>
#include <cctype>
#include <exception>
#include <iostream>
#include <sstream>

#include "bolt/drawkern/dis_vm.hpp"
#include "bolt/drawkern/drawkern.hpp"

namespace bolt {
namespace drawkern {

namespace {

// Tokenizes and parses a glyph description, leaving the lexer and parser it creates in the given owners
// (so their errors can be collected afterwards), and returns the parsed program.
std::unique_ptr<ProgramNode> lex_and_parse(const std::string& description, std::unique_ptr<GlyphLexer>& lexer,
                                           std::unique_ptr<GlyphParser>& parser) {
    lexer = std::make_unique<GlyphLexer>(description);
    auto tokens = lexer->tokenize();

    parser = std::make_unique<GlyphParser>(tokens);
    return parser->parse();
}

}  // namespace

// ASTNode implementation
std::string ASTNode::to_string() const {
    std::string result;
    switch (type) {
        case ASTNodeType::PROGRAM:
            result = "Program";
            break;
        case ASTNodeType::VM_DECL:
            result = "VMDecl";
            break;
        case ASTNodeType::GLYPH_DECL:
            result = "GlyphDecl";
            break;
        case ASTNodeType::PROPERTY:
            result = "Property";
            break;
        case ASTNodeType::BLOCK:
            result = "Block";
            break;
        case ASTNodeType::EXPRESSION:
            result = "Expression";
            break;
        case ASTNodeType::IDENTIFIER:
            result = "Identifier";
            break;
        case ASTNodeType::LITERAL:
            result = "Literal";
            break;
        case ASTNodeType::LIST:
            result = "List";
            break;
        case ASTNodeType::CALL:
            result = "Call";
            break;
    }

    if (!attributes.empty()) {
        result += " {";
        for (const auto& [key, value] : attributes) {
            result += " " + key + "=" + value;
        }
        result += " }";
    }

    return result;
}

// GlyphLexer implementation
GlyphLexer::GlyphLexer(const std::string& input) : input_(input), pos_(0), line_(1), column_(1) {}

GlyphLexer::~GlyphLexer() {}

std::vector<Token> GlyphLexer::tokenize() {
    std::vector<Token> tokens;
    errors_.clear();

    while (pos_ < input_.length()) {
        Token token = next_token();
        if (token.type != TokenType::UNKNOWN) {
            tokens.push_back(token);
        }

        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
    }

    return tokens;
}

Token GlyphLexer::next_token() {
    skip_whitespace();

    if (pos_ >= input_.length()) {
        return Token(TokenType::END_OF_FILE, "", line_, column_);
    }

    char ch = current_char();

    // Comments
    if (ch == '/' && peek_char() == '/') {
        skip_comment();
        return next_token();
    }

    // Single character tokens
    switch (ch) {
        case '=':
            advance();
            return Token(TokenType::ASSIGN, "=", line_, column_ - 1);
        case ':':
            advance();
            return Token(TokenType::COLON, ":", line_, column_ - 1);
        case ';':
            advance();
            return Token(TokenType::SEMICOLON, ";", line_, column_ - 1);
        case ',':
            advance();
            return Token(TokenType::COMMA, ",", line_, column_ - 1);
        case '{':
            advance();
            return Token(TokenType::LBRACE, "{", line_, column_ - 1);
        case '}':
            advance();
            return Token(TokenType::RBRACE, "}", line_, column_ - 1);
        case '(':
            advance();
            return Token(TokenType::LPAREN, "(", line_, column_ - 1);
        case ')':
            advance();
            return Token(TokenType::RPAREN, ")", line_, column_ - 1);
        case '[':
            advance();
            return Token(TokenType::LBRACKET, "[", line_, column_ - 1);
        case ']':
            advance();
            return Token(TokenType::RBRACKET, "]", line_, column_ - 1);
    }

    // String literals
    if (ch == '"') {
        return read_string();
    }

    // Numbers
    if (is_digit(ch)) {
        return read_number();
    }

    // Identifiers and keywords
    if (is_alpha(ch) || ch == '_') {
        return read_identifier();
    }

    add_error("Unexpected character: " + std::string(1, ch));
    advance();
    return Token(TokenType::UNKNOWN, std::string(1, ch), line_, column_ - 1);
}

char GlyphLexer::current_char() {
    if (pos_ >= input_.length())
        return '\0';
    return input_[pos_];
}

char GlyphLexer::peek_char() {
    if (pos_ + 1 >= input_.length())
        return '\0';
    return input_[pos_ + 1];
}

void GlyphLexer::advance() {
    if (pos_ < input_.length()) {
        if (input_[pos_] == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        pos_++;
    }
}

void GlyphLexer::skip_whitespace() {
    while (pos_ < input_.length() && std::isspace(current_char())) {
        advance();
    }
}

void GlyphLexer::skip_comment() {
    while (pos_ < input_.length() && current_char() != '\n') {
        advance();
    }
}

Token GlyphLexer::read_identifier() {
    size_t start_line = line_;
    size_t start_column = column_;
    std::string value;

    while (pos_ < input_.length() && (is_alnum(current_char()) || current_char() == '_')) {
        value += current_char();
        advance();
    }

    // Check for keywords
    TokenType type = TokenType::IDENTIFIER;
    if (value == "vm")
        type = TokenType::VM;
    else if (value == "glyph")
        type = TokenType::GLYPH;
    else if (value == "workbench")
        type = TokenType::WORKBENCH;
    else if (value == "dis")
        type = TokenType::DIS;
    else if (value == "native")
        type = TokenType::NATIVE;
    else if (value == "wasm")
        type = TokenType::WASM;
    else if (value == "render")
        type = TokenType::RENDER;
    else if (value == "ai")
        type = TokenType::AI;
    else if (value == "model")
        type = TokenType::MODEL;
    else if (value == "tools")
        type = TokenType::TOOLS;
    else if (value == "namespace")
        type = TokenType::NAMESPACE;

    return Token(type, value, start_line, start_column);
}

Token GlyphLexer::read_string() {
    size_t start_line = line_;
    size_t start_column = column_;
    std::string value;

    advance();  // Skip opening quote

    while (pos_ < input_.length() && current_char() != '"') {
        if (current_char() == '\\' && peek_char() != '\0') {
            advance();
            char escaped = current_char();
            switch (escaped) {
                case 'n':
                    value += '\n';
                    break;
                case 't':
                    value += '\t';
                    break;
                case 'r':
                    value += '\r';
                    break;
                case '\\':
                    value += '\\';
                    break;
                case '"':
                    value += '"';
                    break;
                default:
                    value += escaped;
                    break;
            }
        } else {
            value += current_char();
        }
        advance();
    }

    if (current_char() == '"') {
        advance();  // Skip closing quote
    } else {
        add_error("Unterminated string literal");
    }

    return Token(TokenType::STRING, value, start_line, start_column);
}

Token GlyphLexer::read_number() {
    size_t start_line = line_;
    size_t start_column = column_;
    std::string value;

    while (pos_ < input_.length() && (is_digit(current_char()) || current_char() == '.')) {
        value += current_char();
        advance();
    }

    return Token(TokenType::NUMBER, value, start_line, start_column);
}

bool GlyphLexer::is_alpha(char c) {
    return std::isalpha(c);
}

bool GlyphLexer::is_digit(char c) {
    return std::isdigit(c);
}

bool GlyphLexer::is_alnum(char c) {
    return std::isalnum(c);
}

void GlyphLexer::add_error(const std::string& message) {
    errors_.push_back("Line " + std::to_string(line_) + ", Column " + std::to_string(column_) + ": " + message);
}

// GlyphParser implementation
GlyphParser::GlyphParser(const std::vector<Token>& tokens) : tokens_(tokens), pos_(0) {}

GlyphParser::~GlyphParser() {}

std::unique_ptr<ProgramNode> GlyphParser::parse() {
    errors_.clear();
    pos_ = 0;

    auto program = std::make_unique<ProgramNode>();

    while (pos_ < tokens_.size() && current_token().type != TokenType::END_OF_FILE) {
        auto statement = parse_statement();
        if (statement) {
            program->add_child(std::move(statement));
        }
    }

    return program;
}

Token GlyphParser::current_token() {
    if (pos_ >= tokens_.size()) {
        return Token(TokenType::END_OF_FILE, "", 0, 0);
    }
    return tokens_[pos_];
}

Token GlyphParser::peek_token() {
    if (pos_ + 1 >= tokens_.size()) {
        return Token(TokenType::END_OF_FILE, "", 0, 0);
    }
    return tokens_[pos_ + 1];
}

void GlyphParser::advance() {
    if (pos_ < tokens_.size()) {
        pos_++;
    }
}

bool GlyphParser::match(TokenType type) {
    return current_token().type == type;
}

bool GlyphParser::consume(TokenType type) {
    if (match(type)) {
        advance();
        return true;
    }
    return false;
}

std::unique_ptr<ASTNode> GlyphParser::parse_statement() {
    if (match(TokenType::VM)) {
        return parse_vm_declaration();
    } else if (match(TokenType::GLYPH) || match(TokenType::WORKBENCH)) {
        return parse_glyph_declaration();
    } else if (match(TokenType::IDENTIFIER)) {
        return parse_property();
    } else {
        add_error("Expected statement, got " + current_token().value);
        advance();
        return nullptr;
    }
}

std::unique_ptr<VMDeclNode> GlyphParser::parse_vm_declaration() {
    if (!consume(TokenType::VM)) {
        return nullptr;
    }

    if (!match(TokenType::IDENTIFIER)) {
        add_error("Expected VM name");
        return nullptr;
    }

    std::string vm_id = current_token().value;
    advance();

    // The documented separator is ':' (vm <name> : <type>); '=' is the form the parser took before the
    // lexer had a ':' token and stays accepted.
    if (!consume(TokenType::COLON) && !consume(TokenType::ASSIGN)) {
        add_error("Expected ':' after VM name");
        return nullptr;
    }

    std::string vm_type;
    if (match(TokenType::DIS)) {
        vm_type = "dis";
        advance();
    } else if (match(TokenType::NATIVE)) {
        vm_type = "native";
        advance();
    } else if (match(TokenType::WASM)) {
        vm_type = "wasm";
        advance();
    } else {
        add_error("Expected VM type (dis, native, wasm)");
        return nullptr;
    }

    auto vm_node = std::make_unique<VMDeclNode>(vm_id, vm_type);

    if (consume(TokenType::LBRACE)) {
        auto block = parse_block(false);
        if (block) {
            vm_node->add_child(std::move(block));
        }
    }

    return vm_node;
}

std::unique_ptr<GlyphDeclNode> GlyphParser::parse_glyph_declaration() {
    bool is_workbench = match(TokenType::WORKBENCH);

    if (!(consume(TokenType::GLYPH) || consume(TokenType::WORKBENCH))) {
        return nullptr;
    }

    if (!match(TokenType::IDENTIFIER)) {
        add_error("Expected glyph name");
        return nullptr;
    }

    std::string glyph_id = current_token().value;
    advance();

    // Expect 'on' keyword (simplified - using identifier)
    if (!match(TokenType::IDENTIFIER) || current_token().value != "on") {
        add_error("Expected 'on' after glyph name");
        return nullptr;
    }
    advance();

    if (!match(TokenType::IDENTIFIER)) {
        add_error("Expected VM name after 'on'");
        return nullptr;
    }

    std::string base_vm = current_token().value;
    advance();

    auto glyph_node = std::make_unique<GlyphDeclNode>(glyph_id, base_vm);
    if (is_workbench) {
        glyph_node->set_attribute("type", "workbench");
    }

    if (consume(TokenType::LBRACE)) {
        auto block = parse_block(true);
        if (block) {
            glyph_node->add_child(std::move(block));
        }
    }

    return glyph_node;
}

bool GlyphParser::is_word(TokenType type) {
    switch (type) {
        case TokenType::IDENTIFIER:
        case TokenType::VM:
        case TokenType::GLYPH:
        case TokenType::WORKBENCH:
        case TokenType::DIS:
        case TokenType::NATIVE:
        case TokenType::WASM:
        case TokenType::RENDER:
        case TokenType::AI:
        case TokenType::MODEL:
        case TokenType::TOOLS:
        case TokenType::NAMESPACE:
            return true;
        default:
            return false;
    }
}

bool GlyphParser::is_value_start(TokenType type) {
    return type == TokenType::STRING || type == TokenType::NUMBER || is_word(type);
}

std::unique_ptr<PropertyNode> GlyphParser::parse_property() {
    // Keywords are ordinary names here: "model = ...;" and "tools = [...];" are properties.
    if (!is_word(current_token().type)) {
        return nullptr;
    }

    std::string name = current_token().value;
    advance();

    if (!consume(TokenType::ASSIGN)) {
        add_error("Expected '=' after property name");
        return nullptr;
    }

    std::unique_ptr<PropertyNode> property;
    if (is_value_start(current_token().type)) {
        property = std::make_unique<PropertyNode>(name, current_token().value);
        advance();
    } else if (match(TokenType::LBRACKET)) {
        auto list = parse_list();
        if (!list) {
            return nullptr;
        }
        // The value keeps its "[a,b]" text form; the items are also kept as a LIST child so that
        // an item containing a comma stays one item.
        std::string value = "[";
        for (size_t i = 0; i < list->children.size(); ++i) {
            value += (i ? "," : "") + list->children[i]->get_attribute("value");
        }
        value += "]";
        property = std::make_unique<PropertyNode>(name, value);
        property->add_child(std::move(list));
    } else {
        add_error("Expected property value");
        return nullptr;
    }

    consume(TokenType::SEMICOLON);  // Optional semicolon

    return property;
}

std::unique_ptr<ASTNode> GlyphParser::parse_mount() {
    advance();  // 'mount'

    if (!match(TokenType::STRING)) {
        add_error("Expected the path to mount, in quotes");
        return nullptr;
    }
    std::string source = current_token().value;
    advance();

    if (!match(TokenType::IDENTIFIER) || current_token().value != "as") {
        add_error("Expected 'as' after the mount path");
        return nullptr;
    }
    advance();

    if (!match(TokenType::STRING)) {
        add_error("Expected the mount target, in quotes");
        return nullptr;
    }
    std::string target = current_token().value;
    advance();

    consume(TokenType::SEMICOLON);  // Optional semicolon

    auto mount = std::make_unique<ASTNode>(ASTNodeType::CALL);
    mount->set_attribute("name", "mount");
    mount->set_attribute("source", source);
    mount->set_attribute("target", target);
    return mount;
}

void GlyphParser::recover_in_block(size_t item_start) {
    // Resume after the next ';', or at the '}' that closes the block.
    while (!match(TokenType::SEMICOLON) && !match(TokenType::RBRACE) && !match(TokenType::END_OF_FILE)) {
        advance();
    }
    consume(TokenType::SEMICOLON);

    if (pos_ == item_start && !match(TokenType::RBRACE)) {
        advance();  // Always make progress
    }
}

std::unique_ptr<ASTNode> GlyphParser::parse_section() {
    const TokenType kind = current_token().type;
    auto section = std::make_unique<ASTNode>(ASTNodeType::BLOCK);
    section->set_attribute("section", current_token().value);
    advance();  // 'render', 'ai' or 'namespace'
    advance();  // '{'

    while (!match(TokenType::RBRACE) && !match(TokenType::END_OF_FILE)) {
        const size_t item_start = pos_;
        std::unique_ptr<ASTNode> item;

        if (kind == TokenType::NAMESPACE) {
            if (match(TokenType::IDENTIFIER) && current_token().value == "mount") {
                item = parse_mount();
            } else {
                add_error("Expected 'mount' in namespace block, got " + current_token().value);
            }
        } else if (is_word(current_token().type) && peek_token().type == TokenType::ASSIGN) {
            item = parse_property();
        } else {
            add_error("Expected property in " + section->get_attribute("section") + " block, got " +
                      current_token().value);
        }

        if (item) {
            section->add_child(std::move(item));
        } else {
            recover_in_block(item_start);
        }
    }

    if (!consume(TokenType::RBRACE)) {
        add_error("Expected '}' to close the " + section->get_attribute("section") + " block");
    }
    return section;
}

std::unique_ptr<ASTNode> GlyphParser::parse_block(bool allow_sections) {
    auto block = std::make_unique<ASTNode>(ASTNodeType::BLOCK);

    while (!match(TokenType::RBRACE) && !match(TokenType::END_OF_FILE)) {
        const size_t item_start = pos_;
        const TokenType type = current_token().type;
        const bool is_section = (type == TokenType::RENDER || type == TokenType::AI || type == TokenType::NAMESPACE) &&
                                peek_token().type == TokenType::LBRACE;
        std::unique_ptr<ASTNode> item;

        if (is_section && allow_sections) {
            item = parse_section();
        } else if (is_section) {
            add_error("A " + current_token().value + " block is only allowed inside a glyph or workbench");
        } else if (is_word(type) && peek_token().type == TokenType::ASSIGN) {
            item = parse_property();
        } else {
            add_error("Expected property, got " + current_token().value);
        }

        if (item) {
            block->add_child(std::move(item));
        } else {
            recover_in_block(item_start);
        }
    }

    if (!consume(TokenType::RBRACE)) {
        add_error("Expected '}'");
    }
    return block;
}

std::unique_ptr<ASTNode> GlyphParser::parse_expression() {
    // Simplified expression parsing
    auto expr = std::make_unique<ASTNode>(ASTNodeType::EXPRESSION);

    if (is_value_start(current_token().type)) {
        expr->set_attribute("value", current_token().value);
        advance();
    }

    return expr;
}

std::unique_ptr<ASTNode> GlyphParser::parse_list() {
    auto list = std::make_unique<ASTNode>(ASTNodeType::LIST);

    if (!consume(TokenType::LBRACKET)) {
        return nullptr;
    }

    if (!match(TokenType::RBRACKET)) {
        do {
            if (!is_value_start(current_token().type)) {
                add_error("Expected list item");
                return nullptr;
            }
            list->add_child(parse_expression());
        } while (consume(TokenType::COMMA));
    }

    if (!consume(TokenType::RBRACKET)) {
        add_error("Expected ',' or ']' in list");
        return nullptr;
    }
    return list;
}

void GlyphParser::add_error(const std::string& message) {
    Token token = current_token();
    errors_.push_back("Line " + std::to_string(token.line) + ", Column " + std::to_string(token.column) + ": " +
                      message);
}

// GlyphCodeGenerator implementation
GlyphCodeGenerator::GlyphCodeGenerator() {}

GlyphCodeGenerator::~GlyphCodeGenerator() {}

namespace {

// The BLOCK child that holds a declaration's body, or nullptr when it was declared without one.
const ASTNode* body_of(const ASTNode& declaration) {
    for (const auto& child : declaration.children) {
        if (child->type == ASTNodeType::BLOCK) {
            return child.get();
        }
    }
    return nullptr;
}

// The named "render", "ai" or "namespace" block inside a body, or nullptr.
const ASTNode* section_of(const ASTNode* body, const std::string& name) {
    if (!body) {
        return nullptr;
    }
    for (const auto& child : body->children) {
        if (child->type == ASTNodeType::BLOCK && child->get_attribute("section") == name) {
            return child.get();
        }
    }
    return nullptr;
}

// The property with the given name directly inside a block, or nullptr.
const PropertyNode* property_of(const ASTNode* block, const std::string& name) {
    if (!block) {
        return nullptr;
    }
    for (const auto& child : block->children) {
        if (child->type == ASTNodeType::PROPERTY) {
            const PropertyNode* prop = static_cast<const PropertyNode*>(child.get());
            if (prop->name == name) {
                return prop;
            }
        }
    }
    return nullptr;
}

// The items of a list property; a scalar property counts as a list of one.
std::vector<std::string> items_of(const PropertyNode& prop) {
    std::vector<std::string> items;
    for (const auto& child : prop.children) {
        if (child->type == ASTNodeType::LIST) {
            for (const auto& item : child->children) {
                items.push_back(item->get_attribute("value"));
            }
            return items;
        }
    }
    items.push_back(prop.value);
    return items;
}

// The first glyph or workbench declaration; a workbench is preferred when asked for.
const GlyphDeclNode* find_glyph(const ASTNode& ast, bool prefer_workbench) {
    const GlyphDeclNode* first = nullptr;
    for (const auto& child : ast.children) {
        if (child->type != ASTNodeType::GLYPH_DECL) {
            continue;
        }
        const GlyphDeclNode* glyph = static_cast<const GlyphDeclNode*>(child.get());
        if (prefer_workbench && glyph->get_attribute("type") == "workbench") {
            return glyph;
        }
        if (!first) {
            first = glyph;
        }
    }
    return first;
}

}  // namespace

bool GlyphCodeGenerator::read_int(const PropertyNode& prop, int32_t& out) {
    try {
        size_t used = 0;
        int value = std::stoi(prop.value, &used);
        if (used == prop.value.size()) {
            out = value;
            return true;
        }
    } catch (const std::exception&) {
        // Reported below.
    }
    add_error("Property '" + prop.name + "' needs a whole number, got '" + prop.value + "'");
    return false;
}

void GlyphCodeGenerator::apply_render_properties(const ASTNode* block, VMGlyph& glyph) {
    if (!block) {
        return;
    }
    for (const auto& child : block->children) {
        if (child->type != ASTNodeType::PROPERTY) {
            continue;
        }
        const PropertyNode& prop = static_cast<const PropertyNode&>(*child);

        if (prop.name == "width") {
            read_int(prop, glyph.render.width);
        } else if (prop.name == "height") {
            read_int(prop, glyph.render.height);
        } else if (prop.name == "interactive") {
            glyph.render.interactive = (prop.value == "true");
        } else if (prop.name == "ai_enabled") {
            glyph.render.ai_enabled = (prop.value == "true");
        } else if (prop.name == "font") {
            glyph.render.font_family = prop.value;
        } else if (prop.name == "background") {
            // "#rrggbb"
            const std::string hex = prop.value.empty() ? "" : prop.value.substr(1);
            const bool well_formed =
                prop.value.size() == 7 && prop.value[0] == '#' &&
                std::all_of(hex.begin(), hex.end(), [](unsigned char c) { return std::isxdigit(c); });
            if (well_formed) {
                glyph.render.background_color = static_cast<uint32_t>(std::stoul(hex, nullptr, 16));
            } else {
                add_error("Property 'background' needs a colour like \"#1e1e1e\", got '" + prop.value + "'");
            }
        }
    }
}

VMGlyph GlyphCodeGenerator::generate_vm_glyph(const ASTNode& ast) {
    errors_.clear();
    VMGlyph glyph;

    // Set defaults
    glyph.vm_type = "dis";
    glyph.architecture = "portable";
    glyph.render.width = 800;
    glyph.render.height = 600;
    glyph.render.background_color = 0x1e1e1e;
    glyph.render.interactive = true;
    glyph.render.ai_enabled = false;

    const GlyphDeclNode* glyph_decl = find_glyph(ast, false);

    // The VM the glyph runs on: the one it names, or the last one declared when there is no glyph.
    const VMDeclNode* vm_decl = nullptr;
    for (const auto& child : ast.children) {
        if (child->type == ASTNodeType::VM_DECL) {
            const VMDeclNode* candidate = static_cast<const VMDeclNode*>(child.get());
            if (!glyph_decl || candidate->vm_id == glyph_decl->base_vm) {
                vm_decl = candidate;
            }
        }
    }
    if (glyph_decl && !vm_decl) {
        add_error("'" + glyph_decl->glyph_id + "' runs on '" + glyph_decl->base_vm + "', which is not declared");
    }

    if (vm_decl) {
        glyph.vm_type = vm_decl->vm_type;
        const ASTNode* vm_body = body_of(*vm_decl);
        if (const PropertyNode* architecture = property_of(vm_body, "architecture")) {
            glyph.architecture = architecture->value;
        }
        if (const PropertyNode* capabilities = property_of(vm_body, "capabilities")) {
            glyph.capabilities = items_of(*capabilities);
        }
    }

    if (glyph_decl) {
        const ASTNode* body = body_of(*glyph_decl);
        if (const PropertyNode* architecture = property_of(body, "architecture")) {
            glyph.architecture = architecture->value;
        }

        // Rendering properties may sit directly in the body or in its render block; the block wins.
        apply_render_properties(body, glyph);
        apply_render_properties(section_of(body, "render"), glyph);

        if (const ASTNode* mounts = section_of(body, "namespace")) {
            for (const auto& mount : mounts->children) {
                if (mount->type == ASTNodeType::CALL) {
                    glyph.namespace_mounts[mount->get_attribute("source")] = mount->get_attribute("target");
                }
            }
        }
    }

    return glyph;
}

AIWorkbenchGlyph GlyphCodeGenerator::generate_ai_workbench(const ASTNode& ast) {
    AIWorkbenchGlyph workbench;

    workbench.workbench_id = "generated_workbench";
    workbench.ai_model = "ggml-rwkv";
    workbench.tools = {"completion", "chat"};
    workbench.styx_address = "tcp!*!9999";

    // Generate host VM
    workbench.host_vm = generate_vm_glyph(ast);
    workbench.host_vm.render.ai_enabled = true;

    if (const GlyphDeclNode* decl = find_glyph(ast, true)) {
        workbench.workbench_id = decl->glyph_id;

        const ASTNode* body = body_of(*decl);
        // "model" in the body names the workbench's model; the one in the ai block is the fallback.
        if (const PropertyNode* model = property_of(body, "model")) {
            workbench.ai_model = model->value;
        } else if (const PropertyNode* ai_model = property_of(section_of(body, "ai"), "model")) {
            workbench.ai_model = ai_model->value;
        }
        if (const PropertyNode* tools = property_of(body, "tools")) {
            workbench.tools = items_of(*tools);
        }
    }

    return workbench;
}

DISProgram GlyphCodeGenerator::generate_dis_program(const ASTNode& ast) {
    DISProgram program;

    // Generate a simple AI workbench program
    program.add_instruction(DISInstruction(DISOpcode::NEWSTR, DISValue("🤖 AI Workbench starting...\n")));
    program.add_instruction(DISInstruction(DISOpcode::PRINT));

    program.add_instruction(DISInstruction(DISOpcode::NEWSTR, DISValue("ggml-rwkv")));
    program.add_instruction(DISInstruction(DISOpcode::AI_INIT));

    program.add_instruction(DISInstruction(DISOpcode::NEWSTR, DISValue("Hello from generated workbench!")));
    program.add_instruction(DISInstruction(DISOpcode::AI_CHAT));
    program.add_instruction(DISInstruction(DISOpcode::PRINT));

    program.add_instruction(DISInstruction(DISOpcode::HALT));

    return program;
}

std::string GlyphCodeGenerator::generate_cpp_code(const ASTNode& ast) {
    std::ostringstream code;

    code << "#include \"bolt/drawkern/drawkern.hpp\"\n";
    code << "#include \"bolt/drawkern/dis_vm.hpp\"\n\n";
    code << "using namespace bolt::drawkern;\n\n";
    code << "AIWorkbenchGlyph create_generated_workbench() {\n";
    code << "    AIWorkbenchGlyph workbench;\n";
    code << "    workbench.workbench_id = \"generated_workbench\";\n";
    code << "    workbench.ai_model = \"ggml-rwkv\";\n";
    code << "    workbench.tools = {\"completion\", \"chat\"};\n";
    code << "    \n";
    code << "    // Configure VM\n";
    code << "    workbench.host_vm.vm_type = \"dis\";\n";
    code << "    workbench.host_vm.architecture = \"portable\";\n";
    code << "    workbench.host_vm.render.width = 1200;\n";
    code << "    workbench.host_vm.render.height = 800;\n";
    code << "    workbench.host_vm.render.ai_enabled = true;\n";
    code << "    \n";
    code << "    return workbench;\n";
    code << "}\n";

    return code.str();
}

std::string GlyphCodeGenerator::generate_json(const ASTNode& ast) {
    std::ostringstream json;

    json << "{\n";
    json << "  \"workbench_id\": \"generated_workbench\",\n";
    json << "  \"ai_model\": \"ggml-rwkv\",\n";
    json << "  \"tools\": [\"completion\", \"chat\"],\n";
    json << "  \"host_vm\": {\n";
    json << "    \"vm_type\": \"dis\",\n";
    json << "    \"architecture\": \"portable\",\n";
    json << "    \"render\": {\n";
    json << "      \"width\": 1200,\n";
    json << "      \"height\": 800,\n";
    json << "      \"ai_enabled\": true\n";
    json << "    }\n";
    json << "  }\n";
    json << "}\n";

    return json.str();
}

std::string GlyphCodeGenerator::generate_yaml(const ASTNode& ast) {
    std::ostringstream yaml;

    yaml << "workbench_id: generated_workbench\n";
    yaml << "ai_model: ggml-rwkv\n";
    yaml << "tools:\n";
    yaml << "  - completion\n";
    yaml << "  - chat\n";
    yaml << "host_vm:\n";
    yaml << "  vm_type: dis\n";
    yaml << "  architecture: portable\n";
    yaml << "  render:\n";
    yaml << "    width: 1200\n";
    yaml << "    height: 800\n";
    yaml << "    ai_enabled: true\n";

    return yaml.str();
}

void GlyphCodeGenerator::add_error(const std::string& message) {
    errors_.push_back(message);
}

// YaccGrammarSystem implementation
YaccGrammarSystem::YaccGrammarSystem() {}

YaccGrammarSystem::~YaccGrammarSystem() {}

VMGlyph YaccGrammarSystem::parse_vm_glyph(const std::string& description) {
    clear_errors();

    auto ast = lex_and_parse(description, lexer_, parser_);

    generator_ = std::make_unique<GlyphCodeGenerator>();
    VMGlyph glyph = generator_->generate_vm_glyph(*ast);

    collect_errors();
    return glyph;
}

AIWorkbenchGlyph YaccGrammarSystem::parse_ai_workbench(const std::string& description) {
    clear_errors();

    auto ast = lex_and_parse(description, lexer_, parser_);

    generator_ = std::make_unique<GlyphCodeGenerator>();
    AIWorkbenchGlyph workbench = generator_->generate_ai_workbench(*ast);

    collect_errors();
    return workbench;
}

bool YaccGrammarSystem::validate_description(const std::string& description) {
    clear_errors();

    // Validation runs no generator, so drop the one an earlier parse or generate call left behind;
    // otherwise collect_errors() would report that call's errors as this description's.
    generator_.reset();

    auto ast = lex_and_parse(description, lexer_, parser_);

    collect_errors();
    return errors_.empty();
}

std::vector<std::string> YaccGrammarSystem::get_validation_errors() const {
    return errors_;
}

std::string YaccGrammarSystem::generate_cpp_code(const std::string& description) {
    clear_errors();

    auto ast = lex_and_parse(description, lexer_, parser_);

    generator_ = std::make_unique<GlyphCodeGenerator>();
    std::string code = generator_->generate_cpp_code(*ast);

    collect_errors();
    return code;
}

std::string YaccGrammarSystem::generate_deployment_script(const std::string& description) {
    std::ostringstream script;

    script << "#!/bin/bash\n";
    script << "# Generated DrawKern deployment script\n\n";
    script << "echo \"Deploying DrawKern workbench...\"\n";
    script << "./drawkern_server --port 9999 &\n";
    script << "sleep 2\n";
    script << "./drawkern_client localhost:9999\n";

    return script.str();
}

std::string YaccGrammarSystem::get_ai_workbench_template() {
    return GlyphTemplates::AI_WORKBENCH;
}

std::string YaccGrammarSystem::get_file_server_template() {
    return GlyphTemplates::FILE_SERVER;
}

std::string YaccGrammarSystem::get_echo_server_template() {
    return GlyphTemplates::ECHO_SERVER;
}

std::vector<std::string> YaccGrammarSystem::get_supported_vm_types() {
    return {"dis", "native", "wasm"};
}

std::vector<std::string> YaccGrammarSystem::get_supported_properties() {
    return {"width",      "height", "background", "font",         "interactive",
            "ai_enabled", "model",  "tools",      "architecture", "capabilities"};
}

std::string YaccGrammarSystem::get_grammar_documentation() {
    return R"(
DrawKern VM Glyph Description Language Grammar:

Keywords:
  vm, glyph, workbench, dis, native, wasm, render, ai, model, tools, namespace

Basic Structure:
  vm <name> : <type> { properties... }
  glyph <name> on <vm> { properties... }
  workbench <name> on <vm> { properties... render { ... } ai { ... } namespace { ... } }

Blocks inside a glyph or workbench:
  render { properties... }
  ai { properties... }
  namespace { mount "<path>" as "<target>"; ... }

Properties:
  width = <number>;
  height = <number>;
  background = "<color>";
  font = "<font_name>";
  interactive = true|false;
  ai_enabled = true|false;
  model = "<model_name>";
  tools = ["tool1", "tool2"];
  architecture = "<arch>";

Example:
  vm my_vm : dis {
    architecture = "portable";
  }
  
  workbench my_workbench on my_vm {
    render { width = 1200; height = 800; }
    ai { model = "ggml-rwkv"; }
    namespace { mount "/ai/models" as "/srv/models"; }
  }
)";
}

void YaccGrammarSystem::clear_errors() {
    errors_.clear();
}

void YaccGrammarSystem::collect_errors() {
    if (lexer_ && lexer_->has_errors()) {
        auto lexer_errors = lexer_->get_errors();
        errors_.insert(errors_.end(), lexer_errors.begin(), lexer_errors.end());
    }

    if (parser_ && parser_->has_errors()) {
        auto parser_errors = parser_->get_errors();
        errors_.insert(errors_.end(), parser_errors.begin(), parser_errors.end());
    }

    if (generator_ && generator_->has_errors()) {
        auto generator_errors = generator_->get_errors();
        errors_.insert(errors_.end(), generator_errors.begin(), generator_errors.end());
    }
}

}  // namespace drawkern
}  // namespace bolt
