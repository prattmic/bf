#include <stdio.h>
#include <stdint.h>

#define MEM_SIZE    100000

uint8_t data[MEM_SIZE];
uint8_t text[MEM_SIZE+1];

struct state {
    uint8_t *pc;
    uint8_t *ptr;
} state = {
    .pc = &text[0],
    .ptr = &data[0],
};

enum commands {
    INC_PTR,
    DEC_PTR,
    INC_MEM,
    DEC_MEM,
    OUTPUT,
    INPUT,
    START_LOOP,
    END_LOOP,
    NUM_COMMANDS,
    END,
};

static int inc_pc(uint8_t **pc) {
    do {
        if (*(++(*pc)) == END) {
            return 1;
        }
    } while (**pc >= NUM_COMMANDS);

    return 0;
}

static int dec_pc(uint8_t **pc) {
    do {
        if (*(--(*pc)) == END) {
            return 1;
        }
    } while (**pc >= NUM_COMMANDS);

    return 0;
}

static void inc_pointer(void) {
    state.ptr++;
}

static void dec_pointer(void) {
    state.ptr--;
}

static void inc_mem(void) {
    (*state.ptr)++;
}

static void dec_mem(void) {
    (*state.ptr)--;
}

static void output(void) {
    putchar(*state.ptr);
}

static void input(void) {
    *state.ptr = getchar();
}

static void jump_forward(void) {
    if (!*state.ptr) {
        int depth = 0;
        uint8_t *i = state.pc;

        inc_pc(&i);

        while (*i != END_LOOP || depth) {
            if (*i == START_LOOP) {
                depth++;
            } else if (*i == END_LOOP) {
                depth--;
            }

            inc_pc(&i);
        }

        /* PC will be incremented to command after this in main() */
        state.pc = i;
    }
}

static void jump_back(void) {
    if (*state.ptr) {
        int depth = 0;
        uint8_t *i = state.pc;

        dec_pc(&i);

        while (*i != START_LOOP || depth) {
            if (*i == END_LOOP) {
                depth++;
            } else if (*i == START_LOOP) {
                depth--;
            }

            dec_pc(&i);
        }

        /* PC will be incremented to command after this in main() */
        state.pc = i;
    }
}

void (*commands[8])(void) = {
    [INC_PTR]       = inc_pointer,
    [DEC_PTR]       = dec_pointer,
    [INC_MEM]       = inc_mem,
    [DEC_MEM]       = dec_mem,
    [OUTPUT]        = output,
    [INPUT]         = input,
    [START_LOOP]    = jump_forward,
    [END_LOOP]      = jump_back,
};

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s file\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL) {
        printf("Unable to open file\n");
        return 1;
    }

    size_t len = fread(text, sizeof(uint8_t), MEM_SIZE, fp);
    if (len == 0) {
        fputs("Error reading file", stderr);
    } else {
        text[len+1] = END; /* Just to be safe. */
    }

    fclose(fp);

    /* Replace all of the characters */
    for (int i = 0; i < len; i++) {
        uint8_t replacement;

        if (text[i] == END) {
            break;
        }

        switch (text[i]) {
        case '>':
            replacement = INC_PTR;
            break;
        case '<':
            replacement = DEC_PTR;
            break;
        case '+':
            replacement = INC_MEM;
            break;
        case '-':
            replacement = DEC_MEM;
            break;
        case '.':
            replacement = OUTPUT;
            break;
        case ',':
            replacement = INPUT;
            break;
        case '[':
            replacement = START_LOOP;
            break;
        case ']':
            replacement = END_LOOP;
            break;
        default:
            replacement = text[i];
        }

        text[i] = replacement;
    }

    /* Run! */
    while (!inc_pc(&state.pc)) {
        commands[*state.pc]();
    }

}
