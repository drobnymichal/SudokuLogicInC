#include "sudoku.h"
#include <ctype.h>
#include <stdlib.h>

const unsigned int NINE_ONES = 0x1ff;
const unsigned int EMPTY_CELL = 0x00;
const char ERROR[] = "ERROR has occurred!\n";

bool contain(unsigned int original, int number);
void copy_array(unsigned int copy_from[81], unsigned int copy_to[81]);
static unsigned int bitset_add(unsigned int original, int number);
static bool bitset_is_unique(unsigned int original);
static int bitset_next(unsigned int bitset, int previous);
unsigned int make_bitset(unsigned int sudoku[9][9], int row_start, int row_end, int col_start, int col_end);

/* ************************************************************** *
 *               Functions required by assignment                 *
 * ************************************************************** */

/**
 * @brief           function eliminates the row of the sudoku.    
 *
 * @param sudoku      sudoku in 2D format
 * @param row_index   index of the checked row
 * 
 * @return          elimination has made changes -> true
 *                  otherwise -> false
 */
bool eliminate_row(unsigned int sudoku[9][9], int row_index)
{
    bool is_change = false;
    unsigned int mask = make_bitset(sudoku, row_index, row_index + 1, 0, 9);
    for (int j = 0; j < 9; j++) {
        if (!bitset_is_unique(sudoku[row_index][j])) {
            unsigned int original = sudoku[row_index][j];
            sudoku[row_index][j] &= mask;
            is_change = (original != sudoku[row_index][j]) || is_change;
        }
    }
    return is_change;
}

/**
 * @brief           function eliminates the col of the sudoku.    
 *
 * @param sudoku      sudoku in 2D format
 * @param col_index   index of the checked col
 * 
 * @return          elimination has made changes -> true
 *                  otherwise -> false
 */
bool eliminate_col(unsigned int sudoku[9][9], int col_index)
{
    bool is_change = false;
    unsigned int mask = make_bitset(sudoku, 0, 9, col_index, col_index + 1);
    for (int i = 0; i < 9; i++) {
        if (!bitset_is_unique(sudoku[i][col_index])) {
            unsigned int original = sudoku[i][col_index];
            sudoku[i][col_index] &= mask;
            is_change = (original != sudoku[i][col_index]) || is_change;
        }
    }
    return is_change;
}

/**
 * @brief           function eliminates the box of the sudoku with specific area.    
 *
 * @param sudoku      sudoku in 2D format
 * @param row_index   index of row - top left corner of the box
 * @param col_index   index of col - top left corner of the box
 * 
 * @return          elimination has made changes -> true
 *                  otherwise -> false
 */
bool eliminate_box(unsigned int sudoku[9][9], int row_index, int col_index)
{
    bool is_change = false;
    unsigned int mask = make_bitset(sudoku, row_index, row_index + 3, col_index, col_index + 3);
    for (int i = row_index; i < row_index + 3; i++) {
        for (int j = col_index; j < col_index + 3; j++) {
            if (!bitset_is_unique(sudoku[i][j])) {
                unsigned int original = sudoku[i][j];
                sudoku[i][j] &= mask;
                is_change = (original != sudoku[i][j]) || is_change;
            }
        }
    }
    return is_change;
}

/**
 * @brief           function checks if the sudoku still needs to solve.    
 *
 * @param sudoku    sudoku in 2D format
 * 
 * @return          sudoku is not solved -> true
 *                  sudoku is solved -> false
 */
