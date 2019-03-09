#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

/*
TODO:
	Improve algorithm
		Make sure program does not infinite loop
		Allow program to make checkpoints and guess, if no singular path forward
	Improve user interface
		Notes on how to use the program
	Check if everything works on UNIX
*/

//Use a bit field to keep track of possible entries for each cell
struct Note {
	//MSB = 9, LSB = 1;
	unsigned int note_values: 9;
};

//Create cells for each space on the game board
class Cell {
	private:
		int m_row;
		int m_col;
		int m_groupCellNum;
		int m_localCellNum;
		Note m_note;
		int m_value;
		
	public:
		//Constructor
		Cell(int row = 0, int col = 0) {
			m_row = row;
			m_col = col;
			//Figure out what group the cell is in (bunch of 9 cells)
			m_groupCellNum = (((m_row - 1) / 3) * 3) + ((m_col - 1) / 3) + 1;
			//Figure out what local cell is (within the group cell)
			m_localCellNum = (((m_row - 1) % 3) * 3) + ((m_col - 1) % 3) + 1;
			m_note.note_values = 0b000000000;
			m_value = 0;
		}
		
		//Set functions
		void clearNotes() {
			m_note.note_values = 0b000000000;
		}
		void makeNote(int noteVal) {
			//Add note for the given value (an integer from 1 to 9)
			m_note.note_values |= static_cast<int>(pow(2, (noteVal - 1)));
		}
		void removeNote(int noteVal) {
			//Remove note for the given value (an integer from 1 to 9)
			m_note.note_values &= ~static_cast<int>(pow(2, (noteVal - 1)));
		}
		void setValue(int value) {
			m_value = value;
		}
		
		//Get functions
		int getRow() {
			return m_row;
		}
		int getCol() {
			return m_col;
		}
		int getGroupCellNum() {
			return m_groupCellNum;
		}
		int getLocalCellNum() {
			return m_localCellNum;
		}
		int getValue() {
			return m_value;
		}
		int getNotes() {
			return m_note.note_values;
		}
		void printNotes() {
			std::cout << m_note.note_values << std::endl;
		}
		bool hasNote(int noteVal) {
			//Check if a value possible in the cell (unused, may be used for better algorithm)
			return m_note.note_values & static_cast<int>(pow(2, (noteVal - 1)));
		}
		bool hasOnlyOneNote () {
			//Check if power of 2
			return m_note.note_values && !(m_note.note_values & (m_note.note_values - 1));
		}
};

int readFile(int board[10][10], std::string fileName, int* numUnsolved);
void printBoard(Cell board[10][10], int numUnsolved);
bool validNumber(Cell board[10][10], int row, int col, int cellValue);
int createNotes(Cell board[10][10]);
int updateNotes(Cell board[10][10], int row, int col, int cellValue);
bool inRow(Cell board[10][10], int row, int value);
bool inCol(Cell board[10][10], int col, int value);
bool inGroup(Cell board[10][10], int row, int col, int value);
bool isSolved(Cell board[10][10]);

int main(int argc, char* argv[]) {	
	if (argc != 2) {
		std::cout << "Program usage: <program.exe> <fileToSolve.txt>\n";
	}

	//Initialize game
	int countUnsolved = 81;
	
	//Read board from file
	int readBoard[10][10];
	switch(readFile(readBoard, argv[1], &countUnsolved)) {
		case -1: 
			return -1;
			break;
	}

	//Initialize game board with default squares
	Cell gameBoard[10][10];
	
	//Fill in known squares and other parameters for still-unknown cells
	for (int row = 1; row <= 9; row++) {
		for (int col = 1; col <= 9; col++) {
			gameBoard[row][col] = Cell(row, col);
			gameBoard[row][col].setValue(readBoard[row][col]);
		}
	}
	
	//Print out the unsolved board
	printBoard(gameBoard, countUnsolved);
	
	//Make notes for each unknown cell
	createNotes(gameBoard);
	
	std::cout << "Solving puzzle";
	
	//While there are unsolved cells...
	while (countUnsolved > 0) {
		for (int row = 1; row <= 9; row++) {
			for (int col = 1; col <= 9; col++) {
				//Locate cells where there is only one possible value
				if (gameBoard[row][col].hasOnlyOneNote()) {
					//Fill in that value
					gameBoard[row][col].setValue(log2(gameBoard[row][col].getNotes()) + 1);
					//Let related cells know that the value is no longer possible
					updateNotes(gameBoard, row, col, gameBoard[row][col].getValue());
					//Decrement number of unsolved cells
					countUnsolved--;
					//Update readout
					std::cout << ".";
				}
			}
		}
	}
	
	std::cout << std::endl;
	
	//Print solved board
	printBoard(gameBoard, countUnsolved);
		
	return 0;
}

