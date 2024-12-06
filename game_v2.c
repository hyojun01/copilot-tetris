#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <termios.h>
#include <ncurses.h>

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

int board[BOARD_HEIGHT][BOARD_WIDTH];

typedef struct {
    int shape[4][4];
    int x, y;
} Block;

Block blocks[] = {
    {{{1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 0, 0},
    {{{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 0, 0},
    {{{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 0, 0},
    {{{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 0, 0},
    {{{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 0, 0},
    {{{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 0, 0},
    {{{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 0, 0}
};

Block currentBlock;

void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    timeout(250); // Non-blocking getch with 250ms delay
}

void end_ncurses() {
    endwin();
}

void initBoard() {
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            board[i][j] = 0;
        }
    }
}

void initBlock() {
    srand(time(NULL));
    currentBlock = blocks[rand() % 7];
    currentBlock.x = BOARD_WIDTH / 2 - 2;
    currentBlock.y = 0;
}

int canMove(Block block, int dx, int dy) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (block.shape[i][j]) {
                int newX = block.x + j + dx;
                int newY = block.y + i + dy;
                if (newX < 0 || newX >= BOARD_WIDTH || newY < 0 || newY >= BOARD_HEIGHT || board[newY][newX]) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

void moveBlock(int dx, int dy) {
    if (canMove(currentBlock, dx, dy)) {
        currentBlock.x += dx;
        currentBlock.y += dy;
    }
}

void rotateBlock() {
    Block rotated = currentBlock;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            rotated.shape[i][j] = currentBlock.shape[3 - j][i];
        }
    }
    if (canMove(rotated, 0, 0)) {
        currentBlock = rotated;
    }
}

void drawBoard() {
    clear();
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (board[i][j]) {
                mvprintw(i, j * 2, "[]");
            } else {
                mvprintw(i, j * 2, "  ");
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentBlock.shape[i][j]) {
                mvprintw(currentBlock.y + i, (currentBlock.x + j) * 2, "[]");
            }
        }
    }
    refresh();
}

void placeBlock() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentBlock.shape[i][j]) {
                board[currentBlock.y + i][currentBlock.x + j] = 1;
            }
        }
    }
}

void clearLines() {
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        int full = 1;
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (!board[i][j]) {
                full = 0;
                break;
            }
        }
        if (full) {
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < BOARD_WIDTH; j++) {
                    board[k][j] = board[k - 1][j];
                }
            }
            for (int j = 0; j < BOARD_WIDTH; j++) {
                board[0][j] = 0;
            }
        }
    }
}

void gameLoop() {
    init_ncurses();
    while (1) {
        drawBoard();
        int ch = getch();
        switch (ch) {
            case 'a':
                moveBlock(-1, 0);
                break;
            case 'd':
                moveBlock(1, 0);
                break;
            case 's':
                moveBlock(0, 1);
                break;
            case 'w':
                rotateBlock();
                break;
            case 'q':
                end_ncurses();
                return;
        }
        if (!canMove(currentBlock, 0, 1)) {
            placeBlock();
            clearLines();
            initBlock();
            if (!canMove(currentBlock, 0, 0)) {
                mvprintw(BOARD_HEIGHT / 2, (BOARD_WIDTH - 5) * 2, "Game Over");
                refresh();
                getch();
                break;
            }
        } else {
            moveBlock(0, 1);
        }
    }
    end_ncurses();
    printf("게임이 종료되었습니다. 터미널이 정상으로 복구되었습니다.\n");
}

int main() {
    initBoard();
    initBlock();
    gameLoop();
    return 0;
}