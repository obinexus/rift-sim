// ================================
// File: include/librift.h
// RIFT Library Header - Complete Stage Pipeline
// ================================

#ifndef LIBRIFT_H
#define LIBRIFT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

// ================================
// RIFT Governance and Configuration
// ================================

typedef struct {
    char* pattern;
    char* intention;
    char* sp_alignment;
} rift_config_entry_t;

typedef struct {
    rift_config_entry_t* entries;
    size_t count;
    size_t capacity;
} rift_governance_t;

// ================================
// RIFT-0: Tokenizer Stage
// ================================

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_WHITESPACE,
    TOKEN_UNKNOWN
} token_type_t;

typedef struct {
    token_type_t type;
    char* value;
    size_t line;
    size_t column;
} rift_token_t;

typedef struct {
    rift_token_t* tokens;
    size_t count;
    size_t capacity;
} token_stream_t;

typedef struct {
    char* pattern;
    token_type_t type;
    bool is_final;
    size_t id;
} rift_state_t;

typedef struct {
    rift_state_t** states;
    size_t state_count;
    rift_state_t* current_state;
    rift_governance_t* governance;
} rift_tokenizer_t;

// ================================
// RIFT-1: Parser Bridge Stage
// ================================

typedef enum {
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_BINARY_OP,
    AST_UNARY_OP
} ast_node_type_t;

typedef struct ast_node {
    ast_node_type_t type;
    char* value;
    struct ast_node* left;
    struct ast_node* right;
} ast_node_t;

typedef struct {
    token_stream_t* input_tokens;
    size_t current_position;
    rift_governance_t* governance;
} rift_parser_t;

// ================================
// RIFT-2: AST Coordinator Stage
// ================================

typedef struct {
    ast_node_t* root;
    rift_governance_t* governance;
    size_t node_count;
    size_t optimization_passes;
} rift_ast_coordinator_t;

// ================================
// RIFT-3: Output Stage
// ================================

typedef struct {
    ast_node_t* ast;
    rift_governance_t* governance;
    char* output_format;
} rift_output_stage_t;

// ================================
// Function Declarations
// ================================

// Governance functions
rift_governance_t* rift_load_governance(const char* config_dir);
void rift_governance_destroy(rift_governance_t* gov);
const char* rift_get_config_value(rift_governance_t* gov, const char* key);

// RIFT-0 functions
rift_tokenizer_t* rift_tokenizer_create(rift_governance_t* gov);
void rift_tokenizer_destroy(rift_tokenizer_t* tokenizer);
token_stream_t* rift_tokenize(rift_tokenizer_t* tokenizer, const char* input);
void token_stream_destroy(token_stream_t* stream);

// RIFT-1 functions
rift_parser_t* rift_parser_create(rift_governance_t* gov);
void rift_parser_destroy(rift_parser_t* parser);
ast_node_t* rift_parse(rift_parser_t* parser, token_stream_t* tokens);

// RIFT-2 functions
rift_ast_coordinator_t* rift_ast_coordinator_create(rift_governance_t* gov);
void rift_ast_coordinator_destroy(rift_ast_coordinator_t* coordinator);
ast_node_t* rift_coordinate_ast(rift_ast_coordinator_t* coordinator, ast_node_t* ast);

// RIFT-3 functions
rift_output_stage_t* rift_output_stage_create(rift_governance_t* gov);
void rift_output_stage_destroy(rift_output_stage_t* output);
void rift_generate_output(rift_output_stage_t* output, ast_node_t* ast);

// Utility functions
void ast_node_destroy(ast_node_t* node);
void rift_print_stage_info(const char* stage, const char* message);

#endif // LIBRIFT_H

// ================================
// File: main.c
// RIFT Complete Pipeline Simulation
// ================================

#include "include/librift.h"

// ================================
// Governance Implementation
// ================================

rift_governance_t* rift_load_governance(const char* config_dir) {
    rift_governance_t* gov = malloc(sizeof(rift_governance_t));
    if (!gov) return NULL;
    
    gov->capacity = 10;
    gov->count = 0;
    gov->entries = malloc(sizeof(rift_config_entry_t) * gov->capacity);
    
    rift_print_stage_info("GOVERNANCE", "Loading .rift configuration files");
    
    // Simulate loading tokenizer-config.rift with R"" syntax
    rift_config_entry_t* entry1 = &gov->entries[gov->count++];
    entry1->pattern = strdup(R"(^[a-zA-Z_]\w*$)");
    entry1->intention = strdup("IDENTIFIER_RECOGNITION");
    entry1->sp_alignment = strdup("STAGE_0_TOKENIZER");
    
    // Simulate loading intention-map.rift with R'' syntax
    rift_config_entry_t* entry2 = &gov->entries[gov->count++];
    entry2->pattern = strdup(R'(^\d+$)');
    entry2->intention = strdup("NUMBER_RECOGNITION");
    entry2->sp_alignment = strdup("STAGE_0_TOKENIZER");
    
    rift_config_entry_t* entry3 = &gov->entries[gov->count++];
    entry3->pattern = strdup(R"(^[+\-*/]$)");
    entry3->intention = strdup("OPERATOR_RECOGNITION");
    entry3->sp_alignment = strdup("STAGE_0_TOKENIZER");
    
    printf("  → Loaded %zu configuration entries from .rift files\n", gov->count);
    return gov;
}

