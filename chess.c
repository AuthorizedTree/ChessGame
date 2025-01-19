#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <term.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#define KVDB_STORE_URL "https://kvdb.io/3dr3Q56Ta3bihD7oq4cKMi/"
#define KVDB_RETRIEVE_URL "https://kvdb.io/3dr3Q56Ta3bihD7oq4cKMi/"



wchar_t** initializeChessboard() {
    // Initialize the chessboard
    wchar_t pieces[8][8] = {
        {L'♜', L'♞', L'♝', L'♛', L'♚', L'♝', L'♞', L'♜'},
        {L'♟', L'♟', L'♟', L'♟', L'♟', L'♟', L'♟', L'♟'},
        {L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' '},
        {L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' '},
        {L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' '},
        {L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' '},
        {L'♙', L'♙', L'♙', L'♙', L'♙', L'♙', L'♙', L'♙'},
        {L'♖', L'♘', L'♗', L'♕', L'♔', L'♗', L'♘', L'♖'}
    };

    wchar_t** chessboard = malloc(8 * sizeof(wchar_t*));
    for (int i = 0; i < 8; i++) {
        chessboard[i] = malloc(8 * sizeof(wchar_t));
        for (int j = 0; j < 8; j++) {
            chessboard[i][j] = pieces[i][j];
        }
    }

    return chessboard;
}
wchar_t convertFENToPiece(char piece) {
    switch (piece) {
        case 'r': return L'♜';
        case 'n': return L'♞';
        case 'b': return L'♝';
        case 'q': return L'♛';
        case 'k': return L'♚';
        case 'p': return L'♟';
        case 'R': return L'♖';
        case 'N': return L'♘';
        case 'B': return L'♗';
        case 'Q': return L'♕';
        case 'K': return L'♔';
        case 'P': return L'♙';
        default: return L' ';
    }
}
wchar_t** createBoardFromFEN(char *fen) {
    #define BOARD_SIZE 8
    // Extracting board layout from FEN
    wchar_t** board = malloc(BOARD_SIZE * sizeof(wchar_t*));
    for (int i = 0; i < BOARD_SIZE; i++) {
        board[i] = malloc(BOARD_SIZE * sizeof(wchar_t));
    }

    int row = 0, col = 0;
    for (int i = 0; fen[i] != '\0'; i++) {
        if (fen[i] == '/') {
            row++;
            col = 0;
        } else if (fen[i] >= '1' && fen[i] <= '8') {
            int spaces = fen[i] - '0';
            for (int j = 0; j < spaces; j++) {
                board[row][col++] = L' ';
            }
        } else {
            board[row][col++] = convertFENToPiece(fen[i]);
        }
    }

    return board;
}
char convertPieceToFEN(wchar_t piece) {
    switch (piece) {
        case L'♜': return 'r';
        case L'♞': return 'n';
        case L'♝': return 'b';
        case L'♛': return 'q';
        case L'♚': return 'k';
        case L'♟': return 'p';
        case L'♖': return 'R';
        case L'♘': return 'N';
        case L'♗': return 'B';
        case L'♕': return 'Q';
        case L'♔': return 'K';
        case L'♙': return 'P';
        default: return '.';
    }
}
char* printFENFromBoard(wchar_t** board) {
    char* fen = malloc(100 * sizeof(char)); // Dynamically allocate memory for the FEN string
    int index = 0;

    // Convert the board to FEN format
for (int i = 0; i < BOARD_SIZE; i++) {
    int emptyCounter = 0;
    for (int j = 0; j < BOARD_SIZE; j++) {
        wchar_t piece = board[i][j];
        if (piece == L' ') {
            emptyCounter++;
        } else {
            if (emptyCounter > 0) {
                fen[index++] = '0' + emptyCounter;
                emptyCounter = 0;
            }
            // Convert the piece to the corresponding character in FEN format
            fen[index++] = convertPieceToFEN(piece);
        }
    }
    if (emptyCounter > 0) {
        fen[index++] = '0' + emptyCounter;
    }
    if (i < BOARD_SIZE - 1) {
        fen[index++] = '/';
    } 
}


    // Null-terminate the FEN string
    fen[index] = '\0';

    // Print the FEN format

    return fen;
}
size_t kvdb_curl_write_callback(void *contents, size_t size, size_t nmemb, char **response) {
    size_t realsize = size * nmemb;
    *response = (char *)realloc(*response, realsize + 1);
    if (*response == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }
    memcpy(*response, contents, realsize);
    (*response)[realsize] = '\0';
    return realsize;
}
int kvdb_store(const char *key, const char *value) {
    CURL *curl;
    CURLcode res;
    char *url = NULL;
    char *post_fields = NULL;
    char *response = NULL;
    int success = 0;

    curl = curl_easy_init();
    if (curl) {
        url = (char *)malloc(strlen(KVDB_STORE_URL) + strlen(key) + strlen(value) + 2); // Allocate memory for the URL
        if (url == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 0;
        }
        sprintf(url, "%s%s", KVDB_STORE_URL, key);

        post_fields = (char *)malloc(strlen(value) + 1);
        if (post_fields == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            free(url); // Free the previously allocated memory
            return 0;
        }
        strcpy(post_fields, value);

        curl_easy_setopt(curl, CURLOPT_URL, url); 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields);

        // Perform the HTTP POST request
        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            success = 1;
        } else {
            fprintf(stderr, "HTTP request failed: %s\n", curl_easy_strerror(res));
        }

        free(url);
        free(post_fields);
        curl_easy_cleanup(curl);
    }

    return success;
}
char *kvdb_retrieve(const char *key) {
    CURL *curl;
    CURLcode res;
    char *url = NULL;
    char *response = NULL;

    curl = curl_easy_init();
    if (curl) {
        url = (char *)malloc(strlen(KVDB_RETRIEVE_URL) + strlen(key) + 1); // Allocate memory for the URL
        if (url == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return NULL;
        }
        sprintf(url, "%s%s", KVDB_RETRIEVE_URL, key);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, kvdb_curl_write_callback); // Use renamed callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the HTTP GET request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "HTTP request failed: %s\n", curl_easy_strerror(res));
            free(response);
            response = NULL;
        }

        free(url);
        curl_easy_cleanup(curl);
    }

    return response;
}


