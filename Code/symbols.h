#include <stdlib.h>
#include "tree.h"
void init_symbol_table();
void destroy_symbol_table();
int add_type_declaration(Node* r);
int add_function_declaration(Node* r);
int add_value_declaration(Node* r);