void rift_governance_destroy(rift_governance_t* gov) {
    if (!gov) return;
    
    for (size_t i = 0; i < gov->count; i++) {
        free(gov->entries[i].pattern);
        free(gov->entries[i].intention);
        free(gov->entries[i].sp_alignment);
    }
    free(gov->entries);
    free(gov);
}

const char* rift_get_config_value(rift_governance_t* gov, const char* key) {
    for (size_t i = 0; i < gov->count; i++) {
        if (strcmp(gov->entries[i].intention, key) == 0) {
            return gov->entries[i].pattern;
        }
    }
    return NULL;
}

// ================================
// RIFT-0: Tokenizer Implementation
// ================================

rift_tokenizer_t* rift_tokenizer_create(rift_governance_t* gov) {
    rift_tokenizer_t* tokenizer = malloc(sizeof(rift_tokenizer_t));
    if (!tokenizer) return NULL;
    
    tokenizer->governance = gov;
    tokenizer->state_count = 4;
    tokenizer->states = malloc(sizeof(rift_state_t*) * tokenizer->state_count);
    
    // Create states based on governance configuration
    for (size_t i = 0; i < tokenizer->state_count; i++) {
        tokenizer->states[i] = malloc(sizeof(rift_state_t));
        tokenizer->states[i]->id = i;
        tokenizer->states[i]->is_final = true;
    }
    
    // Configure states from governance
    tokenizer->states[0]->pattern = strdup(rift_get_config_value(gov, "IDENTIFIER_RECOGNITION"));
    tokenizer->states[0]->type = TOKEN_IDENTIFIER;
    
    tokenizer->states[1]->pattern = strdup(rift_get_config_value(gov, "NUMBER_RECOGNITION"));
    tokenizer->states[1]->type = TOKEN_NUMBER;
    
    tokenizer->states[2]->pattern = strdup(rift_get_config_value(gov, "OPERATOR_RECOGNITION"));
    tokenizer->states[2]->type = TOKEN_OPERATOR;
    
    tokenizer->states[3]->pattern = strdup(R"(^\s+$)");
    tokenizer->states[3]->type = TOKEN_WHITESPACE;
    
    return tokenizer;
}

void rift_tokenizer_destroy(rift_tokenizer_t* tokenizer) {
    if (!tokenizer) return;
    
    for (size_t i = 0; i < tokenizer->state_count; i++) {
        free(tokenizer->states[i]->pattern);
        free(tokenizer->states[i]);
    }
    free(tokenizer->states);
    free(tokenizer);
}

bool matches_pattern(const char* pattern, const char* text) {
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result) return false;
    
    result = regexec(&regex, text, 0, NULL, 0);
    regfree(&regex);
    return (result == 0);
}

token_stream_t* rift_tokenize(rift_tokenizer_t* tokenizer, const char* input) {
    rift_print_stage_info("RIFT-0", "DFA-based tokenization starting");
    
    token_stream_t* stream = malloc(sizeof(token_stream_t));
    stream->capacity = 10;
    stream->count = 0;
    stream->tokens = malloc(sizeof(rift_token_t) * stream->capacity);
    
    // Simple tokenization: split by spaces and classify each token
    char* input_copy = strdup(input);
    char* token_str = strtok(input_copy, " ");
    
    while (token_str != NULL) {
        // Skip whitespace tokens for this demo
        if (strlen(token_str) > 0 && token_str[0] != ' ') {
            rift_token_t* token = &stream->tokens[stream->count++];
            token->value = strdup(token_str);
            token->line = 1;
            token->column = stream->count;
            token->type = TOKEN_UNKNOWN;
            
            // Classify token using DFA states
            for (size_t i = 0; i < tokenizer->state_count; i++) {
                if (matches_pattern(tokenizer->states[i]->pattern, token_str)) {
                    token->type = tokenizer->states[i]->type;
                    break;
                }
            }
            
            const char* type_name = "UNKNOWN";
            switch (token->type) {
                case TOKEN_IDENTIFIER: type_name = "IDENTIFIER"; break;
                case TOKEN_NUMBER: type_name = "NUMBER"; break;
                case TOKEN_OPERATOR: type_name = "OPERATOR"; break;
                case TOKEN_WHITESPACE: type_name = "WHITESPACE"; break;
                default: break;
            }
            
            printf("  → Token: '%s' classified as %s\n", token_str, type_name);
        }
        
        token_str = strtok(NULL, " ");
    }
    
    free(input_copy);
    printf("  → Tokenization complete: %zu tokens generated\n", stream->count);
    return stream;
}

