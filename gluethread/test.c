#include "glthread.h"
#include <stdio.h>
#include <memory.h>
#include <stdio.h>

typedef struct _person{
    int age;
    int weight;
    glthread_t glthread;
}person_t;

int
senior_citizen(person_t* p1, person_t *p2){
    if(p1->age==p2->age)
        return 0;
    
    if(p1->age<p2->age)
        return 1;
    
    return -1;
}

#define offset(struct_name, fld_name)\
    (unsigned int)&(( (struct_name*)0 )->fld_name)

GLTHREAD_TO_STRUCT(thread_to_person, person_t,glthread);

int main(int argc, char**argv){
    person_t person[5];
    memset(person,0,sizeof(person_t)*5);
    person[0].age = 12 ;
    person[0].weight =22 ;
    person[1].age = 33;
    person[1].weight =14 ;
    person[2].age = 55;
    person[2].weight = 32;
    person[3].age = 11;
    person[3].weight = 12;
    person[4].age = 81;
    person[4].weight = 90;

    glthread_t base_glthread;
    init_glthread(&base_glthread);
    glthread_priority_insert(&base_glthread, &person[4].glthread,senior_citizen,offset(person_t,glthread));
    glthread_priority_insert(&base_glthread, &person[3].glthread,senior_citizen,offset(person_t,glthread));
    glthread_priority_insert(&base_glthread, &person[2].glthread,senior_citizen,offset(person_t,glthread));
    glthread_priority_insert(&base_glthread, &person[1].glthread,senior_citizen,offset(person_t,glthread));
    glthread_priority_insert(&base_glthread, &person[0].glthread,senior_citizen,offset(person_t,glthread));

    glthread_t *cur = NULL;
    ITERATE_GLTHREAD_BEGIN(&base_glthread,cur){
        person_t *p = thread_to_person(cur);
        printf("Age = %d\n", p->age);
    }ITERATE_GLTHREAD_END(&base_glthread,cur);

    return 0;
}