bool needs_solving(unsigned int sudoku[9][9])
{
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (!bitset_is_unique(sudoku[i][j])) {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief           function checks if the row is valid.    
 *
 * @param sudoku    sudoku in 2D format
 * @param row       index of the checked row
 * 
 * @return          row is valid -> true
 *                  otherwise -> false
 */
bool is_valid_row(unsigned int sudoku[9][9], int row)
{
    unsigned int mask = 0;
    for (int col = 0; col < 9; col++) {
        if (sudoku[row][col] == 0) {
            return false;
        }
        if (bitset_is_unique(sudoku[row][col])) {
            if ((sudoku[row][col] | mask) == mask) {
                return false;
            }
            mask = sudoku[row][col] | mask;
        }
    }
    return true;
}

/**
 * @brief           function checks if the col is valid.    
 *
 * @param sudoku    sudoku in 2D format
 * @param col       index of the checked col
 * 
 * @return          col is valid -> true
 *                  otherwise -> false
 */
bool is_valid_col(unsigned int sudoku[9][9], int col)
{
    unsigned int mask = 0;
    for (int row = 0; row < 9; row++) {
        if (sudoku[row][col] == 0) {
            return false;
        }
        if (bitset_is_unique(sudoku[row][col])) {
            if ((sudoku[row][col] | mask) == mask) {
                return false;
            }
            mask = sudoku[row][col] | mask;
        }
    }
    return true;
}

/**
 * @brief           function checks if the box 3x3 with the spefic area is valid.    
 *
 * @param sudoku    sudoku in 2D format
 * @param row       index of row - top left corner of the box
 * @param col       index of col - top left corner of the box
 * 
 * @return          box is valid -> true
 *                  otherwise -> false
 */
bool is_valid_box(unsigned int sudoku[9][9], int row, int col)
{
    unsigned int mask = 0;
    for (int i = row; i < (row + 3); i++) {
        for (int j = col; j < (col + 3); j++) {
            if (sudoku[i][j] == 0) {
                return false;
            }
            if (bitset_is_unique(sudoku[i][j])) {
                if ((sudoku[i][j] | mask) == mask) {
                    return false;
                }
                mask = sudoku[i][j] | mask;
            }
        }
    }
    return true;
}

/**
 * @brief           function checks whether it is a valid sudoku.    
 *
 * @param sudoku    sudoku in 2D format 
 * 
 * @return          sudoku is valid -> true
 *                  otherwise -> false
 */
bool is_valid(unsigned int sudoku[9][9])
{
    for (int i = 0; i < 9; i++) {
        if (!is_valid_row(sudoku, i)) {
            return false;
        }
        if (!is_valid_col(sudoku, i)) {
            return false;
        }
        if (!is_valid_box(sudoku, ((i / 3) * 3), ((i % 3) * 3))) {
            return false;
        }
    }
    return true;
}

/**
 * @brief           The function tries to solve the sudoku using elimination.   
 *
 * @param sudoku    sudoku in 2D format 
 * 
 * @return          has been successfully solved -> true
 *                  otherwise -> false
 */
bool solve(unsigned int sudoku[9][9])
{
    if (!is_valid(sudoku)) {
        fprintf(stderr, ERROR);
        return false;
    }
    while (needs_solving(sudoku)) {
        bool is_change = false;
        for (int row = 0; row < 9; row++) {
            is_change = eliminate_row(sudoku, row) || is_change;
        }
        for (int col = 0; col < 9; col++) {
            is_change = eliminate_col(sudoku, col) || is_change;
        }
        for (int row = 0; row < 9; row += 3) {
            for (int col = 0; col < 9; col += 3) {
                is_change = eliminate_box(sudoku, row, col) || is_change;
            }
        }
        if (is_valid(sudoku) == false) {
            fprintf(stderr, ERROR);
            return false;
        }
        if (!is_change) {
            return false;
        }
    }
    return true;
}

/**
 * @brief           The function tries to load row of the sudoku in 
 *                  ASCII format. Function is controlling row with "+-".    
 *
 * @param col_index  the index from which the row is to be checked.
 * 
 * @return          has been successfully loaded 26 chars -> true
 *                  otherwise -> false
 */
bool check_plus_minus(int col_index)
{
    const char pref[] = "+-------+-------+-------+\n";
    int chr;
    while (col_index < 26 && (chr = getchar()) != EOF) {
        if (pref[col_index] != chr) {
            return false;
        }
        col_index++;
    }
    return col_index == 26;
}

/**
 * @brief           The function tries to load row of the sudoku in 
 *                  ASCII format. Function is controlling row with numbers.
 * 
 * @param row       the index of the row being checked
 * @param sudoku    sudoku in 2D format 
 * 
 * @return          has been successfully loaded 26 chars -> true
 *                  otherwise -> false
 */
bool check_normal_row(int row, unsigned int sudoku[9][9])
{
    int cell_pointer = 0, col = 0, chr;
    while (cell_pointer < 25 && (chr = getchar()) != EOF) {
        if (cell_pointer % 8 == 0) {
            if (chr != '|') {
                return false;
            }
        } else if (cell_pointer % 2 == 0) {
            sudoku[row][col] = 0;
            if (chr == '.' || chr == '0') {
                sudoku[row][col] = NINE_ONES;
            } else if (chr == '!') {
                sudoku[row][col] = EMPTY_CELL;
            } else if ('1' <= chr && chr <= '9') {
                sudoku[row][col] = bitset_add(sudoku[row][col], chr - '0');
            } else {
                return false;
            }
            col++;
        } else if (chr != ' ') {
            return false;
        }
        cell_pointer++;
    }
    return (cell_pointer == 25) && (getchar() == '\n');
}

/**
 * @brief           The function tries to load the sudoku in ASCII format
 *                  from standard input.   
 *
 * @param sudoku    sudoku in 2D format 
 * 
 * @return          has been successfully loaded 13 rows -> true
 *                  otherwise -> false
 */
bool load_ascii_format(unsigned int sudoku[9][9])
{
    int num_rows_loaded = 0, rows_loaded = 1;
    while (rows_loaded < 13) {
        if (rows_loaded % 4 == 0) {
            if (!check_plus_minus(0)) {
                return false;
            }
        } else {
            if (!check_normal_row(num_rows_loaded, sudoku)) {
                return false;
            }
            num_rows_loaded++;
        }
        rows_loaded++;
    }
    return rows_loaded == 13;
}

/**
 * @brief           The function tries to load the sudoku in numeric format
 *                  from standard input.   
 *
 * @param sudoku    sudoku in 2D format 
 * 
 * @return          has been successfully loaded -> true
 *                  otherwise -> false
 */
bool load_numeric_format(unsigned int sudoku[81])
{
    int cell_pointer = 1, chr;
    while (cell_pointer < 81 && (chr = getchar()) != EOF) {
        if (!isdigit(chr)) {
            return false;
        }
        sudoku[cell_pointer] = 0;
        sudoku[cell_pointer] = (chr != '0') ? bitset_add(sudoku[cell_pointer], chr - '0') : NINE_ONES;
        cell_pointer++;
    }
    return cell_pointer == 81 && (((chr = getchar()) == EOF) || chr == '\n');
}

/**
 * @brief           The function decides which of the two formats will 
 *                  be loaded and calls it.  
 *
 * @param sudoku    sudoku in 2D format 
 * 
 * @return          has been successfully loaded -> true
 *                  otherwise -> false
 */
bool load(unsigned int sudoku[9][9])
{
    int chr = getchar();
    if (isdigit(chr)) {
        int num = chr - '0';
        sudoku[0][0] = 0;
        sudoku[0][0] = (num != 0) ? bitset_add(sudoku[0][0], num) : NINE_ONES;
        if (load_numeric_format((unsigned int *) sudoku)) {
            return true;
        }
    }
    if (chr == '+') {
        if (check_plus_minus(1) && load_ascii_format(sudoku)) {
            return true;
        }
    }
    fprintf(stderr, ERROR);
    return false;
}

/**
 * @brief           Function print the sudoku to standard output.  
 *
 * @param sudoku    sudoku in 2D format 
 * 
 * @return          None
 */
void print(unsigned int sudoku[9][9])
{
    char delim[] = "+-------+-------+-------+\n";
    for (int i = 0; i < 9; i++) {
        if (i % 3 == 0) {
            printf("%s", delim);
        }
        for (int j = 0; j < 9; j++) {
            if (j % 3 == 0) {
                printf("| ");
            }
            if (sudoku[i][j] == 0) {
                printf("! ");
            } else if (bitset_is_unique(sudoku[i][j])) {
                printf("%d ", bitset_next(sudoku[i][j], 0));
            } else {
                printf(". ");
            }
        }
        printf("|\n");
    }
    printf("%s", delim);
}

/* ************************************************************** *
 *                              Bonus                             *
 * ************************************************************** */

/**
 * @brief           Return count of cells which can be deleted and
 *                  sudoku will be still solvable with <solve()>. Indexes
 *                  of these cells are storing in the <mask>.  
 *
 * @param sudoku     original sudoku in 1D format
 * @param sud_copy   auxiliary array in 1D format
 * @param mask       auxiliary array in 1D format for storing indexes  
 * 
 * @return          count of cells which can be deleted and
 *                  sudoku will be still solvable with <solve()>.
 */
int is_solvable(unsigned int sudoku[81], unsigned int sud_copy[81], unsigned int mask[81]) //return count of deletable cells.
{
    int index = 0;
    for (int i = 0; i < 81; i++) {
        mask[i] = 0;
        if (bitset_is_unique(sudoku[i])) {
            copy_array(sudoku, sud_copy);
            sud_copy[i] = 0x1ff;
            if (solve((unsigned int(*)[9]) sud_copy)) {
                mask[index] = i;
                index++;
            }
        }
    }
    return index;
}

/**
 * @brief           Return pseudo-random number from 0 to (limit - 1)  
 *
 * @param limit     limit of searching random numbers
 * 
 * @return          pseudo-random number from 0 to (limit - 1)
 */
int shake(int limit)
{
    return rand() % limit;
}

bool is_empty(unsigned int sudoku[81]) {
    for (int i = 0; i < 81; i++) {
        if (sudoku[i] != 0x1ff) {
            return false;
        } 
    }
    return true;
}
/**
 * @brief           The function removes unique elements if it loses
 *                  its solution after removing any other sudoku cells.  
 *
 * @param sudoku    sudoku (array 9x9)
 * 
 * @return          None -> function modify array
 */
void generate(unsigned int sudoku[9][9])
{
    unsigned int *sud = (unsigned int *) sudoku;
    unsigned int sud_copy[81];
    unsigned int mask[81] = { 0 };
    int i;
    int count = is_solvable(sud, sud_copy, mask);

    while (count > 0) {
        i = mask[shake(count)];
        sud[i] = 0x1ff;
        count = is_solvable(sud, sud_copy, mask);
    }
}

/**
 * @brief Tries to solve the sudoku using backtracking and elimination 
 *
 * @param sudoku    sudoku (array 9x9)
 * 
 * @return          solution found -> true
 *                  otherwise -> false
 */
bool generic_solve(unsigned int sudoku[9][9])
{
    if (!is_valid(sudoku)) {
        return false;
    }
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            if (!bitset_is_unique(sudoku[row][col])) {
                unsigned int orig_sud[9][9];
                copy_array((unsigned int *) sudoku, (unsigned int *) orig_sud);
                if (solve(sudoku)) {
                    return true;
                }
                if (bitset_is_unique(sudoku[row][col])) {
                    return generic_solve(sudoku);
                }
                for (int num = 1; num < 10; num++) {
                    if (contain(orig_sud[row][col], num)) {
                        sudoku[row][col] = 0;
                        sudoku[row][col] = bitset_add(sudoku[row][col], num);
                        if (generic_solve(sudoku)) {
                            return true;
                        }
                        copy_array((unsigned int *) orig_sud, (unsigned int *) sudoku);
                    }
                }
                return false;
            }
        }
    }
    return true;
}

