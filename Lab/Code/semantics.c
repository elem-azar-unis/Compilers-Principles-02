#include "semantics.h"
#include "symbols.h"
val_kind kind=USER_DEFINED;			//当前处理类型：int，float，用户定义类型
struct type_d* val_type=NULL;		//当前处理的用户定义类型的定义结构体指针（如果需要）
int para_count=0;					//参数数目。
val_d* paras[100];					//各参数定义。不想些链表了。一个结构体，一个函数的变量、参数不超过100个。
int need_count=0;					//是否需要记录定义的变量。
int do_not_push=0;					//提醒在函数刚建立的CompSt不需要push符号表。
typedef struct stack
{
	val_kind kind;						//当前处理类型：int，float，用户定义类型
	struct type_d* val_type;			//当前处理的用户定义类型的定义结构体指针（如果需要）
	int para_count;						//参数数目。
	val_d* paras[100];					//各参数定义。不想些链表了。一个结构体，一个函数的变量、参数不超过100个。
	int need_count;						//是否需要记录定义的变量。
	struct stack* next;
}stack;
stack* st_head=NULL;
void push()
{
	stack* p=(stack*)malloc(sizeof(stack));
	p->kind=kind;
	p->val_type=val_type;
	p->para_count=para_count;
	p->need_count=need_count;
	for(int i=0;i<para_count;i++)
		p->paras[i]=paras[i];
	para_count=0;
	insert_head(st_head,p);
}
void pop()
{
	stack* p=st_head;
	st_head=st_head->next;
	kind=p->kind;
	val_type=p->val_type;
	para_count=p->para_count;
	for(int i=0;i<p->para_count;i++)
		paras[i]=p->paras[i];
	need_count=p->need_count;
	free(p);
}
void semantic_analysis(Node* h)
{
	if(h==NULL)return;
	switch(h->type)
	{
		case Specifier:
		{
			//在这里获得当前处理的类型，更改全局变量
			if(h->child[0]->type==_TYPE)
			{
				if(strcmp(h->child[0]->name,"int")==0)kind=_int;
				else kind=_float;
			}
			else
			{
				kind=USER_DEFINED;
				semantic_analysis(h->child[0]);
			}
			break;
		}
		case StructSpecifier:
		{
			if(h->child_count==2)
			{
				//这里是使用已经定义的结构体
				val_type=find_type(h->child[1]->child[0]->name);
				if(val_type==NULL)
					printf("Error type 17 at Line %d: undefined struct %s.\n",h->line,h->child[1]->child[0]->name);
			}
			else
			{
				//定义结构体，可能会直接使用于定义变量，可以使匿名结构体定义。
				char name[33];
				if(h->child[1]->child[0]->type==None)
					get_a_name(name);
				else
					strcpy(name,h->child[1]->child[0]->name);
				val_type=find_type(name);
				if(val_type!=NULL || find_value(name)!=NULL)
				{
					printf("Error type 16 at Line %d: name %s is used.\n",h->line,name);
					val_type=NULL;
				}
				else
				{
					val_type=new_type(name);
					add_type_declaration(val_type);
					push();
					need_count=1;
					para_count=0;
					semantic_analysis(h->child[3]);
					st_head->val_type->def.s->define_count=para_count;
					st_head->val_type->def.s->def_list=(val_d**)malloc(sizeof(val_d*)*para_count);
					for(int i=0;i<para_count;i++)
						st_head->val_type->def.s->def_list[i]=paras[i];
					need_count=0;
					pop();
				}
			}
			break;
		}
		case ExtDef:
		{
			/* 此处是定义，依据第二个子结点是ExtDecList,SEMI,FunDec,
			 * 分别确定是定义全局变量、函数的定义，分别作不同处理
			 */
			if(h->child[1]->type==FunDec)
			{
				func_d* temp=find_function(h->child[1]->child[0]->name);
				if(temp!=NULL)
					printf("Error type 4 at Line %d: function %s is defined.\n",h->line,h->child[1]->child[0]->name);
				else
				{
					temp=new_function(h->child[1]->child[0]->name);
					temp->return_type=val_type;
					temp->return_kind=kind;
					value_stack_push();
					do_not_push=1;
					need_count=1;
					para_count=0;
					semantic_analysis(h->child[1]->child[2]);
					temp->parameter_count=para_count;
					temp->parameters=(val_d**)malloc(sizeof(val_d*)*para_count);
					for(int i=0;i<para_count;i++)
						temp->parameters[i]=paras[i];
					need_count=0;
					semantic_analysis(h->child[2]);
					do_not_push=0;
				}
			}
			else
				for(int i=0;i<h->child_count;i++)
					semantic_analysis(h->child[i]);
			break;
		}
		case VarDec:
		{
			Node* temp=h;
			while(temp->child_count!=1)
				temp=temp->child[0];
			temp=temp->child[0];
			if(!value_stack_check(temp->name))
			{
				val_d* che=find_value(temp->name);
				int i=-1;
				if(do_not_push==0 && need_count==1)
					for(i=0;i<para_count;i++)
						if(paras[i]==che)
							break;
				if(i==para_count||i==-1)
					printf("Error type 3 at Line %d: variable %s is defined.\n",h->line,temp->name);
				else
					printf("Error type 15 at Line %d: Redefined field %s.",h->line,temp->name);
			}
			else if(!(kind==USER_DEFINED && val_type==NULL))
			{
				val_d* v=new_value(temp->name);
				v->is_true_value=(do_not_push==1 || need_count==0);
				if(h->child_count==1)
				{
					v->kind=kind;
					v->val_type=val_type;
				}
				else
				{
					v->kind=USER_DEFINED;
					v->val_type=new_type(NULL);
					add_type_declaration(v->val_type);
					temp=h->child[0];
					array_generate_basic_dimension(v->val_type,h->child[2]->value_i,kind,val_type);
					while(temp->child_count!=1)
					{
						array_expand_dimension(v->val_type,temp->child[2]->value_i);
						temp=temp->child[0];
					}
				}
				add_value_declaration(v);
				if(need_count)
				{
					paras[para_count]=v;
					para_count++;
				}
			}
			break;
		}
		default:
		{
			for(int i=0;i<h->child_count;i++)
				semantic_analysis(h->child[i]);
			break;
		}
	}
}