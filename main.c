/*
 *
 * Program description: Conway's game of life coded in c.
						Can load/save from files, dynamic memory allocation, can edit boards
						Can use pre-defined boards or start from scratch
 *						Visualises evolution of game board by 1's and 0's being printed as per the board grid
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h> //for board delay purposes

#define N 2 //board_size array size controller; not needed but if more info needed to be stored here, easier to do this way
#define FMAX 1000 //file reader max

typedef unsigned int uint; //usual scanf = 0 will catch negative numbers

typedef struct {
    int status; //status to be printed
	int next_gen; //stored info for update
} board_cell;

/* Prototype functions */

/*		Main functions	*/
board_cell **GenerateBoard(uint board_size[N]); //BOARD GENERATING FUNCTION
void Ticker(float delay, board_cell **Board, uint board_size[N], int End_Generation, int grid);	//MAIN TICK MANAGEMENT FUNCTION
void TimeControl(float sec); //for delays to prevent board print spam
void FreeMemory(board_cell **Board, uint board_size[N]); //frees memory after usage

/*		Board updating functions		*/
void PrintBoard(board_cell **Board, uint board_size[N]); //print
void PrintBoardGrid(board_cell **Board, uint board_size[N]); //PrintBoard but with a grid along top and side
void BoardScan(board_cell **Board, uint board_size[N]); //neighbour check
void BoardUpdate(board_cell **Board, uint board_size[N]);
void ClearNextGen(board_cell **Board, uint board_size[N]); //wipes next_gen Board info

/*		Board manipulator functions		*/
int iadd(int i, int p, uint board_size[N]); // function that handles edges of board for i
int jadd(int j, int q, uint board_size[N]); // function that handles edges of board for j
int neighbour(int i, int j, board_cell **Board, uint board_size[N]); //func that returns number of neighbours
int FileCountCols();
int FileCountRows(int Cols);

/*		Menu functions		*/
void PlayMenu(int grid);
void CustomPlay(int grid);
void PreDefBoardMenu(int grid);
void CustomiseBoard(board_cell **Board, uint board_size[N], int grid);
void SaveMenu(board_cell **Board, uint board_size[N]);
void LoadMenu(int grid);

/*		File functions		*/
void FileLoad(board_cell **Board, uint board_size[N]);//format of files must be X[SPACE]X[SPACE]X[SPACE].. where X is 0 or 1. must be the right board size
void FileSave(board_cell **Board, uint board_size[N]);

/*		Functions for pre-defined boards		*/
void BoardOscillatorLoad(board_cell **Board);
void BoardSpaceshipLoad(board_cell **Board);



int main()
{

	int grid = 0; //master grid control variable
	int breaker = 1;
	int flag;
	int option_main;

	while(breaker == 1){
		printf("CONWAY'S GAME OF LIFE \nbut its in c\n\n\n");
		printf("MAIN MENU\n");
		printf("Option (0): Quit the game.\n");
		printf("Option (1): Play the game.\n");
		printf("Option (2): Toggle Grid Mode\n");
		printf("Please enter the number that corresponds to your selection: ");
		if((flag = scanf("%i", &option_main)) == 0){
			option_main = 5;
			printf("\n\n\nA fatal error has been encountered. The program will now exit\n\n");
			return -1;
		}
		printf("\n");

		switch(option_main){
			case 0:
				breaker = 0;
				break;
			case 1:
				PlayMenu(grid);
				continue;
			case 2:
				if(grid == 0){
                    grid = 1;
					printf("\nGrid Mode ON\n\n");
					continue;
				}
				else grid = 0;
				printf("\nGrid Mode OFF\n\n");
				continue;
			default:
				printf("\nUnknown selection. Please try again\n");
				continue;
		}
	}
	printf("Quitting...");
    return 0;
}

/*		FUNCTIONS		*/

//dynamically generates the game board with calloc
board_cell **GenerateBoard(uint board_size[N]){ //calloc is used as it sets array values to 0 automatically.
	board_cell **Board = (board_cell **)calloc(board_size[0], sizeof(board_cell *));
		for (int i = 0; i < board_size[0]; ++i)
			Board[i] = (board_cell *)calloc(board_size[1], sizeof(board_cell));
	if(Board == NULL){
		printf("\nERROR! MEMORY ALLOCATION FAILED!\n");
		return NULL;
	}
	return Board;
}

