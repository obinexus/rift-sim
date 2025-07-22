// ================================
// OBINexus Framework - Fault-Tolerant Escape Sequence Processor
// Handles multi-layer character encoding transitions with explicit control
// ================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// ================================
// Escape Sequence Mapping Types
// ================================

typedef struct {
    char* source_sequence;      // Input pattern (e.g., "\\\\w")
    char* target_sequence;      // Output pattern (e.g., "\\w")
    char* fault_token;          // Fallback if transformation fails
    char* encoding_context;     // Which layer this applies to
    int priority;               // Processing order
} escape_mapping_t;

typedef struct {
    escape_mapping_t* mappings;
    size_t count;
    size_t capacity;
    char* default_fault_token;  // Universal fallback
} escape_processor_t;

typedef struct {
    char* processed_pattern;
    bool transformation_successful;
    char* fault_tokens_used;    // Record any fallbacks applied
    int transformations_applied;
} pattern_result_t;

// ================================
// Escape Processor Implementation
// ================================

static escape_processor_t* escape_processor_create(void) {
    escape_processor_t* processor = malloc(sizeof(escape_processor_t));
    if (!processor) return NULL;
    
    processor->capacity = 20;  // Room for many escape mappings
    processor->count = 0;
    processor->mappings = malloc(sizeof(escape_mapping_t) * processor->capacity);
    processor->default_fault_token = strdup("[UNKNOWN_ESCAPE]");
    
    if (!processor->mappings || !processor->default_fault_token) {
        free(processor->mappings);
        free(processor->default_fault_token);
        free(processor);
        return NULL;
    }
    
    return processor;
}

static void escape_processor_destroy(escape_processor_t* processor) {
    if (!processor) return;
    
    for (size_t i = 0; i < processor->count; i++) {
        free(processor->mappings[i].source_sequence);
        free(processor->mappings[i].target_sequence);
        free(processor->mappings[i].fault_token);
        free(processor->mappings[i].encoding_context);
    }
    
    free(processor->mappings);
    free(processor->default_fault_token);
    free(processor);
}

static bool escape_processor_add_mapping(escape_processor_t* processor,
                                        const char* source,
                                        const char* target, 
                                        const char* fault_token,
                                        const char* context,
                                        int priority) {
    if (!processor || !source || !target) return false;
    
    // Resize if needed
    if (processor->count >= processor->capacity) {
        processor->capacity *= 2;
        escape_mapping_t* new_mappings = realloc(processor->mappings,
                                                sizeof(escape_mapping_t) * processor->capacity);
        if (!new_mappings) return false;
        processor->mappings = new_mappings;
    }
    
    escape_mapping_t* mapping = &processor->mappings[processor->count];
    mapping->source_sequence = strdup(source);
    mapping->target_sequence = strdup(target);
    mapping->fault_token = fault_token ? strdup(fault_token) : strdup(processor->default_fault_token);
    mapping->encoding_context = context ? strdup(context) : strdup("general");
    mapping->priority = priority;
    
    processor->count++;
    return true;
}

// ================================
// Pattern Transformation with Fault Tolerance
// ================================