void token_stream_destroy(token_stream_t* stream) {
    if (!stream) return;
    
    for (size_t i = 0; i < stream->count; i++) {
        free(stream->tokens[i].value);
    }
    free(stream->tokens);
    free(stream);
}

// ================================
// RIFT-1: Parser Implementation
// ================================

rift_parser_t* rift_parser_create(rift_governance_t* gov) {
    rift_parser_t* parser = malloc(sizeof(rift_parser_t));
    if (!parser) return NULL;
    
    parser->governance = gov;
    parser->current_position = 0;
    return parser;
}

void rift_parser_destroy(rift_parser_t* parser) {
    if (parser) free(parser);
}

ast_node_t* create_ast_node(ast_node_type_t type, const char* value) {
    ast_node_t* node = malloc(sizeof(ast_node_t));
    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->left = NULL;
    node->right = NULL;
    return node;
}

ast_node_t* parse_expression(rift_parser_t* parser);
ast_node_t* parse_term(rift_parser_t* parser);
ast_node_t* parse_factor(rift_parser_t* parser);

rift_token_t* current_token(rift_parser_t* parser) {
    if (parser->current_position >= parser->input_tokens->count) return NULL;
    return &parser->input_tokens->tokens[parser->current_position];
}

void advance_token(rift_parser_t* parser) {
    if (parser->current_position < parser->input_tokens->count) {
        parser->current_position++;
    }
}

ast_node_t* parse_factor(rift_parser_t* parser) {
    rift_token_t* token = current_token(parser);
    if (!token) return NULL;
    
    if (token->type == TOKEN_IDENTIFIER) {
        advance_token(parser);
        return create_ast_node(AST_IDENTIFIER, token->value);
    } else if (token->type == TOKEN_NUMBER) {
        advance_token(parser);
        return create_ast_node(AST_NUMBER, token->value);
    }
    
    return NULL;
}

ast_node_t* parse_term(rift_parser_t* parser) {
    ast_node_t* left = parse_factor(parser);
    
    while (current_token(parser) && 
           current_token(parser)->type == TOKEN_OPERATOR &&
           (strcmp(current_token(parser)->value, "*") == 0 || 
            strcmp(current_token(parser)->value, "/") == 0)) {
        
        char* op = strdup(current_token(parser)->value);
        advance_token(parser);
        
        ast_node_t* right = parse_factor(parser);
        ast_node_t* binary_op = create_ast_node(AST_BINARY_OP, op);
        binary_op->left = left;
        binary_op->right = right;
        left = binary_op;
        free(op);
    }
    
    return left;
}

ast_node_t* parse_expression(rift_parser_t* parser) {
    ast_node_t* left = parse_term(parser);
    
    while (current_token(parser) && 
           current_token(parser)->type == TOKEN_OPERATOR &&
           (strcmp(current_token(parser)->value, "+") == 0 || 
            strcmp(current_token(parser)->value, "-") == 0)) {
        
        char* op = strdup(current_token(parser)->value);
        advance_token(parser);
        
        ast_node_t* right = parse_term(parser);
        ast_node_t* binary_op = create_ast_node(AST_BINARY_OP, op);
        binary_op->left = left;
        binary_op->right = right;
        left = binary_op;
        free(op);
    }
    
    return left;
}

ast_node_t* rift_parse(rift_parser_t* parser, token_stream_t* tokens) {
    rift_print_stage_info("RIFT-1", "Parsing token stream to AST");
    
    parser->input_tokens = tokens;
    parser->current_position = 0;
    
    ast_node_t* ast = parse_expression(parser);
    
    printf("  → Parsing complete: AST root created\n");
    return ast;
}

// ================================
// RIFT-2: AST Coordinator Implementation
// ================================

rift_ast_coordinator_t* rift_ast_coordinator_create(rift_governance_t* gov) {
    rift_ast_coordinator_t* coordinator = malloc(sizeof(rift_ast_coordinator_t));
    if (!coordinator) return NULL;
    
    coordinator->governance = gov;
    coordinator->node_count = 0;
    coordinator->optimization_passes = 1;
    return coordinator;
}