//Updates the board every generation
void Ticker(float delay, board_cell **Board, uint board_size[N], int End_Generation, int grid){
	int Gen = 1;
	while(Gen < End_Generation + 1){
		printf("\n Generation: [%i / %i]\n", Gen, End_Generation);
		if(grid == 0){
			PrintBoard(Board, board_size);
		}
		else{
			PrintBoardGrid(Board, board_size);
		};
		if(delay <= 0){
			delay = 0.01;
		}
		
		BoardScan(Board, board_size);
		BoardUpdate(Board, board_size);
		ClearNextGen(Board, board_size);
		TimeControl(delay);
		++Gen;
	}
}

//manages delay, using <time.h>
void TimeControl(float sec){
    // Converting seconds to milliseconds
    float msec = 1000 * sec;

    // Stroing start time
    clock_t start_time = clock();

    // looping till required time is not acheived
    while (clock() < start_time + msec);
}

//Frees a memory
void FreeMemory(board_cell **Board, uint board_size[N]){
	for(int F=0; F<board_size[0];F++){
		free(Board[F]);
    }
    free(Board);
    //printf("\n Memory is now free!\n");
	return;
}

//prints the array
void PrintBoard(board_cell **Board, uint board_size[N]){
	printf("\n");
	for(int i=0; i<board_size[0]; ++i){
		for(int j=0; j<board_size[1]; ++j){
			printf("%i", Board[i][j].status);
		}
		printf("\n");
	}
}

//modified PrintBoard that prints to a grid with numbers along top and bottom
void PrintBoardGrid(board_cell **Board, uint board_size[N]){ //supports board size of 999
	printf("[   ]");
	for(int k=0; k<board_size[1]; ++k){
	printf("[%3i]", k);
	}
	printf("\n");
	for(int i=0; i<board_size[0]; ++i){
		printf("[%3i]", i);
		for(int j=0; j<board_size[1]; ++j){
			printf("  %i  ", Board[i][j].status);
		}
		printf("\n");
	}
}

//handles rule set of Life from https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
void BoardScan(board_cell **Board, uint board_size[N]){
	for(int i=0; i<board_size[0]; ++i){
		for(int j=0; j<board_size[1]; ++j){
			int count = neighbour(i, j, Board, board_size);

			if(count == 2)Board[i][j].next_gen = Board[i][j].status; //stagnation, stasis
			if(count == 3)Board[i][j].next_gen = 1; //reproduction, life
			if(count > 3)Board[i][j].next_gen = 0; //overpopulation, death
			if(count < 2)Board[i][j].next_gen = 0; //underpopulation, death

			}//end of j loop
		}//end of i loop
	return;
}

//commits result of board scan to board
void BoardUpdate(board_cell **Board, uint board_size[N]){
	for(int i=0; i<board_size[0]; ++i){
		for(int j=0; j<board_size[1]; ++j){
			Board[i][j].status = Board[i][j].next_gen;
		}//end of j loop
	}//end of i loop
}

//clears the next_gen property of struct board_cell for each element
void ClearNextGen(board_cell **Board, uint board_size[N]){
	for(int i=0; i<board_size[0]; ++i){
		for(int j=0; j<board_size[1]; ++j){
				Board[i][j].next_gen = 0;
		}//end of j loop
	}//end of i loop
}

//function that handles the loop around in i direc
int iadd(int i, int p, uint board_size[N]){
	i = i+p;
	while(i < 0) i += board_size[0];
	while(i >= board_size[0]) i -= board_size[0];
	return i;
}

//function that handles the loop around in the j direc
int jadd(int j, int q, uint board_size[N]){
	j = j+q;
	while(j < 0) j += board_size[1];
	while(j >= board_size[1]) j -= board_size[1];
	return j;
}

//goes around a grid cell and counts number of 1s
int neighbour(int i, int j, board_cell **Board, uint board_size[N]){
	int sum=0;
	for(int p=-1; p<=1; ++p){
		for(int q=-1;q<=1;++q){

				if(p == 0 && q == 0);//if p=0 and q=0 then the cell is the one we're centered on, so don't count 
				else{
				if(Board[iadd(i, p, board_size)][jadd(j, q, board_size)].status == 1) sum += 1;
				};

		}//q loop
	}//p loop
	return sum;
}

