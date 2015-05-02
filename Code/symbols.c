#include "symbols.h" 
struct type_d;
struct val_d;
//变量的类型：int，float，用户自定义类型（数组，结构体）
typedef enum val_kind
{
	INT,FLOAT,USER_DEFINED
}val_kind;
//用户自定义类型：结构体，数组
typedef enum type_kind
{
	STRUCT,ARRAY
}type_kind;
//数组定义的单元，数组定义用链表表示。该层有几个元素，每个元素多大，基本元素是什么（及定义指针，如果需要），降一维度指针
typedef struct array_def_list
{
	int number;
	int size_of_each;
	val_kind kind;	
	struct type_d* val_type;
	struct array_def_list* next;
}array_def_list;
//结构体定义单元。该结构体有几个域，每个域的定义
typedef struct struct_def_list
{
	int define_count;
	struct val_d** def_list;
}struct_def_list;
//变量定义单元。函数参数，用户变量，结构体内变量。名称，类型，类型定义指针（如果需要），下一个单元地址
typedef struct val_d
{
	char name[MAX_LEN_OF_NAME];
	val_kind kind;
	struct type_d* val_type;
	struct val_d* next;
}val_d;
//类型定义单元。名称，类型，类型具体定义，下一个单元地址
typedef struct type_d
{
	char name[MAX_LEN_OF_NAME];
	type_kind kind;
	union
	{
		array_def_list* a;
		struct_def_list* s;
	}def;
	struct type_d* next;
}type_d;
//函数定义单元。名称，参数个数，参数定义列表，返回值类型定义，下一个单元地址
typedef struct func_d
{
	char name[MAX_LEN_OF_NAME];
	int parameter_count;
	val_d** parameters;
	type_d* return_type;
	struct func_d* next;
}func_d;
//符号表主表。本实验只有一张符号表。
struct
{
	type_d* types;
	func_d* funcs;
	val_d* values;
}symbols;
void init_symbol_table()
{
	symbols.types=NULL;
	symbols.funcs=NULL;
	symbols.values=NULL;
}
void destroy_symbol_table()
{
	{
		val_d* p;
		while(symbols.values!=NULL)
		{
			p=symbols.values;
			symbols.values=symbols.values->next;
			free(p);
		}
	}
	{
		func_d* p;
		while(symbols.funcs!=NULL)
		{
			p=symbols.funcs;
			symbols.funcs=symbols.funcs->next;
			free(p->parameters);
			free(p);
		}
	}
	{
		type_d* p;
		while(symbols.types!=NULL)
		{
			p=symbols.types;
			symbols.types=symbols.types->next;
			if(p->kind==STRUCT)
			{
				free(p->def.s->def_list);
				free(p->def.s);
			}
			else
			{
				array_def_list* q;
				while(p->def.a!=NULL)
				{
					q=p->def.a;
					p->def.a=p->def.a->next;
					free(q);
				}
			}
			free(p);
		}
	}
}
int add_type_declaration(Node* r);
int add_function_declaration(Node* r);
int add_value_declaration(Node* r);