/* ************************************************************** *
 *                      Auxiliary functionns                      *
 * ************************************************************** */

/**
 * @brief            Copy array to another array.
 *
 * @param copy_from  array which will be copy to array <copy_to> represented as 1D array.
 * @param copy_to    number which should be in bitset.
 * 
 * @return           None -> function modify array.
 */
void copy_array(unsigned int copy_from[81], unsigned int copy_to[81])
{
    for (int i = 0; i < 81; i++) {
        copy_to[i] = copy_from[i];
    }
}

/**
 * @brief Check if cell contains value
 *
 * @param original  contents of the 2D sudoku cell.
 * @param number    number which should be in bitset
 * 
 * @return          bitset contains number -> true
 *                  otherwise -> false
 */
bool contain(unsigned int original, int number)
{
    unsigned int mask = original & (1 << (number - 1));
    return mask > 0;
}

/**
 * @brief Add number into bit set
 *
 * @param original  contents of the 2D sudoku cell.
 * @param number    to be added to the set
 * 
 * @return          new value of the cell with the number included
 */
static unsigned int bitset_add(unsigned int original, int number)
{
    return original | (1 << (number - 1));
}

/**
 * @brief  Check whether given cell has a unique value assigned.
 *
 * @param original  bitset to check for single vs. multiple values.
 * 
 * @return          true if set, false otherwise
 */