//count cols of load.txt by counting \n new line buffers
int FileCountCols(){
	FILE *F;
	F = fopen("load.txt", "r");
	int counter = 0;
	char cbuffer;

	for(cbuffer = getc(F); cbuffer != EOF; cbuffer = getc(F)){ //keep looping until end of file (EOF) is reached
		if(cbuffer == '\n'){
			counter += 1;
		}
	}
	fclose(F);
	return counter;
}

//counts the rows from a load.txt file, by using the result from FileCountCols and the total number of elements of the board
int FileCountRows(int Cols){
	FILE *F;
	F = fopen("load.txt", "r");
	int counter = 0;
	int dum[FMAX];

	while(counter < FMAX && fscanf(F, "%d", &dum[counter]) == 1){
		++counter;
	}
	fclose(F);
	return counter / Cols;//height and width = [a,b] area = ab , simple geometry
}

/*		Menu functions		*/
//asks user if they wish the current board to be saved
void SaveMenu(board_cell **Board, uint board_size[N]){
	printf("Would you like to save the current board to a file? Enter 1 for yes, 0 for no : ");
	int save_choice;
	int flag;
	int breaker_save = 1;
	while(breaker_save == 1){
        if((flag = scanf("%i", &save_choice)) != 1) save_choice = 0;
		breaker_save = 0;
		if(save_choice == 1){
			FileSave(Board, board_size);
		}
	}
}

//loads a board and then runs board
void LoadMenu(int grid){
	int flag;
	uint board_size[N];
	printf("\nLoading a custom game...\n");

	FILE *F;
	F = fopen("load.txt", "r");
	if(F == NULL){
		printf("Error accessing file!\n");
		return;
	}

	int Cols = FileCountCols();
	int Rows = FileCountRows(Cols);

	board_size[0] = Cols;
	board_size[1] = Rows;


	board_cell **Board = NULL; //pointer initialisation
	Board = GenerateBoard(board_size);
	FileLoad(Board, board_size);
	if(grid == 0){
		PrintBoard(Board, board_size);
	}
	else{
		PrintBoardGrid(Board, board_size);
	}
	printf("\nHow many generations to play the board for?");
	int End_Generation;
	if((flag = scanf("%i", &End_Generation)) != 1) return;

	printf("\nDelay between generation displays:");
	float delay;
	if((flag = scanf("%f", &delay)) != 1) return;
	if(delay <= 0) delay = 0.01; //prevents crashes due to how clock_t works

	printf("\n\nLOADING THE BOARD\n\n");
	Ticker(delay, Board, board_size, End_Generation, grid);
	FreeMemory(Board, board_size);
	return;

}

//menu for what to play
void PlayMenu(int grid){
	int flag;
	int option_play;
	int breaker_play = 1;
	while(breaker_play == 1){
		printf("\nPLAY MENU\n");
		printf("Option (0): Return to main menu\n");
		printf("Option (1): Play with a custom board\n");
		printf("Option (2): Browse predefined boards to use\n");
		printf("Option (3): Load from file\n");
		printf("Please enter the number that corresponds to your selection: ");
		if((flag = scanf("%i", &option_play)) != 1) option_play = 0;

		switch(option_play){
			case 0:
				printf("\n\n\n");
				breaker_play = 0;
				break;

			case 1: //custom game from within console
				CustomPlay(grid);
				continue;

			case 2: //predefined boards
				PreDefBoardMenu(grid);
				continue;

			case 3: //file load
				LoadMenu(grid);
				continue;
			
			default:
				printf("Unknown selection. Please try again");
				continue;
		}
	}
}


