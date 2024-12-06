#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <termios.h>

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

struct termios orig_termios;

void reset_terminal_mode() {
    tcsetattr(0, TCSANOW, &orig_termios);
    printf("\033[2J\033[%d;0H\033[?25h\033[0m\n", BOARD_HEIGHT + 3);
}

void set_conio_terminal_mode() {
    struct termios new_termios;

    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));
    atexit(reset_terminal_mode);

    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit() {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch() {
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
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
    printf("\033[H");
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (board[i][j]) {
                printf("[]");
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentBlock.shape[i][j]) {
                printf("\033[%d;%dH[]", currentBlock.y + i + 1, (currentBlock.x + j) * 2 + 1);
            }
        }
    }
    fflush(stdout);
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
    set_conio_terminal_mode();
    while (1) {
        drawBoard();
        usleep(250000);
        if (kbhit()) {
            char c = getch();
            switch (c) {
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
            }
        } else {
            if (canMove(currentBlock, 0, 1)) {
                moveBlock(0, 1);
            } else {
                placeBlock();
                clearLines();
                initBlock();
                if (!canMove(currentBlock, 0, 0)) {
                    printf("\033[H\033[J");
                    printf("Game Over\n");
                    break;
                }
            }
        }
    }
    reset_terminal_mode();
    printf("게임이 종료되었습니다. 터미널이 정상으로 복구되었습니다.\n");
}

int main() {
    set_conio_terminal_mode();
    initBoard();
    initBlock();
    gameLoop();
    return 0;
}
