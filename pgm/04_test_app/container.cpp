#include <iostream>
#include <stdio.h>

using namespace std;

struct Data_t {
   char a;
   int b;
   char c;
   int d;
};   

Data_t data = {'a',0xb,'c',0xd};

Data_t* getContainer(char *ptr)
{
   Data_t temp;

   long offset = (char *)&temp.c - (char *)&temp;

   cout << "size of Data_t: " << sizeof(Data_t) << "offset: " << offset <<"\n";

   return (Data_t*)((char*)ptr - (char*)offset);
}

/** 
 * calculating offset by using zeroed reference
 */
Data_t* getContainer_a(char *ptr)
{
   long offset =   (long)(&((Data_t *)0)->c);

   return (Data_t*)((char*)ptr - (char*)offset);
}

/* Making it more generic, for any data member of any type
 */
#define container_of(ptr, type, member) ((type*) ((char *)ptr - (char *)(&((type \
                                *)0)->member)))

int main(int argc, char *argv[])
{
   //Data_t *temp = getContainer(&data.c);
   //Data_t *temp = getContainer_a(&data.c);
   Data_t *temp = container_of(&data.c, Data_t, c);

   cout << "address of data: " << &data << "address of member c: " << &data.c
           <<"\n";

   printf("Add of data: %p\n", &data);
   printf("Add of data.member C: %p\n", &data.c);
   printf("temp %p\n", temp);

   cout << temp->a << " " << temp->b << " " << temp->c << " " << temp->d <<"\n";
}