//create new board from scratch
void CustomPlay(int grid){
	int flag;
	uint board_size[N]={1,1};
	printf("\nCreating a custom game...\n");
	printf("Please enter two integers for the height and width, respectively, for the game board: ");
	flag = scanf("%u %u", &board_size[0], &board_size[1]);
	if(board_size[0] <= 0){
		printf("Board size must be greater than 0.\n");
		return;
	}
	if(board_size[1] <= 0){
		printf("Board size must be greater than 0.\n");
		return;
	}
	if(flag == 0){
		printf("Flag = 0!\n");
		return;
	}
	board_cell **Board = NULL; //pointer initialisation
	Board = GenerateBoard(board_size);
	if(grid == 0){
		PrintBoard(Board, board_size);
	}
	else{
		PrintBoardGrid(Board, board_size);
	}

	int customise_control = 1;
	while(customise_control == 1){
		CustomiseBoard(Board, board_size, grid);
		printf("Keep editing cells? 1 for yes, 0 for no : ");
		scanf("%i", &customise_control);
	}

	printf("\nHow many generations to play the board for?");
	int End_Generation;
	if((flag = scanf("%i", &End_Generation)) != 1) return;

	printf("\nDelay between generation displays:");
	float delay;
	if((flag = scanf("%f", &delay)) != 1) return;
	if(delay == 0) delay = 0.01;

	SaveMenu(Board, board_size);

	printf("\n\nLOADING THE BOARD\n\n");
	Ticker(delay, Board, board_size, End_Generation, grid);
	FreeMemory(Board, board_size);
	return;

}

/*		Menu for pre-defined boards	*/
void PreDefBoardMenu(int grid){
	int flag;
	int option_predef;
	int breaker_predef = 1;
	while(breaker_predef == 1){
		printf("\nPREDEFINED BOARDS MENU\n");
		printf("Option (0): Return to previous menu\n");
		printf("Option (1): Oscillator Sampler\n");
		printf("Option (2): Spaceship Sampler\n");
		printf("Please enter the number that corresponds to your selection: ");
		if((flag = scanf("%i", &option_predef)) != 1) option_predef = -1;
		board_cell **Board = NULL; //pointer initialisation
		uint board_size[N] = {32,32};
		Board = GenerateBoard(board_size);
		int End_Generation;
		float delay;

		switch(option_predef){
			case 0:
				printf("\n\n\n");
				breaker_predef = 0;
				break;
			case 1: //predefined board setup
				BoardOscillatorLoad(Board);
				BoardUpdate(Board, board_size);
				if(grid == 0){
					PrintBoard(Board, board_size);
				}
				else{
					PrintBoardGrid(Board, board_size);
				}
				printf("Would you like to customise the board before playing?\nEnter 1 for yes, 0 for no. : ");
				int predef_blinker_control;
				if((flag = scanf("%i", &predef_blinker_control)) == 0) return;
				printf("\n");
				while(predef_blinker_control == 1){
					CustomiseBoard(Board, board_size, grid);
					printf("Keep editing cells? 1 for yes, 0 for no : ");
					scanf("%i", &predef_blinker_control);
				}

				printf("\nHow many generations to play the board for?");

				if((flag = scanf("%i", &End_Generation)) != 1) return;
				if(End_Generation < 0){
					End_Generation = 0;
				}
				
				
				printf("\nDelay between generation displays:");
				if((flag = scanf("%f", &delay)) != 1) return;
				if(delay == 0) delay = 0.01;
				printf("\n\nLOADING THE BOARD\n\n");
				Ticker(delay, Board, board_size, End_Generation, grid);
				FreeMemory(Board, board_size);
				continue;


			case 2://spaceship board setup
				BoardSpaceshipLoad(Board);
				BoardUpdate(Board, board_size);
				if(grid == 0){
					PrintBoard(Board, board_size);
				}
				else{
					PrintBoardGrid(Board, board_size);
				}
				printf("Would you like to customise the board before playing?\nEnter 1 for yes, 0 for no. : ");
				int predef_space_control;
				if((flag = scanf("%i", &predef_space_control)) == 0) return;
				printf("\n");
				while(predef_space_control == 1){
					CustomiseBoard(Board, board_size, grid);
					printf("Keep editing cells? 1 for yes, 0 for no : ");
					scanf("%i", &predef_space_control);
				}

				printf("\nHow many generations to play the board for?");
				if((flag = scanf("%i", &End_Generation)) != 1) return;
				if(End_Generation < 0){
					End_Generation = 0;
				}
				
				printf("\nDelay between generation displays:");

				if((flag = scanf("%f", &delay)) != 1) return;
				if(delay <= 0) delay = 0.01;

				printf("\n\nLOADING THE BOARD\n\n");
				Ticker(delay, Board, board_size, End_Generation, grid);
				FreeMemory(Board, board_size);
				continue;


			default:
				printf("Unknown selection. Please try again");
				continue;
		}
	}
}

