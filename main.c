
#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARD_FILE_PATH  "marbleBoardConfig.txt"
#define FOOD_FILE_PATH "marbleFoodConfig.txt"
#define FEST_FILE_PATH "marbleFestivalConfig.txt"

//board configuration parameters
static int board_nr;
static int food_nr;
static int festival_nr;
static int player_nr;

typedef struct player {
    int energy;
    int position;
    char name[MAX_CHARNAME];
    int accumCredit;
    int flag_graduate;
    int isExperiment;
    char taken[MAX_NODE][MAX_CHARNAME];
    int takenCount;
} player_t;

static player_t *cur_player;
static int lab;

#if 0
static int player_energy[MAX_PLAYER];
static int player_position[MAX_PLAYER];
static char player_name[MAX_PLAYER][MAX_CHARNAME];
#endif
//function prototypes
#if 0
int isGraduated(void); //check if any player is graduated
 //print grade history of the player
void goForward(int player, int step); //make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void); //print all player status at the beginning of each turn
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
void printGrades(int player); //print all the grade history of the player
#endif


void printGrades(int player)
{
     int i;
     void *gradePtr;
     for (i = 0;i < smmdb_len(LISTNO_OFFSET_GRADE + player); i++)
     {
         gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
         printf("%s : %d\n", smmObj_getNodeName(i), smmObj_getNodeGrade(i));
     }
}

void printPlayerStatus(void)
{
     int i;
     
     for (i = 0;i < player_nr; i++)
     {
         printf("%s : credit %i, energy %i, position %i\n",
                      cur_player[i].name,
                      cur_player[i].accumCredit,
                      cur_player[i].energy,
                      cur_player[i].position);
     }
}

void generatePlayers(int n, int initEnergy) //generate a new player
{
     int i;
     //n time loop
     for (i = 0; i < n; i++)
     {
         //input name
         printf("Input player %i's name:", i); //æ»≥ª πÆ±∏
         scanf("%s", cur_player[i].name);
         
         //set position
         //player_position[i] = 0;
         cur_player[i].position = 0;
         
         //set energy
         //player_energy[i] = initEnergy;
         cur_player[i].energy = initEnergy;
         cur_player[i].accumCredit = 0;
         cur_player[i].flag_graduate = 0;
         cur_player[i].isExperiment = 0;
         cur_player[i].takenCount = 0;
     }
}


int rolldie(int player)
{
    char c;
    fflush(stdin);
    printf("\n\nPress any key to roll a die (press g to see grade): ");
    c = getchar();
    fflush(stdin);

    if (c == 'g')
        printGrades(player);
    
    return (rand() % MAX_DIE + 1);
}

//action code when a player stays at a node
void actionNode(int player)
{
    void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
    //int type = smmObj_getNodeType( cur_player[player].position );
    int type = smmObj_getNodeType(cur_player[player].position);
    int credit = smmObj_getNodeCredit(cur_player[player].position);
    char *name = smmObj_getNodeName(cur_player[player].position);
    int energy = smmObj_getNodeEnergy(cur_player[player].position);
    void *gradePtr;
    int num;
    
    switch(type)
    {
        //case lecture:
        case SMMNODE_TYPE_LECTURE:
            if (cur_player[player].energy >= energy) {
                printf("Press \'t\' or \'d\' (t: take, d: drop):");
                char choice = getchar();
                fflush(stdin);
                
                if (choice == 't') {
                    int position = cur_player[player].position;
                    cur_player[player].accumCredit += credit;
                    cur_player[player].energy -= energy;
                    int flag = 0;
                    int i =0;
                    for ( i = 0; i < cur_player[player].takenCount; i++) {
                        if (strcmp(cur_player[player].taken[i], name) == 0) {
                            flag = 1;
                        }
                    }
                    if (!flag) {
                        strcpy(cur_player[player].taken[cur_player[player].takenCount], name);
                        cur_player[player].takenCount++;
                    }
//                    smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
                } else {
                    break;
                }
            }
            else {
                printf("Can't take %s.\n", name);
            }
        
            break;
        case SMMNODE_TYPE_RESTAURANT:
            cur_player[player].energy += energy;
            break;
        case SMMNODE_TYPE_LABORATORY:
            if (cur_player[player].isExperiment == 1) { // 실험 중
                cur_player[player].energy -= energy;
                int dice = rolldie(player);
                if (dice >= SUCCESS_EXPERIMENT) {
                    cur_player[player].isExperiment = 0;    // 실험 끝
                    cur_player[player].position += dice;
                }
            }
            break;
        case SMMNODE_TYPE_HOME:
            cur_player[player].energy += smmObj_getNodeEnergy(player);
            break;
        case SMMNODE_TYPE_GOTOLAB:
            cur_player[player].isExperiment = 1;
            cur_player[player].position = lab;
            break;
        case SMMNODE_TYPE_FOODCHANCE:
            num = rand() % food_nr;
            char * food = getFoodName(num);
            int energy = getFoodEnergy(num);
            cur_player[player].energy += energy;
            printf("%s eat %s. He gains %d energy! Total energy is %d\n", cur_player[player].name, food, energy, cur_player[player].energy);
            break;
        case SMMNODE_TYPE_FESTIVAL:
            num = rand() % festival_nr;
            char * mission = getFestivalContent(num);
            printf("Mission: %s\n", mission);
            break;
        default:
            break;
    }
}