void playerChoice(int *player1_color, int *player2_color, int * player) {
    wprintf(L"\nPlayer 1, select your color:\n");
    wprintf(L"1. White\n");
    wprintf(L"2. Black\n");
    wprintf(L"\nwhite pieces are represented by: ♖ ♘ ♗ ♕ ♔ ♙\n");
    wprintf(L"black pieces are represented by: ♜ ♞ ♝ ♛ ♚ ♟\n");
    wprintf(L"\nEnter your choice: ");
    scanf("%d", player1_color);

    switch(*player1_color) {
        case 1:
            wprintf(L"\nPlayer 1 has chosen White.\n");
            *player2_color = 2;
            *player = 1; // Player 2 automatically gets Black
            break;
        case 2:
            wprintf(L"\nPlayer 1 has chosen Black.\n");
            *player2_color = 1;
            *player = 0;// Player 2 automatically gets White
            break;
        default:
            wprintf(L"\nInvalid choice for Player 1. Defaulting to White.\n");
            *player1_color = 1;
            *player2_color = 2;
            *player = 1;
    }
}


int validate_move(int scol, int srow, int dcol, int drow, wchar_t** chessar, int player_color) {

    // Check if source and destination positions are within the board
    if (scol < 0 || scol > 7 || srow < 0 || srow > 7 || dcol < 0 || dcol > 7 || drow < 0 || drow > 7)
        return 0;

    // Check if there's a piece at the source position
    wchar_t piece = chessar[srow][scol];
    wchar_t destination = chessar[drow][dcol];

    if (piece == L' ') // Ensures the source postion contains an actual chess piece 
        return 0;

    // Check if the destination position is the same as the source position
    if (scol == dcol && srow == drow)
        return 0;

    if (player_color == 1 && (piece == L'♜' || piece == L'♞' || piece == L'♝' || piece == L'♛' || piece == L'♚' || piece == L'♟')) {
        return 0;
    }

    if (player_color == 2 && (piece == L'♖' || piece == L'♘' || piece == L'♗' || piece == L'♕' || piece == L'♔' || piece == L'♙')) {
        return 0;
       
    }
   
    

  
    // Pawn movement
    if (piece == L'♙' || piece == L'♟') {

        
        int direction = (piece == L'♙') ? 1 : -1; // White pawn moves upward, black moves downward

        // Check if the destination is one square forward
        if (dcol == scol && drow == srow - direction && (chessar[drow][dcol] == L' ' || (piece == L'♙' && (destination == L'♜' || destination == L'♞' || destination == L'♝' || destination == L'♛' || destination == L'♚' || destination == L'♟')) || (piece == L'♟' && (destination == L'♖' || destination == L'♘' || destination == L'♗' || destination == L'♕' || destination == L'♔' || destination == L'♙')) ))
            return 1;

        // Check if the destination is two squares forward (for the first move)
       

        if (srow == (piece == L'♙' ? 6 : 1) && dcol == scol && drow == srow - 2 * direction && chessar[drow][dcol] == L' ' && chessar[srow - direction][scol] == L' ')
            return 1;

       // Check if the destination is a capture diagonally
        if ((dcol == scol + 1 || dcol == scol - 1) && drow == srow - direction) {
            // Check if the destination contains an enemy piece
            wchar_t destination = chessar[drow][dcol];
            if (destination != L' ') {
                if (piece == L'♙' && (destination == L'♜' || destination == L'♞' || destination == L'♝' || destination == L'♛' || destination == L'♚' || destination == L'♟')) {
                    return 1; // Valid capture for white pawn
                }
                if (piece == L'♟' && (destination == L'♖' || destination == L'♘' || destination == L'♗' || destination == L'♕' || destination == L'♔' || destination == L'♙')) {
                    return 1; // Valid capture for black pawn
                }
            }
        }


        // Otherwise, invalid move
        return 0;
    }

    // Rook movement
    if (piece == L'♖' || piece == L'♜') {
        // Check if the move is in a straight line
        if (scol != dcol && srow != drow) {
            return 0;
        }

        // Check if the path is clear
        if (scol == dcol) {
            // Moving vertically
            int start = srow < drow ? srow : drow;
            int end = srow > drow ? srow : drow;
            for (int i = start + 1; i < end; i++) {
                if (chessar[i][scol] != L' ') {
                    return 0;
                }
            }
        } else {
            // Moving horizontally
            int start = scol < dcol ? scol : dcol;
            int end = scol > dcol ? scol : dcol;
            for (int i = start + 1; i < end; i++) {
                if (chessar[srow][i] != L' ') {
                    return 0;
                }
            }
        }

        // Check if the destination is clear or contains an enemy piece

        if (destination == L' ') {
            return 1;
        } else if ((piece == L'♖' && (destination == L'♜' || destination == L'♞' || destination == L'♝' || destination == L'♛' || destination == L'♚' || destination == L'♟')) ||
         (piece == L'♜' && (destination == L'♖' || destination == L'♘' || destination == L'♗' || destination == L'♕' || destination == L'♔' || destination == L'♙'))) {
            return 1;
}

    }

    //Knight Movement
    if (piece == L'♘' || piece == L'♞') {
    // Check if the destination is reachable by a knight's L-shaped move
    int diff_col = abs(dcol - scol);
    int diff_row = abs(drow - srow);
    if ((diff_col == 2 && diff_row == 1) || (diff_col == 1 && diff_row == 2)) {
        // Check if the destination is clear or contains an enemy piece

        if (destination == L' ') {
            return 1; // Valid knight move
        } else if ((piece == L'♘' && (destination == L'♜' || destination == L'♞' || destination == L'♝' || destination == L'♛' || destination == L'♚' || destination == L'♟')) ||
         (piece == L'♞' && (destination == L'♖' || destination == L'♘' || destination == L'♗' || destination == L'♕' || destination == L'♔' || destination == L'♙'))) {
              return 1; // Valid knight move, capturing an enemy piece
        } else {
            return 0; // Invalid move, destination contains a piece of the same team
        }
    } else {
        return 0; // Invalid move for a knight
    }
    }



    // Bishop movement
     if (piece == L'♗' || piece == L'♝') {
        // Calculate the absolute differences between source and destination coordinates
        int row_diff = drow - srow;
        int col_diff = dcol - scol;

        // Ensure the absolute differences in row and column are equal
        if (abs(row_diff) != abs(col_diff)) {
            return 0; // Invalid move, not along a diagonal
        }

        // Determine the direction of movement
        int row_step = (row_diff > 0) ? 1 : -1;
        int col_step = (col_diff > 0) ? 1 : -1;

        // Check for obstacles along the diagonal path
        int row = srow + row_step;
        int col = scol + col_step;
        while (row != drow && col != dcol) {
            // Check if there's a piece at the current position
            if (chessar[row][col] != L' ' && chessar[row][col] != L'\0') {
                return 0; // Invalid move, obstacle in the path
            }
            // Move to the next position along the diagonal
            row += row_step;
            col += col_step;
        }

        // Check if the destination position contains an enemy piece or is empty

        if (destination == L' ' ||
    ((piece == L'♗' && (destination == L'♜' || destination == L'♞' || destination == L'♝' || destination == L'♛' || destination == L'♚' || destination == L'♟')) ||
     (piece == L'♝' && (destination == L'♖' || destination == L'♘' || destination == L'♗' || destination == L'♕' || destination == L'♔' || destination == L'♙')))) {
        return 1; // Valid move
        }
    }
    // Queen movement
    if (piece == L'♕' || piece == L'♛') {
        int dx = dcol - scol;
        int dy = drow - srow;
        int stepx, stepy;

        // Check if the move is in a straight line or diagonal
        if(dx == 0 || dy == 0 || abs(dx) == abs(dy)) {
            stepx = (dx > 0) - (dx < 0); // will be -1, 0, or 1
            stepy = (dy > 0) - (dy < 0); // will be -1, 0, or 1

            // Check each square along the path for a piece
            for(int x = scol + stepx, y = srow + stepy; x != dcol || y != drow; x += stepx, y += stepy) {
                if(chessar[y][x] != L' ') {
                    // There's a piece in the path, move is invalid
                    return 0;
                }
            }

            // Check the destination square

            if(destination != L' ') {
             if((piece == L'♕' && (destination == L'♜' || destination == L'♞' || destination == L'♝' || destination == L'♛' || destination == L'♚' || destination == L'♟')) ||
             (piece == L'♛' && (destination == L'♖' || destination == L'♘' || destination == L'♗' || destination == L'♕' || destination == L'♔' || destination == L'♙'))) {
        // There's a piece of the opposite color at the destination, move is valid
                 return 1;
          } else {
        // There's a piece of the same color at the destination, move is invalid
                return 0;
            }
        }


            // The move is valid
            return 1;
        }
    }





    // King movement
   if (piece == L'♔' || piece == L'♚') {
    // Check if the destination is reachable by the king's move
    int diff_col = abs(dcol - scol);
    int diff_row = abs(drow - srow);
    if (diff_col <= 1 && diff_row <= 1) {
        // The destination is reachable by the king's move
        if (destination != L' ') {
            // There's a piece at the destination
            if ((piece == L'♔' && (destination == L'♜' || destination == L'♞' || destination == L'♝' || destination == L'♛' || destination == L'♚' || destination == L'♟')) ||
                (piece == L'♚' && (destination == L'♖' || destination == L'♘' || destination == L'♗' || destination == L'♕' || destination == L'♔' || destination == L'♙'))) {
                // There's a piece of the opposite color at the destination, move is valid
                return 1;
            } else {
                // There's a piece of the same color at the destination, move is invalid
                return 0;
            }
        } else {
            // The destination square is empty, move is valid
            return 1;
        }
    } else {
        // The destination is not reachable by the king's move, move is invalid
        return 0;
    }
    }

    // If none of the piece-specific rules match, return invalid move
    return 0;
}