//customise any board
void CustomiseBoard(board_cell **Board, uint board_size[N], int grid){
	printf("\nPlease enter x coordinate in grid to flip state: ");
	uint j;
	int flag;
	if((flag = scanf("%u", &j)) == 0){
		printf("You did not enter an appropriate value\n");
		return;
	}
	if(j>=board_size[1]){
		printf("You entered a number greater than the board size.\n");
		j = board_size[1] -1;
		return;
	}
	printf("Please enter y coordinate in grid to flip state: ");
	uint i;
	if((flag = scanf("%u", &i) == 0)){
		printf("You did not enter an appropriate value\n");
		return;
	}
	if(i>=board_size[0]){
		printf("You entered a number greater than the board size.\n");
		i = board_size[0] -1;
		return;
	}

	int k = (int) i; //used uint before for the flag check, needs to be set back to int
	int l = (int) j;
	if(Board[k][l].status == 1) Board[k][l].next_gen = 0;
	if(Board[k][l].status == 0) Board[k][l].next_gen = 1;
	BoardUpdate(Board, board_size);
	if(grid == 0){
		PrintBoard(Board, board_size);
		}
	else PrintBoardGrid(Board, board_size);
	return;
}

/*		File functions		*/
//loads board from file "load.txt" in the root directory
void FileLoad(board_cell **Board, uint board_size[N]){
	FILE *F;
	F = fopen("load.txt", "r");

	int dummy_load = 0; //0 or 1, will be overwritten a bunch
    //scan vars from files
	for(int i=0; i<board_size[0]; ++i){
		for(int j=0; j<board_size[1]; ++j){
			fscanf(F, "%1i ", &dummy_load);
			Board[i][j].status = dummy_load;
		}
	}
	fclose(F);
	return;
}

//saves board to a file called "save.txt" in the root directory
void FileSave(board_cell **Board, uint board_size[N]){ 
	FILE *O;
	O = fopen("save.txt", "w");

	for(int i=0; i<board_size[0]; ++i){
		for(int j=0; j<board_size[1]; ++j){
			fprintf(O,"%i ", Board[i][j].status);
		}//j loop
		fprintf(O, "\n"); //new line at end of row
	}//i loop
	fclose(O);
	return;
}