void goForward(int player, int step)
{
    void *boardPtr;
    cur_player[player].position += step;
    if (cur_player[player].position >= board_nr) {
        cur_player[player].position %= board_nr;
    }
    
    boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
     
    printf("%s go to node %i (name: %s)\n",
                cur_player[player].name, cur_player[player].position,
                smmObj_getNodeName(cur_player[player].position));
}

int checkPlayer(int player) {
    if (cur_player[player].accumCredit >= GRADUATE_CREDIT && cur_player[player].position == 0) {
        return 1;
    }
    return 0;
}

int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char node_name[MAX_CHARNAME];
    char food_name[MAX_CHARNAME];
    char content[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int i;
    int initEnergy;
    int turn = 0;
    
    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;
    
    srand(time(NULL));
    
    
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig
    if ((fp = fopen(BOARD_FILE_PATH, "r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARD_FILE_PATH);
        getchar();
        return -1;
    }
    
    printf("Reading board component......\n");
    while ( fscanf(fp, "%s %i %i %i", node_name, &type, &credit, &energy) == 4 ) //read a node parameter set
    {
        //store the parameter set
        //(char* name, smmObjType_e objType, int type, int credit, int energy, smmObjGrade_e grade)
        void *boardObj = smmObj_genObject(node_name, smmObjType_board, type, credit, energy, 0);
        smmdb_addTail(LISTNO_NODE, boardObj);
        
        if (type == SMMNODE_TYPE_HOME)
            initEnergy = energy;
        else if (type == SMMNODE_TYPE_GOTOLAB)
            lab = board_nr;
        board_nr++;
    }
    fclose(fp);
    printf("Total number of board nodes : %i\n", board_nr);
    
    
    for (i = 0;i < board_nr; i++)
    {
        void *boardObj = smmdb_getData(LISTNO_NODE, i);
        
        printf("node %i : %s, %i(%s), credit %i, energy %i\n",
                     i, smmObj_getNodeName(i),
                     smmObj_getNodeType(i), smmObj_getTypeName(smmObj_getNodeType(i)),
                     smmObj_getNodeCredit(i), smmObj_getNodeEnergy(i));
    }
    //printf("(%s)", smmObj_getTypeName(SMMNODE_TYPE_LECTURE));
    
    //#if 0
    //2. food card config
    if ((fp = fopen(FOOD_FILE_PATH, "r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOOD_FILE_PATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while (fscanf(fp, "%s %i", food_name, &energy) == 2) //read a food parameter set
    {
        //store the parameter set
        setFoodCard(LISTNO_FOODCARD, food_name, energy);
        food_nr++;
    }
    fclose(fp);
    
    printf("Total number of food cards : %i\n", food_nr);
    for (i = 0;i < food_nr; i++) {
        printf("food %i : food %s, energy %d\n", i, getFoodName(i), getFoodEnergy(i));
    }
    
    //3. festival card config
    if ((fp = fopen(FEST_FILE_PATH, "r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FEST_FILE_PATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while (fscanf(fp, "%s", content) == 1) //read a festival card string
    {
        //store the parameter set
        setFestivalCard(LISTNO_FESTCARD, content);
        festival_nr++;
    }
    fclose(fp);
    
    printf("Total number of festival cards : %i\n", festival_nr);
   
    for (i = 0; i < festival_nr; i++) {
        printf("festival %i: content %s\n", i, getFestivalContent(i));
    }
    printf("\n");
    
    //2. Player configuration ---------------------------------------------------------------------------------
    
    do {
        //input player number to player_nr
        printf("input player no.:");
        scanf("%d", &player_nr);
        fflush(stdin);
    } while (player_nr < 0 || player_nr >  MAX_PLAYER);
    
    cur_player = (player_t *) malloc(player_nr * sizeof(player_t));
    generatePlayers(player_nr, initEnergy);
    
    
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (1) //is anybody graduated?
    {
        int die_result;
        
        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(turn, die_result);

        //4-4. take action at the destination node of the board
        actionNode(turn);
        
        int flag = checkPlayer(turn);
        if (flag == 1) {
            printf("Player %s graduate!\n", cur_player[turn].name);
            printf("Courses taken: ");
            int i = 0; 
            for (i = 0; i < cur_player[turn].takenCount; i++) {
                printf("%s ", cur_player[turn].taken[i]);
            }
            printf("\n");
            printf("Credit: %d, Grade: %.2f\n", 
                   cur_player[turn].accumCredit,
                   (double) cur_player[turn].accumCredit / cur_player[turn].takenCount);
            break;
        }
        
        //4-5. next turn
        turn = (turn + 1) % player_nr;
    }
    
    
    free(cur_player);
    system("PAUSE");
    return 0;
}