void convertPos(char srcrow, char srccol, int *dstrow, int *dstcol) {
    *dstcol = srccol - 'A'; // Convert column character to array index
    *dstrow = '8' - srcrow; // Convert row character to array index
}



void displayChessboard(wchar_t** chessboard) {
    wprintf(L"  a b c d e f g h\n");
    for (int i = 0; i < 8; i++) {
        wprintf(L"%d ", 8 - i);
        for (int j = 0; j < 8; j++) {
            wprintf(L"%lc ", chessboard[i][j]);
        }
        wprintf(L"\n");
    }
}

wchar_t** retrieveGame(wchar_t** chessboard, int *player1_color, int *player2_color, int *player, char *save_key) {
    char response[256];
    strcpy(response, kvdb_retrieve(save_key));
    char fen[100];
        char *token = strtok(response, ",");
    if (token != NULL) {
        strcpy(fen, token);  // Copy the first token to fen
        token = strtok(NULL, ",");
        if (token != NULL) {
            *player = atoi(token);  // Convert the second token to int and store it in player
            token = strtok(NULL, ",");
            if (token != NULL) {
                *player1_color = atoi(token);  // Convert the third token to int and store it in player1_color
                token = strtok(NULL, ",");
                if (token != NULL) {
                    *player2_color = atoi(token);  // Convert the fourth token to int and store it in player2_color
                }
            }
        }
    }






chessboard = createBoardFromFEN(fen);





    return chessboard;
}
void save_game(wchar_t** chessboard, int player, int player1_color, int player2_color, char *save_key) {
    char* fen = printFENFromBoard(chessboard);
    char store[256];
    sprintf(store, "%s,%d,%d,%d", fen, player, player1_color, player2_color);
    kvdb_store(save_key, store);
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char date[11];
    sprintf(date, "%02d/%02d/%04d", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900);

    // Open the file in append mode
    FILE* file = fopen("saved.txt", "a");
    if (file != NULL) {
        // Write the save_key and the date to the file
        fprintf(file, "Save Name: %s Date: %s\n", save_key, date);
        // Close the file
        wprintf(L"Game Saved Successfully!\n");
        fclose(file);
    } else {
        printf("Error opening file!\n");
    }

}
int printSaves() {
    FILE* file = fopen("saved.txt", "r");
    if (file == NULL) {
        
    } else {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            printf("%s", line);
        }
        fclose(file);
        return 1;
    }
    return 0;
}