/*Pre-defined board functions
	Would normally be in seperate files to be loaded but can only upload the one .c file so has to be put in functions	
*/
void BoardOscillatorLoad(board_cell **Board){ //different oscillators on one board
	/*BOARD SETUP FOR THIS EXAMPLE.	FROM https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life		*/

	//blinker period 1
	Board[1][13].next_gen= 1;
	Board[1][14].next_gen= 1;
	Board[1][15].next_gen= 1;
	//toad period 2

	Board[7][3].next_gen= 1;
	Board[8][4].next_gen= 1;
	Board[9][4].next_gen= 1;
	Board[8][1].next_gen= 1;
	Board[9][1].next_gen= 1;
	Board[10][2].next_gen= 1;

	//beacon period 2
	Board[1][26].next_gen= 1;
	Board[1][27].next_gen= 1;
	Board[2][26].next_gen= 1;
	Board[3][29].next_gen= 1;
	Board[4][29].next_gen= 1;
	Board[4][28].next_gen= 1;

	//penta-decathlon period 15
	Board[18][2].next_gen= 1;
	Board[18][3].next_gen= 1;
	Board[18][4].next_gen= 1;
	Board[19][1].next_gen= 1;
	Board[20][1].next_gen= 1;
	Board[19][5].next_gen= 1;
	Board[20][5].next_gen= 1;
	Board[21][2].next_gen= 1;
	Board[21][3].next_gen= 1;
	Board[21][4].next_gen= 1;
	Board[26][2].next_gen= 1;
	Board[26][3].next_gen= 1;
	Board[26][4].next_gen= 1;
	Board[27][5].next_gen= 1;
	Board[28][5].next_gen= 1;
	Board[27][1].next_gen= 1;
	Board[28][1].next_gen= 1;
	Board[29][2].next_gen= 1;
	Board[29][3].next_gen= 1;
	Board[29][4].next_gen= 1;

	//pulsar period 3
	Board[9][14].next_gen= 1;
	Board[10][14].next_gen= 1;
	Board[11][14].next_gen= 1;
	Board[11][15].next_gen= 1;
	Board[9][20].next_gen= 1;
	Board[10][20].next_gen= 1;
	Board[11][20].next_gen= 1;
	Board[11][19].next_gen= 1;
	Board[13][10].next_gen= 1;
	Board[13][11].next_gen= 1;
	Board[13][12].next_gen= 1;
	Board[14][12].next_gen= 1;
	Board[18][12].next_gen= 1;
	Board[19][12].next_gen= 1;
	Board[19][11].next_gen= 1;
	Board[19][10].next_gen= 1;
	Board[21][14].next_gen= 1;
	Board[21][14].next_gen= 1;
	Board[21][15].next_gen= 1;
	Board[22][14].next_gen= 1;
	Board[23][14].next_gen= 1;
	Board[21][19].next_gen= 1;
	Board[21][20].next_gen= 1;
	Board[22][20].next_gen= 1;
	Board[23][20].next_gen= 1;
	Board[13][15].next_gen= 1;
	Board[13][15].next_gen= 1;
	Board[13][16].next_gen= 1;
	Board[13][16].next_gen= 1;
	Board[14][14].next_gen= 1;
	Board[15][14].next_gen= 1;
	Board[14][14].next_gen= 1;
	Board[15][15].next_gen= 1;
	Board[13][18].next_gen= 1;
	Board[13][19].next_gen= 1;
	Board[14][18].next_gen= 1;
	Board[14][20].next_gen= 1;
	Board[15][20].next_gen= 1;
	Board[15][19].next_gen= 1;
	Board[17][19].next_gen= 1;
	Board[17][19].next_gen= 1;
	Board[17][20].next_gen= 1;
	Board[18][20].next_gen= 1;
	Board[18][18].next_gen= 1;
	Board[19][18].next_gen= 1;
	Board[19][19].next_gen= 1;
	Board[13][22].next_gen= 1;
	Board[13][22].next_gen= 1;
	Board[13][23].next_gen= 1;
	Board[13][24].next_gen= 1;
	Board[14][22].next_gen= 1;
	Board[18][22].next_gen= 1;
	Board[19][22].next_gen= 1;
	Board[19][22].next_gen= 1;
	Board[19][23].next_gen= 1;
	Board[19][24].next_gen= 1;
	Board[17][14].next_gen= 1;
	Board[17][15].next_gen= 1;
	Board[18][14].next_gen= 1;
	Board[18][16].next_gen= 1;
	Board[19][15].next_gen= 1;
	Board[19][16].next_gen= 1;
	Board[14][16].next_gen= 1;
}


void BoardSpaceshipLoad(board_cell **Board){ //different spaceships on one board
	/*	Spaceships from https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life 	*/
	//light spaceship
	Board[1][3].next_gen = 1;
	Board[1][6].next_gen = 1;
	Board[2][7].next_gen = 1;
	Board[3][3].next_gen = 1;
	Board[3][7].next_gen = 1;
	Board[4][4].next_gen = 1;
	Board[4][5].next_gen = 1;
	Board[4][6].next_gen = 1;
	Board[4][7].next_gen = 1;

	//medium spaceship
	Board[10][3].next_gen = 1;
	Board[10][4].next_gen = 1;
	Board[10][5].next_gen = 1;
	Board[10][6].next_gen = 1;
	Board[10][7].next_gen = 1;
	Board[11][2].next_gen = 1;
	Board[11][7].next_gen = 1;
	Board[12][7].next_gen = 1;
	Board[13][2].next_gen = 1;
	Board[13][6].next_gen = 1;
	Board[14][4].next_gen = 1;

	//heavy spaceship
	Board[20][2].next_gen = 1;
	Board[20][3].next_gen = 1;
	Board[20][4].next_gen = 1;
	Board[20][5].next_gen = 1;
	Board[20][6].next_gen = 1;
	Board[20][7].next_gen = 1;
	Board[21][1].next_gen = 1;
	Board[21][7].next_gen = 1;
	Board[22][7].next_gen = 1;
	Board[23][1].next_gen = 1;
	Board[23][6].next_gen = 1;
	Board[24][3].next_gen = 1;
	Board[24][4].next_gen = 1;
}