static char* safe_string_replace(const char* source, const char* find, const char* replace) {
    if (!source || !find || !replace) return NULL;
    
    size_t find_len = strlen(find);
    size_t replace_len = strlen(replace);
    size_t source_len = strlen(source);
    
    // Count occurrences to calculate result size
    size_t count = 0;
    const char* temp = source;
    while ((temp = strstr(temp, find)) != NULL) {
        count++;
        temp += find_len;
    }
    
    // Calculate new string size
    size_t new_size = source_len + count * (replace_len - find_len) + 1;
    char* result = malloc(new_size);
    if (!result) return NULL;
    
    // Perform replacement
    char* dest = result;
    const char* src = source;
    while (*src) {
        if (strncmp(src, find, find_len) == 0) {
            strcpy(dest, replace);
            dest += replace_len;
            src += find_len;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
    
    return result;
}

static pattern_result_t* process_escape_pattern(escape_processor_t* processor, 
                                              const char* input_pattern,
                                              const char* encoding_context) {
    if (!processor || !input_pattern) return NULL;
    
    pattern_result_t* result = malloc(sizeof(pattern_result_t));
    if (!result) return NULL;
    
    result->processed_pattern = strdup(input_pattern);  // Start with original
    result->transformation_successful = true;
    result->fault_tokens_used = malloc(256);  // Buffer for fault token tracking
    result->fault_tokens_used[0] = '\0';
    result->transformations_applied = 0;
    
    printf("  → Processing escape pattern: \"%s\" in context: %s\n", 
           input_pattern, encoding_context ? encoding_context : "general");
    
    // Apply transformations in priority order
    for (size_t i = 0; i < processor->count; i++) {
        escape_mapping_t* mapping = &processor->mappings[i];
        
        // Check if this mapping applies to the current context
        if (encoding_context && strcmp(mapping->encoding_context, encoding_context) != 0 &&
            strcmp(mapping->encoding_context, "general") != 0) {
            continue;
        }
        
        // Check if pattern contains the source sequence
        if (strstr(result->processed_pattern, mapping->source_sequence)) {
            char* new_pattern = safe_string_replace(result->processed_pattern, 
                                                   mapping->source_sequence,
                                                   mapping->target_sequence);
            
            if (new_pattern) {
                printf("    ↳ Applied mapping: \"%s\" → \"%s\"\n", 
                       mapping->source_sequence, mapping->target_sequence);
                
                free(result->processed_pattern);
                result->processed_pattern = new_pattern;
                result->transformations_applied++;
            } else {
                // Transformation failed, apply fault token
                char* fault_pattern = safe_string_replace(result->processed_pattern,
                                                         mapping->source_sequence,
                                                         mapping->fault_token);
                if (fault_pattern) {
                    printf("    ↳ Transformation failed, applied fault token: \"%s\"\n", 
                           mapping->fault_token);
                    
                    strcat(result->fault_tokens_used, mapping->fault_token);
                    strcat(result->fault_tokens_used, " ");
                    
                    free(result->processed_pattern);
                    result->processed_pattern = fault_pattern;
                    result->transformation_successful = false;
                }
            }
        }
    }
    
    printf("    ↳ Final pattern: \"%s\" (transformations: %d)\n", 
           result->processed_pattern, result->transformations_applied);
    
    return result;
}

static void pattern_result_destroy(pattern_result_t* result) {
    if (!result) return;
    
    free(result->processed_pattern);
    free(result->fault_tokens_used);
    free(result);
}

// ================================
// OBINexus Integration Functions
// ================================

static escape_processor_t* create_obinexus_escape_processor(void) {
    escape_processor_t* processor = escape_processor_create();
    if (!processor) return NULL;
    
    printf("[ESCAPE_PROCESSOR] Initializing OBINexus fault-tolerant mappings\n");
    
    // Double-escaped sequences for configuration layer
    escape_processor_add_mapping(processor, "\\\\\\\\w", "\\w", "[WORD_CLASS]", "config_to_c", 100);
    escape_processor_add_mapping(processor, "\\\\\\\\d", "\\d", "[DIGIT_CLASS]", "config_to_c", 100);
    escape_processor_add_mapping(processor, "\\\\\\\\s", "\\s", "[SPACE_CLASS]", "config_to_c", 100);
    escape_processor_add_mapping(processor, "\\\\\\\\.", "\\.", "[DOT_LITERAL]", "config_to_c", 100);
    
    // Single-escaped sequences for C string layer  
    escape_processor_add_mapping(processor, "\\\\w", "[a-zA-Z0-9_]", "[WORD_FALLBACK]", "c_to_regex", 90);
    escape_processor_add_mapping(processor, "\\\\d", "[0-9]", "[DIGIT_FALLBACK]", "c_to_regex", 90);
    escape_processor_add_mapping(processor, "\\\\s", "[ \\t\\n\\r]", "[SPACE_FALLBACK]", "c_to_regex", 90);
    escape_processor_add_mapping(processor, "\\\\.", "\\.", "[DOT_FALLBACK]", "c_to_regex", 90);
    
    // Raw string processing for R'' contexts
    escape_processor_add_mapping(processor, "\\\\\\\\", "\\", "[BACKSLASH]", "raw_string", 80);
    
    printf("  → Loaded %zu escape sequence mappings\n", processor->count);
    printf("  → Fault tolerance enabled with systematic fallbacks\n");
    
    return processor;
}

// ================================
// Demonstration and Testing
// ================================

static void demonstrate_escape_processing(void) {
    printf("\nOBINexus Escape Sequence Processing Demonstration\n");
    printf("================================================\n");
    
    escape_processor_t* processor = create_obinexus_escape_processor();
    if (!processor) {
        printf("Failed to create escape processor\n");
        return;
    }
    
    // Test the double-escaped patterns from your .riftrc.0 configuration
    const char* test_patterns[] = {
        "^[a-zA-Z_]\\\\\\\\w*$",      // Double-escaped identifier pattern
        "^\\\\\\\\d+(\\\\\\\\.\\\\\\\\d+)?$",  // Double-escaped number pattern  
        "^[+\\\\\\\\-*/]$",           // Double-escaped operator pattern
        "^\\\\\\\\s+$"                // Double-escaped whitespace pattern
    };
    
    const char* pattern_names[] = {
        "IDENTIFIER_PATTERN",
        "NUMBER_PATTERN", 
        "OPERATOR_PATTERN",
        "WHITESPACE_PATTERN"
    };
    
    size_t test_count = sizeof(test_patterns) / sizeof(test_patterns[0]);
    
    for (size_t i = 0; i < test_count; i++) {
        printf("\n[PATTERN_TEST] Processing %s\n", pattern_names[i]);
        
        // First transformation: config_to_c context
        pattern_result_t* config_result = process_escape_pattern(processor, 
                                                               test_patterns[i], 
                                                               "config_to_c");
        
        if (config_result) {
            // Second transformation: c_to_regex context  
            pattern_result_t* regex_result = process_escape_pattern(processor,
                                                                  config_result->processed_pattern,
                                                                  "c_to_regex");
            
            if (regex_result) {
                printf("  → Final regex-ready pattern: \"%s\"\n", regex_result->processed_pattern);
                printf("  → Transformation successful: %s\n", 
                       regex_result->transformation_successful ? "YES" : "NO");
                
                if (strlen(regex_result->fault_tokens_used) > 0) {
                    printf("  → Fault tokens applied: %s\n", regex_result->fault_tokens_used);
                }
                
                pattern_result_destroy(regex_result);
            }
            
            pattern_result_destroy(config_result);
        }
    }
    
    escape_processor_destroy(processor);
}

// ================================
// Main Function for Testing
// ================================

int main(void) {
    printf("OBINexus Framework - Fault-Tolerant Escape Sequence Processing\n");
    printf("=============================================================\n");
    printf("Handling multi-layer character encoding transitions\n");
    printf("Supporting R'' and R'' mapping with systematic fault tolerance\n");
    
    demonstrate_escape_processing();
    
    printf("\n[SYSTEM] Escape sequence processing validation complete\n");
    printf("[SYSTEM] Ready for integration with RIFT governance architecture\n");
    
    return 0;
}