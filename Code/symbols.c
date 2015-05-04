#include "symbols.h" 

//用于取名的变量和定义
#define name_head "__compiler_exp"
#define name_tail "__"
static int name_count=0;

//符号表主表。本实验只有一张符号表。
struct
{
	type_d* types;
	func_d* funcs;
	value_stack* values;
}symbols;

void itoa(unsigned long val, char* buf,unsigned radix)
{
	char *p;					/* pointer to traverse string */
	char *firstdig;				/* pointer to first digit */
	char temp;					/* temp char */
	unsigned digval;			/* value of digit */
	
	p = buf;
	firstdig = p;				/* save pointer to first digit */
	
	do {
		digval = (unsigned)(val % radix);
		val /= radix;			/* get next digit */
		
		/* convert to ascii and store */
		if (digval > 9)
			*p++ = (char)(digval - 10 + 'a');	/* a letter */
		else
			*p++ = (char)(digval + '0'); 		/* a digit */
	} while (val > 0);
	
	/* We now have the digit of the number in the buffer, but in reverse 
	 * order. Thus we reverse them now. */
	
	*p-- = '\0';				/* terminate string; p points to last digit */
	
	do {
		temp = *p;
		*p = *firstdig;
		*firstdig = temp;		/* swap *p and *firstdig */
		--p;
		++firstdig;				/* advance to next two digits */
	} while (firstdig < p);		/* repeat until halfway */
}
void get_a_name(char* name)
{
	strcpy(name,name_head);
	char temp[20];
	itoa(name_count,temp,10);
	name_count++;
	strcat(name,temp);
	strcat(name,name_tail);
}
void init_symbol_table()
{
	symbols.types=NULL;
	symbols.funcs=NULL;
	symbols.values=NULL;
	value_stack_push();
}
void destroy_symbol_table()
{
	while(symbols.values!=NULL)
		value_stack_pop();
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
type_d* find_type(const char* name)
{
	type_d* p=symbols.types;
	while(p!=NULL)
	{
		if(p->kind==STRUCT && strcmp(name,p->name)==0)
			return p;
		p=p->next;
	}
	return NULL;
}
func_d* find_function(const char* name)
{
	func_d* p=symbols.funcs;
	while(p!=NULL)
	{
		if(strcmp(name,p->name)==0)
			return p;
		p=p->next;
	}
	return NULL;
}
val_d* find_value(const char* name)
{
	value_stack* q=symbols.values;
	while(q!=NULL)
	{
		val_d* p=q->values;
		while(p!=NULL)
		{
			if(strcmp(name,p->name)==0)
				return p;
			p=p->next;
		}
		q=q->next;
	}
	return NULL;
}
type_d* add_type_declaration(type_d* r)
{
	insert_head(symbols.types,r);
	return r;
}
func_d* add_function_declaration(func_d* r)
{
	insert_head(symbols.funcs,r);
	return r;
}
val_d* add_value_declaration(val_d* r)
{
	insert_head(symbols.values->values,r);
	return r;
}
type_d* new_type(const char* name)
{
	type_d* p=(type_d*)malloc(sizeof(type_d));
	p->next=NULL;
	if(name!=NULL)
	{
		strcpy(p->name,name);
		p->kind=STRUCT;
		p->def.s=(struct_def_list*)malloc(sizeof(struct_def_list));
		p->def.s->define_count=0;
		p->def.s->def_list=NULL;
	}
	else
	{
		p->name[0]='\0';
		p->kind=ARRAY;
		p->def.a=NULL;
	}
	return p;
}
func_d* new_function(const char* name)
{
	func_d* p=(func_d*)malloc(sizeof(func_d));
	p->next=NULL;
	strcpy(p->name,name);
	p->parameter_count=0;
	p->parameters=NULL;
	p->return_type=NULL;
	return p;
}
val_d* new_value(const char* name)
{
	val_d* p=(val_d*)malloc(sizeof(val_d));
	strcpy(p->name,name);
	p->next=NULL;
	p->is_true_value=true;
	p->kind=INT;
	p->val_type=NULL;
	return p;
}
void array_generate_basic_dimension(type_d* t,int number,val_kind kind,type_d* val_type)
{
	t->def.a=(array_def_list*)malloc(sizeof(array_def_list));
	t->def.a->dimension=0;
	t->def.a->number=number;
	t->def.a->size_of_each=4;
	t->def.a->kind=kind;
	t->def.a->val_type=val_type;
	t->def.a->next=NULL;
}
void array_expand_dimension(type_d* t,int number)
{
	array_def_list* p=(array_def_list*)malloc(sizeof(array_def_list));
	p->dimension=t->def.a->dimension+1;
	p->number=number;
	p->size_of_each=(t->def.a->number)*(t->def.a->size_of_each);
	p->kind=t->def.a->kind;
	p->val_type=t->def.a->val_type;
	p->next=NULL;
	insert_head(t->def.a,p);
}
bool type_equal(type_d* p,type_d* q)
{
	if(p==q)
		return true;
	if(p->kind!=q->kind)
		return false;
	if(p->kind==STRUCT)
		return false;
	if(p->def.a->dimension==q->def.a->dimension && p->def.a->kind==q->def.a->kind && p->def.a->val_type==q->def.a->val_type)
		return true;
	return false;
}
void value_stack_push()
{
	value_stack* p=(value_stack*)malloc(sizeof(value_stack));
	p->values=NULL;
	p->next=NULL;
	insert_head(symbols.values,p);
}
void value_stack_pop()
{
	val_d* p;
	while(symbols.values->values!=NULL)
	{
		p=symbols.values->values;
		symbols.values->values=symbols.values->values->next;
		free(p);
	}
	value_stack* q=symbols.values;
	symbols.values=symbols.values->next;
	free(q);
}
bool value_stack_check(const char* name)
{
	val_d* p=symbols.values->values;
	while(p!=NULL)
	{
		if(strcmp(p->name,name)==0)
			return false;
		p=p->next;
	}
	return true;
}