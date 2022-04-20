
#include "types.h"

const int ElemSize = sizeof(Elem);

void setElem(Elem * e,const char * name,const char * value)
{
	strcpy(e -> name,name);
	strcpy(e -> value,value);
}

void delElem(Elem * e)
{
	if(e != NULL)
		free(e);
}

void catElem(Elem e)
{
	printf("name = %s, value = %s\n",e.name,e.value);
}

void catElems(Elem e[],int length)
{
	for(int i = 0; i < length;i++)
		printf("name = %s, value = %s\n",e[i].name,e[i].value);
}
