#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#define MAX_LINES 100000
#define MAX_LEN 1024

char *lines[MAX_LINES];
int line_count = 0;

char search[MAX_LEN] = "";
int search_len = 0;

char prompt_text[256] = ">";

void print_help(const char *prog_name) {
    printf("Usage: <command> | %s [OPTIONS] [INITIAL_SEARCH]\n\n", prog_name);
    printf("Options:\n");
    printf("  -h, --help            Print this help message and exit\n");
    printf("  --word                After selecting a line, open a second menu to choose a specific word\n");
    printf("  -d <delimiter>        Set a custom delimiter for word splitting (default: space)\n");
    printf("  --print <template>    Format output using a template, replacing \"{}\" with the selected item\n");
    printf("  --prompt <text>       Set custom prompt text (default: \">\")\n");
    printf("  --output <file>       Print the selected item to a file instead of stdout\n\n");
    printf("Arguments:\n");
    printf("  [INITIAL_SEARCH]      Any remaining arguments are used as the initial search string\n");
}

int select_item(char **items, int count) {
    int *filtered_indices = malloc(sizeof(int) * (count > 0 ? count : 1));
    int filtered_count = 0;
    int selected = 0;
    int start_idx = 0;
    int done = 0;
    int result_idx = -1;

    while (!done) {
        filtered_count = 0;
        for (int i = 0; i < count; i++) {
            if (search_len == 0 || strstr(items[i], search) != NULL) {
                filtered_indices[filtered_count++] = i;
            }
        }
        
        if (selected >= filtered_count) {
            selected = filtered_count > 0 ? filtered_count - 1 : 0;
        }
        if (selected < 0) selected = 0;

        clear();
        mvprintw(0, 0, "%s %s", prompt_text, search);

        int max_display = LINES - 1;
        if (max_display < 1) max_display = 1;

        if (selected >= start_idx + max_display) {
            start_idx = selected - max_display + 1;
        } else if (selected < start_idx) {
            start_idx = selected;
        }

        for (int i = 0; i < max_display && (i + start_idx) < filtered_count; i++) {
            int idx = i + start_idx;
            if (idx == selected) attron(A_REVERSE);
            mvprintw(i + 1, 0, "%.*s", COLS, items[filtered_indices[idx]]);
            if (idx == selected) attroff(A_REVERSE);
        }
        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (selected > 0) selected--;
                break;
            case KEY_DOWN:
                if (selected < filtered_count - 1) selected++;
                break;
            case 21: // Ctrl-U
            case KEY_PPAGE:
                selected -= max_display;
                if (selected < 0) selected = 0;
                break;
            case 4:  // Ctrl-D
            case KEY_NPAGE:
                selected += max_display;
                if (selected >= filtered_count) {
                    selected = filtered_count > 0 ? filtered_count - 1 : 0;
                }
                break;
            case '\n':
            case '\r':
            case KEY_ENTER:
                if (filtered_count > 0) {
                    result_idx = filtered_indices[selected];
                }
                done = 1;
                break;
            case KEY_BACKSPACE:
            case 127:
            case '\b':
                if (search_len > 0) {
                    search[--search_len] = '\0';
                }
                break;
            case 27: 
                done = 1;
                break;
            default:
                if (ch >= 32 && ch <= 126 && search_len < MAX_LEN - 1) {
                    search[search_len++] = ch;
                    search[search_len] = '\0';
                }
                break;
        }
    }
    
    free(filtered_indices);
    return result_idx;
}

int main(int argc, char *argv[]) {
    int word_mode = 0;
    char delim_str[16] = " ";
    char *print_template = NULL;
    char *output_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--word") == 0) {
            word_mode = 1;
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            strncpy(delim_str, argv[++i], sizeof(delim_str) - 1);
            delim_str[sizeof(delim_str) - 1] = '\0';
        } else if (strcmp(argv[i], "--print") == 0 && i + 1 < argc) {
            print_template = argv[++i];
        } else if (strcmp(argv[i], "--prompt") == 0 && i + 1 < argc) {
            strncpy(prompt_text, argv[++i], sizeof(prompt_text) - 1);
            prompt_text[sizeof(prompt_text) - 1] = '\0';
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else {
            if (search_len > 0 && search_len < MAX_LEN - 1) {
                search[search_len++] = ' ';
                search[search_len] = '\0';
            }
            int arg_len = strlen(argv[i]);
            if (search_len + arg_len < MAX_LEN - 1) {
                strcat(search, argv[i]);
                search_len += arg_len;
            }
        }
    }

    char buffer[MAX_LEN];
    while (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        lines[line_count] = strdup(buffer);
        line_count++;
        if (line_count >= MAX_LINES) break;
    }

    FILE *tty = fopen("/dev/tty", "r+");
    if (!tty) return 1;

    SCREEN *term = newterm(NULL, tty, tty);
    if (!term) return 1;
    set_term(term);
    
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int selected_line_idx = select_item(lines, line_count);
    char *final_result = NULL;
    int ret_code = 0;
    
    if (selected_line_idx != -1) {
        int line_num = selected_line_idx + 1;
        
        if (word_mode) {
            char *line_copy = strdup(lines[selected_line_idx]);
            char **words = malloc(sizeof(char*) * MAX_LEN);
            int word_count = 0;
            
            char *token = strtok(line_copy, delim_str);
            while (token != NULL && word_count < MAX_LEN) {
                words[word_count++] = token;
                token = strtok(NULL, delim_str);
            }
            
            if (word_count > 0) {
                int selected_word_idx = select_item(words, word_count);
                if (selected_word_idx != -1) {
                    final_result = strdup(words[selected_word_idx]);
                    ret_code = (line_num < 254) ? line_num : 255;
                }
            }
            free(words);
            free(line_copy);
        } else {
            final_result = strdup(lines[selected_line_idx]);
            ret_code = (line_num < 254) ? line_num : 255;
        }
    }

    endwin();
    delscreen(term);
    fclose(tty);

    if (final_result) {
        FILE *out = stdout;
        if (output_file) {
            out = fopen(output_file, "w");
            if (!out) {
                fprintf(stderr, "Error: Could not open output file '%s'\n", output_file);
                out = stdout;
            }
        }

        if (print_template) {
            char *p = print_template;
            char *match;
            while ((match = strstr(p, "{}")) != NULL) {
                fprintf(out, "%.*s", (int)(match - p), p);
                fprintf(out, "%s", final_result);
                p = match + 2;
            }
            fprintf(out, "%s\n", p);
        } else {
            fprintf(out, "%s\n", final_result);
        }

        if (out != stdout) {
            fclose(out);
        }
        
        free(final_result);
    }

    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }

    return ret_code;
}