//Read the game board from file 
int readFile(int board[10][10], std::string fileName, int* numUnsolved) {
	//Open file
	std::ifstream inputFile;
	inputFile.open(fileName);
	std::string inputLine;	
	int rawVal;
	
	if (inputFile.fail()) {
		std::cout << "Input puzzle file was not found. Please try again." << std::endl;
		return -1;
	}
	
	//Load integers from file into board
	else {
		for (int row = 1; row <= 9; row++) {
			for (int col = 1; col <= 9; col++) {
				inputFile >> std::ws;
				inputFile >> board[row][col];
				if (board[row][col] != 0) {
					//If value already known, decrement unknown counter
					--(*numUnsolved);
				}
			}
		}
	}
	
	return 0;
}

//Print out every cell value
void printBoard(Cell board[10][10], int numUnsolved) {
	for (int row = 1; row <= 9; row++) {
		for (int col = 1; col <= 9; col++) {
			std::cout << board[row][col].getValue() << " ";
		}
		std::cout << std::endl;
	}
	
	std::cout << numUnsolved << " unsolved cells!" << std::endl;
	
	return;
}

//Check if cellValue can go into the board at position [row][col]
bool validNumber(Cell board[10][10], int row, int col, int cellValue) {
	bool goodRow = !inRow(board, row, cellValue);
	bool goodCol = !inCol(board, col, cellValue);
	bool goodGroup = !inGroup(board, row, col, cellValue);
	
	return goodRow && goodCol && goodGroup;
}

//Initalize notes for all cells on the board
int createNotes(Cell board[10][10]) {
	int numUnsolved = 0;
	
	for (int row = 1; row <= 9; row++) {
		for (int col = 1; col <= 9; col++) {
			if (board[row][col].getValue() == 0) {
				numUnsolved++;
				for (int guess = 1; guess <= 9; guess++) {
					if(validNumber(board, row, col, guess)) {
						board[row][col].makeNote(guess);
					}
				}
			}
		}
	}
	
	return numUnsolved;
}

//Remove possibility for cellValue in the groupCell, row, and column
int updateNotes(Cell board[10][10], int row, int col, int cellValue) {
	int origin_row = ((row - 1) / 3) * 3 + 1;
	int origin_col = ((col - 1) / 3) * 3 + 1;
	
	//Remove note for value in cell
	for (int row_offset = 0; row_offset <= 2; row_offset++) {
		for (int col_offset = 0; col_offset <= 2; col_offset++) {
			board[origin_row + row_offset][origin_col + col_offset].removeNote(cellValue);
		}
	}
	//Remove note for value in row
	for (int i_col = 1; i_col <= 9; i_col++) {
		board[row][i_col].removeNote(cellValue);
	}
	//Remove note for value in col
	for (int i_row = 1; i_row <= 9; i_row++) {
		board[i_row][col].removeNote(cellValue);
	}
}

//Check if value already in row
bool inRow(Cell board[10][10], int row, int value) {
	for (int col = 1; col <= 9; col++) {
		if (board[row][col].getValue() == value) {
			return true;
		}
	}
	
	return false;
}

//Check if value already in column
bool inCol(Cell board[10][10], int col, int value) {
	for (int row = 1; row <= 9; row++) {
		if (board[row][col].getValue() == value) {
			return true;
		}
	}
	
	return false;
}

//Check if value already in groupCell
bool inGroup(Cell board[10][10], int row, int col, int value) {
	//Find location of top-left member of groupCell
	int origin_row = ((row - 1) / 3) * 3 + 1;
	int origin_col = ((col - 1) / 3) * 3 + 1;
	
	//Offest from top-left to check rest of groupCell
	for (int row_offset = 0; row_offset <= 2; row_offset++) {
		for (int col_offset = 0; col_offset <= 2; col_offset++) {
			if (board[origin_row + row_offset][origin_col + col_offset].getValue() == value) {
				return true;
			}
		}
	}
	
	return false;
}

//In progress: verify that the game board is solved
bool isSolved(Cell board[10][10]) {
		
	for (int row = 1; row <= 9; row++) {
		for (int col = 1; col <= 9; col++) {
			//Figure this out later. Need to verify doneness
		}
	}
	
}