void rift_ast_coordinator_destroy(rift_ast_coordinator_t* coordinator) {
    if (coordinator) free(coordinator);
}

size_t count_ast_nodes(ast_node_t* node) {
    if (!node) return 0;
    return 1 + count_ast_nodes(node->left) + count_ast_nodes(node->right);
}

ast_node_t* rift_coordinate_ast(rift_ast_coordinator_t* coordinator, ast_node_t* ast) {
    rift_print_stage_info("RIFT-2", "Coordinating and optimizing AST");
    
    coordinator->root = ast;
    coordinator->node_count = count_ast_nodes(ast);
    
    printf("  → AST contains %zu nodes\n", coordinator->node_count);
    printf("  → Applying %zu optimization passes\n", coordinator->optimization_passes);
    printf("  → AST coordination complete\n");
    
    return ast;
}

// ================================
// RIFT-3: Output Stage Implementation
// ================================

rift_output_stage_t* rift_output_stage_create(rift_governance_t* gov) {
    rift_output_stage_t* output = malloc(sizeof(rift_output_stage_t));
    if (!output) return NULL;
    
    output->governance = gov;
    output->output_format = strdup("LISP_STYLE_AST");
    return output;
}

void rift_output_stage_destroy(rift_output_stage_t* output) {
    if (!output) return;
    free(output->output_format);
    free(output);
}

void print_ast_recursive(ast_node_t* node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case AST_IDENTIFIER:
            printf("(Identifier %s)\n", node->value);
            break;
        case AST_NUMBER:
            printf("(Number %s)\n", node->value);
            break;
        case AST_BINARY_OP:
            printf("(BinOp %s\n", node->value);
            print_ast_recursive(node->left, indent + 1);
            print_ast_recursive(node->right, indent + 1);
            for (int i = 0; i < indent; i++) printf("  ");
            printf(")\n");
            break;
        default:
            printf("(Unknown)\n");
            break;
    }
}

void rift_generate_output(rift_output_stage_t* output, ast_node_t* ast) {
    rift_print_stage_info("RIFT-3", "Generating final output");
    
    output->ast = ast;
    
    printf("  → Output format: %s\n", output->output_format);
    printf("  → Final AST structure:\n");
    printf("(AST\n");
    print_ast_recursive(ast, 1);
    printf(")\n");
}

// ================================
// Utility Functions
// ================================

void rift_print_stage_info(const char* stage, const char* message) {
    printf("\n[%s] %s\n", stage, message);
}

void ast_node_destroy(ast_node_t* node) {
    if (!node) return;
    
    ast_node_destroy(node->left);
    ast_node_destroy(node->right);
    free(node->value);
    free(node);
}

// ================================
// Main Execution Pipeline
// ================================

int main(void) {
    printf("RIFT Complete Pipeline Simulation\n");
    printf("==================================\n");
    printf("Executing RIFT-0 → RIFT-1 → RIFT-2 → RIFT-3\n");
    
    // Initialize governance from .rift configuration files
    rift_governance_t* governance = rift_load_governance("rift-gov/");
    if (!governance) {
        fprintf(stderr, "Failed to load RIFT governance configuration\n");
        return 1;
    }
    
    // Test input: mathematical expression
    const char* source_input = "x + 2 * y";
    printf("\nProcessing input: \"%s\"\n", source_input);
    
    // RIFT-0: Tokenization Stage
    rift_tokenizer_t* tokenizer = rift_tokenizer_create(governance);
    token_stream_t* tokens = rift_tokenize(tokenizer, source_input);
    
    // RIFT-1: Parser Bridge Stage
    rift_parser_t* parser = rift_parser_create(governance);
    ast_node_t* ast = rift_parse(parser, tokens);
    
    // RIFT-2: AST Coordinator Stage
    rift_ast_coordinator_t* coordinator = rift_ast_coordinator_create(governance);
    ast_node_t* coordinated_ast = rift_coordinate_ast(coordinator, ast);
    
    // RIFT-3: Output Stage
    rift_output_stage_t* output_stage = rift_output_stage_create(governance);
    rift_generate_output(output_stage, coordinated_ast);
    
    printf("\n[PIPELINE] Complete RIFT execution successful\n");
    printf("[PIPELINE] All stages executed with SP alignment\n");
    
    // Cleanup
    rift_output_stage_destroy(output_stage);
    rift_ast_coordinator_destroy(coordinator);
    rift_parser_destroy(parser);
    ast_node_destroy(ast);
    token_stream_destroy(tokens);
    rift_tokenizer_destroy(tokenizer);
    rift_governance_destroy(governance);
    
    return 0;
}