static bool bitset_is_unique(unsigned int original)
{
    int count = 0;
    unsigned int mask = 1;
    for (int num = 1; num < 10; num++) {
        if ((original & mask) != 0) {
            count++;
        }
        mask <<= 1;
    }
    return count == 1;
}

/**
 * @brief Return next number present in bit set.
 *
 * @param original  contents of the 2D sudoku cell.
 * @param previous  last known number present, 0 for start
 * 
 * @return          * next (higher) number than argument if such
 *                   such is present.
 *                  * -1 otherwise
 *
 * @note The value previous might not be in the bitset
 */
static int bitset_next(unsigned int bitset, int previous)
{
    unsigned int mask = 1;
    for (int i = previous; i < 10; i++) {
        if ((bitset & (mask << i)) > 0)
            return i + 1;
    }
    return -1;
}

/**
 * @brief Return bitset of values which can be stored in cells in specified area.
 * 
 * @param sudoku    pointer to 2D array[9][9] (sudoku).
 * @param row_start row-index of start
 * @param row_end   row-index of end
 * @param col_start col-index of start
 * @param col_end   col-index of end
 *
 * @return          bitset of specified area.
 */
unsigned int make_bitset(unsigned int sudoku[9][9], int row_start, int row_end, int col_start, int col_end)
{
    unsigned int mask = NINE_ONES;
    for (int i = row_start; i < row_end; i++) {
        for (int j = col_start; j < col_end; j++) {
            if (bitset_is_unique(sudoku[i][j])) {
                mask = (mask ^ sudoku[i][j]) & NINE_ONES;
            }
        }
    }
    return mask;
}