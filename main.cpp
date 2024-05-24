#include <iostream>
#include <cstdlib>
#include <ctime>
#include <SDL.h>
#include <SDL_ttf.h>

using namespace std;

#define N 9
#define CELL_SIZE 50
#define GRID_SIZE (N * CELL_SIZE)
#define WINDOW_SIZE GRID_SIZE

void drawThickLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int thickness) {
    if (thickness <= 1) {
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        return;
    }

    SDL_Rect rect;
    if (x1 == x2) {
        rect = { x1 - thickness / 2, y1, thickness, y2 - y1 };
    } else if (y1 == y2) {
        rect = { x1, y1 - thickness / 2, x2 - x1, thickness };
    }
    SDL_RenderFillRect(renderer, &rect);
}

void printSudoku(SDL_Renderer* renderer, TTF_Font* font, int grid[N][N], int currentRow, int currentCol) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    for (int i = 0; i <= N; i++) {
        int pos = i * CELL_SIZE;
        if (i % 3 != 0) {
            SDL_RenderDrawLine(renderer, pos, 0, pos, GRID_SIZE);
            SDL_RenderDrawLine(renderer, 0, pos, GRID_SIZE, pos);
        }
    }

    for (int i = 0; i <= N; i += 3) {
        int pos = i * CELL_SIZE;
        drawThickLine(renderer, pos, 0, pos, GRID_SIZE, 4);
        drawThickLine(renderer, 0, pos, GRID_SIZE, pos, 4);
    }

    SDL_Rect highlightRect = {currentCol * CELL_SIZE, currentRow * CELL_SIZE, CELL_SIZE, CELL_SIZE};
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 128);
    SDL_RenderFillRect(renderer, &highlightRect);

    SDL_Color textColor = {0, 0, 255, 255};
    SDL_Surface* textSurface = nullptr;
    SDL_Texture* textTexture = nullptr;
    SDL_Rect textRect;

    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            if (grid[row][col] != 0) {
                string numStr = to_string(grid[row][col]);

                textSurface = TTF_RenderText_Solid(font, numStr.c_str(), textColor);
                textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

                textRect.w = textSurface->w;
                textRect.h = textSurface->h;

                textRect.x = col * CELL_SIZE + (CELL_SIZE - textRect.w) / 2;
                textRect.y = row * CELL_SIZE + (CELL_SIZE - textRect.h) / 2;

                SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

                SDL_FreeSurface(textSurface);
                SDL_DestroyTexture(textTexture);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

bool isSudokuSolved(int grid[N][N]) {
    bool rowCheck[N][N + 1] = { false };
    bool colCheck[N][N + 1] = { false };
    bool boxCheck[N][N + 1] = { false };

    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            int num = grid[row][col];
            if (num == 0) {
                return false;
            }

            int boxIndex = (row / 3) * 3 + col / 3;
            if (rowCheck[row][num] || colCheck[col][num] || boxCheck[boxIndex][num]) {
                return false;
            }

            rowCheck[row][num] = colCheck[col][num] = boxCheck[boxIndex][num] = true;
        }
    }

    return true;
}

bool isSafe(int grid[N][N], int row, int col, int num) {
    for (int x = 0; x < N; x++) {
        if (grid[row][x] == num || grid[x][col] == num) {
            return false;
        }
    }

    int startRow = row - row % 3, startCol = col - col % 3;
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            if (grid[x + startRow][y + startCol] == num) {
                return false;
            }
        }
    }

    return true;
}

bool solveSudoku(int grid[N][N]) {
    int row, col;

    bool isEmpty = false;
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++) {
            if (grid[row][col] == 0) {
                isEmpty = true;
                break;
            }
        }
        if (isEmpty) {
            break;
        }
    }

    if (!isEmpty) {
        return true;
    }

    for (int num = 1; num <= 9; num++) {
        if (isSafe(grid, row, col, num)) {
            grid[row][col] = num;
            if (solveSudoku(grid)) {
                return true;
            }
            grid[row][col] = 0;
        }
    }

    return false;
}

void generateSudoku(int grid[N][N], int numEmptyCells) {
    srand(time(0));
    solveSudoku(grid);

    for (int i = 0; i < numEmptyCells; i++) {
        int row = rand() % N;
        int col = rand() % N;
        while (grid[row][col] == 0) {
            row = rand() % N;
            col = rand() % N;
        }
        int temp = grid[row][col];
        grid[row][col] = 0;

        int tempGrid[N][N];
        for (int x = 0; x < N; x++) {
            for (int y = 0; y < N; y++) {
                tempGrid[x][y] = grid[x][y];
            }
        }

        if (!solveSudoku(tempGrid)) {
            grid[row][col] = temp;
            i--;
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Sudoku", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_SIZE, WINDOW_SIZE, SDL_WINDOW_SHOWN);
    if (!window) {
        cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() == -1) {
        cout << "TTF_Init: " << TTF_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        cout << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << endl;
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int grid[N][N] = {{0}};
    int numEmptyCells = 40;
    generateSudoku(grid, numEmptyCells);

    int currentRow = 0, currentCol = 0;
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_w:
                    case SDLK_UP:
                        if (currentRow > 0)
                            currentRow--;
                        break;
                    case SDLK_s:
                    case SDLK_DOWN:
                        if (currentRow < N - 1)
                            currentRow++;
                        break;
                    case SDLK_a:
                    case SDLK_LEFT:
                        if (currentCol > 0)
                            currentCol--;
                        break;
                    case SDLK_d:
                    case SDLK_RIGHT:
                        if (currentCol < N - 1)
                            currentCol++;
                        break;
                    case SDLK_q:
                        quit = true;
                        break;
                    case SDLK_BACKSPACE:
                        grid[currentRow][currentCol] = 0;
                        break;
                    default:
                        break;
                }

                if (e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym <= SDLK_9) {
                    int num = e.key.keysym.sym - SDLK_1 + 1;
                    if (grid[currentRow][currentCol] == 0 || grid[currentRow][currentCol] != num) {
                        grid[currentRow][currentCol] = num;
                    } else {
                        grid[currentRow][currentCol] = 0;
                    }
                }

                if (isSudokuSolved(grid)) {
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Congratulations!", "You solved the Sudoku! Press Q to quit the game.", window);
                }
            }
        }

        printSudoku(renderer, font, grid, currentRow, currentCol);
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
