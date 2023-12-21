#include "smm_common.h"
#include "smm_object.h"
#include <string.h>

#define MAX_NODETYPE    7
#define MAX_GRADE       9

static char smmNodeName[SMMNODE_TYPE_MAX][MAX_CHARNAME] = {
       "강의 ",
       "식당",
       "실험실", 
       "집", 
       "실험실로이동",
       "음식찬스",
       "축제시간"
};

char* smmObj_getTypeName(int type)
{
    return (char*)smmNodeName[type];
}

//1. 구조체 형식 정의 
typedef struct smmObject {
    char name[MAX_CHARNAME];
    smmObjType_e objType;
    int type;
    int credit;
    int energy;
    smmObjGrade_e grade;
} smmObject_t;

typedef struct food_card {
    smmObjType_e objType;
    char name[MAX_CHARNAME];
    int energy;
} FOOD;

typedef struct festival_card {
    smmObjType_e objType;
    char content[MAX_CHARNAME];
} FESTIVAL;

static smmObject_t smm_node[MAX_NODE];
static int smmObj_noNode = 0;

static FOOD foods[MAX_NODE];
static int food_num = 0;

static FESTIVAL festivals[MAX_NODE];
static int fest_num = 0;

//3. 관련 함수 변경 
//object generation
void* smmObj_genObject(char* name, smmObjType_e objType, int type, int credit, int energy, smmObjGrade_e grade)
{
    smmObject_t* ptr;
    
    ptr = (smmObject_t*)malloc(sizeof(smmObject_t));
    
    strcpy(ptr->name, name);
    ptr->objType = objType;
    ptr->type = type;
    ptr->credit = credit;
    ptr->energy = energy;
    ptr->grade = grade;
    
    smm_node[smmObj_noNode++] = *ptr;
    
    return ptr;
}

char* smmObj_getNodeName(int node_nr)
{
    return smm_node[node_nr].name;
}

int smmObj_getNodeType(int node_nr)
{
    return smm_node[node_nr].type;
}

int smmObj_getNodeCredit(int node_nr)
{
    return smm_node[node_nr].credit;
}

int smmObj_getNodeEnergy(int node_nr)
{
    return smm_node[node_nr].energy;
}

smmObjGrade_e smmObj_getNodeGrade(int node_nr) {
    return smm_node[node_nr].grade;
}

// food
void * setFoodCard(smmObjType_e objType, char *name, int energy) {
    FOOD *new_card = (FOOD *)malloc(sizeof(FOOD));
    new_card->objType = objType;
    strcpy(new_card->name, name);
    new_card->energy = energy;
    
    foods[food_num++] = *new_card;
    
    return new_card;
}

char * getFoodName(int card_nr) {
    return foods[card_nr].name;
}

int getFoodEnergy(int card_nr) {
    return foods[card_nr].energy;
}

// festival
void * setFestivalCard(smmObjType_e objType, char *content) {
    FESTIVAL *new_card = (FESTIVAL *) malloc(sizeof(FESTIVAL));
    new_card->objType = objType;
    strcpy(new_card->content, content);
    
    festivals[fest_num++] = *new_card;
    
    return new_card;
}

char * getFestivalContent(int card_nr) {
    return festivals[card_nr].content;
}