int main() {
    setlocale(LC_ALL, "");  // Set the locale of the program
    int count = 0;
    wchar_t** chessboard = initializeChessboard();
    int player1_color, player2_color;
    int choice;

    wprintf(L"\n********Welcome to the Chess Game!********\n");
    int player = 0;
    wprintf(L"Please Select an Option Below:\n");
    wprintf(L"1. New Game \n");
    wprintf(L"2. Resume Saved Game \n");
    wprintf(L"Enter your choice: ");
    scanf("%d", &choice);
    char save[50];

    switch(choice){
        case 1:
            wprintf(L"\nStarting New Game....\n");
            playerChoice(&player1_color, &player2_color, &player);
             wprintf(L"Player 2 gets the opposite color.\n");
             wprintf(L"Player 2's color: %s\n\n", (player2_color == 1) ? "White" : "Black");

             if (player2_color == 2) {
                    wprintf(L"White Moves First, Player 1's Turn: \n");
            } else {
                wprintf(L"White Moves First, Player 2's Turn: \n");
                }
            break;
        case 2:
            wprintf(L"\nSaved Games:\n");
            if(printSaves() == 1) {
                wprintf(L"\n");
                wprintf(L"Enter the Save Key: ");
              scanf("%s", save);
            wprintf(L"\nResuming Saved Game...\n");
            wprintf(L"\n");
            chessboard = retrieveGame(chessboard, &player1_color, &player2_color, &player, save);
            break;
            }else {
                wprintf(L"Invalid Choice. Starting New Game\n");
            }
           
            break;
        default:
            wprintf(L"Invalid Choice. Starting New Game\n");
            break;
    }

    
    displayChessboard(chessboard);
    wprintf(L"\n");


    while (1) {
    wprintf(L"Please Select an Option Below:\n");
    wprintf(L"1. Continue Game \n");
    wprintf(L"2. Save Game \n");
    wprintf(L"3. Forfeit Game \n");
    wprintf(L"Enter your choice: ");
    scanf("%d", &choice);
    char save_key[50];

    switch(choice){
        case 1:
            if (player == 1) {
                wprintf(L"\nPlayer 1 Has Chosen to Continue the Game.\n");
            } else {
                wprintf(L"\nPlayer 2 Has Chosen to Continue the Game.\n");
            }
            break;
        case 2:
            if (player == 1) {
                wprintf(L"\nPlayer 1 Has Chosen to Save the Game.\n");
            } else {
                wprintf(L"\nPlayer 2 Has Chosen to Save the Game.\n");
            }
            wprintf(L"\nEnter a Save Key: ");
            scanf("%s", save_key);
            wprintf(L"\nThank You for Saving Your Game. \n");
            save_game(chessboard, player, player1_color, player2_color, save_key);
            exit(0);
        case 3:
            if (player == 1) {
                wprintf(L"\nPlayer 1 Has Chosen to Forfeit the Game.\n");
                wprintf(L"Congradulations Player 2!\n");
            } else {
                wprintf(L"\nPlayer 2 Has Chosen to Forfeit the Game.\n");
                wprintf(L"Congradulations Player 1!\n");
            }
            exit(0);
        default:
            wprintf(L"\nInvalid Choice. Continuing Game.\n");
            break;
    }

        if (player == 1) {
             // Prompt the user to enter the piece to be moved
            wprintf(L"Player 1's Turn: \n");
            wprintf(L"\nEnter piece to be moved (ex. A 1): ");
            char srcCol, srcRow, dstCol, dstRow;
            int srcRowInt, srcColInt, dstRowInt, dstColInt;
            scanf(" %c %c", &srcCol, &srcRow);
            wprintf(L"Enter the new board location (ex. A 1): ");
            scanf(" %c %c", &dstCol, &dstRow);
            convertPos(srcRow, srcCol, &srcRowInt, &srcColInt);
            convertPos(dstRow, dstCol, &dstRowInt, &dstColInt);
            int test = validate_move(srcColInt, srcRowInt, dstColInt, dstRowInt, chessboard, player1_color); //Check if the move is valid

            if (test == 1) {
                if(chessboard[dstRowInt][dstColInt] == L'♔' || chessboard[dstRowInt][dstColInt] == L'♚') { //Check if the game has been won
                    wprintf(L"Congrats!\n"); 
                    wprintf(L"Player 1 Has Won!\n");
                    exit(0);
                }
                chessboard[dstRowInt][dstColInt] = chessboard[srcRowInt][srcColInt]; //Move the piece to the new location
                chessboard[srcRowInt][srcColInt] = L' '; //Clear the old location
                wprintf(L"\n");
                displayChessboard(chessboard);
                wprintf(L"\n");
                player = (player == 1) ? 0 : 1; //Switch player turn value
                count++;

            } else {
                wprintf(L"\nInvalid move. Please Try Again. \n");
                wprintf(L"\n");
                displayChessboard(chessboard);
                wprintf(L"\n");

                
            }
        } else {
            wprintf(L"Player 2's Turn: \n");
            // Prompt the user to enter the piece to be moved
            wprintf(L"\nEnter piece to be moved (ex. A 1): ");
            char srcCol, srcRow, dstCol, dstRow;
            int srcRowInt, srcColInt, dstRowInt, dstColInt;
            scanf(" %c %c", &srcCol, &srcRow);
            wprintf(L"Enter the new board location (ex. A 1): ");
            scanf(" %c %c", &dstCol, &dstRow);
            convertPos(srcRow, srcCol, &srcRowInt, &srcColInt);
            convertPos(dstRow, dstCol, &dstRowInt, &dstColInt);
            int test = validate_move(srcColInt, srcRowInt, dstColInt, dstRowInt, chessboard, player2_color);

            if (test == 1) {
                if(chessboard[dstRowInt][dstColInt] == L'♔' || chessboard[dstRowInt][dstColInt] == L'♚') {
                    wprintf(L"\nCongrats!\n");
                    wprintf(L"Player 2 Has Won!\n");
                    exit(0);
                }
                chessboard[dstRowInt][dstColInt] = chessboard[srcRowInt][srcColInt];
                chessboard[srcRowInt][srcColInt] = L' ';
                wprintf(L"\n");
                displayChessboard(chessboard);
                wprintf(L"\n");
                player = (player == 1) ? 0 : 1;
                count++;
            } else {
                wprintf(L"\nInvalid move. Please Try Again. \n");
                wprintf(L"\n");
                displayChessboard(chessboard);
                wprintf(L"\n");
               
            }
        }

    } while (count<120); //Limit the number of moves to 120




    // Free memory
    for (int i = 0; i < 8; i++) {
        free(chessboard[i]);
    }
    free(chessboard);

    return 0;
}

