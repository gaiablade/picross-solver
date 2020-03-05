/* Picross Solver
 * Solves picross/nonogram puzzles based on the numbers signifying blocks in
 * rows and columns.
 * Author: Caleb Geyer (http://www.github.com/gaiablade/)
 *   Date: February 8th, 2020
 */
#include <iostream>
#include <vector>
#include <set>
#include <iterator>
#include <fstream>
#include "Bitmap.h"

void solvePicross(unsigned int width, unsigned int height, unsigned int** picross, std::vector<unsigned int> rows[5], std::vector<unsigned int> columns[5]) {

    std::set<unsigned int> unFilledRows;
    std::set<unsigned int> unFilledColumns;
    for (int i = 0; i < height; i++) unFilledRows.insert(i);
    for (int i = 0; i < width; i++) unFilledColumns.insert(i);

    auto printPicross = [&]() {
        for (int column = 0; column < width; column++) {
            for (int row = 0; row < height; row++)
                std::cout << picross[column][row] << " ";
            std::cout << std::endl;
        }
        std::cout << std::endl;
    };

    auto printVector = [](const std::vector<unsigned int>& vec) {
        for (auto& v : vec) {
            std::cout << v << " ";
        }
        std::cout << std::endl;
        std::cin.get();
    };

    /*
     * Returns the sum of the vector plus 1 for each gap.
     */
    auto sumRC = [&](const std::vector<unsigned int>& vec) {
        unsigned int sum = 0;
        for (auto& u : vec) sum += u;
        sum += vec.size() - 1;
        return sum;
    };

    /*
     * Returns the sum of the vector.
     */
    auto sumVector = [](const std::vector<unsigned int>& vec) {
        unsigned int sum = 0;
        for (auto& v : vec) sum += v;
        return sum;
    };

    // Check if the sum of each row is equal to the expected sum.
    auto isSolved = [&]() {
        if (unFilledColumns.empty() && unFilledRows.empty()) {
            return true;
        }
        return false;
    };

    auto firstSweep = [&](unsigned char crs) {
        // crs = 0: columns, crs = 1: rows
        const unsigned int wh = (crs) ? height : width; // wh: width or height

        // crv is a handle to the vector or row or column values.
        const std::vector<unsigned int>* crv = (crs) ? &rows[0] : &columns[0]; // crv: column or row vector

        for (int cr = 0; cr < wh; cr++) { // cr: column or row
            //printVector(crv[cr]);
            const unsigned int sum = sumRC(crv[cr]);
            // If the sum of the row/column (e.g. {2, 2} = 2 + 1(gap) + 2 = 5)
            // equals the width/height, there is only one solution to that
            // row/column:
            if (sum == 0) {
                for (int i = 0; i < wh; i++) {
                    if (crs) {
                        picross[cr][i] = 2;
                    }
                    else {
                        picross[i][cr] = 2;
                    }
                }
            }
            else if (sum == wh) {
                std::vector<uint8_t> guarenteedLayout;
                for (int i = 0; i < crv[cr].size(); i++) {
                    for (int j = 0; j < crv[cr][i]; j++) {
                        guarenteedLayout.push_back(1);
                    }
                    guarenteedLayout.push_back(0);
                }
                for (int i = 0; i < wh; i++) {
                    if (crs) {
                        picross[cr][i] = guarenteedLayout[i];
                    }
                    else {
                        picross[i][cr] = guarenteedLayout[i];
                    }
                }
            }
            // If a block is larger than the largest possible gap, fill in the
            // tiles that are guarenteed:
            else {
                int largestGap = wh - sum;
                if (crv[cr].size() > 1) {
                    largestGap++;
                }
                std::vector<uint8_t> guarenteedLayout;
                for (int i = 0; i < crv[cr].size(); i++) {
                    if (crv[cr][i] >= largestGap) {
                        if (crv[cr][i] == largestGap) {
                            for (int j = 0; j < crv[cr][i] + 1 - largestGap; j++) {
                                guarenteedLayout.push_back(0);
                            }
                            guarenteedLayout.push_back(1);
                        }
                        else {
                            for (int j = 0; j < largestGap; j++) {
                                guarenteedLayout.push_back(0);
                            }
                            for (int j = 0; j < crv[cr][i] - largestGap; j++) {
                                guarenteedLayout.push_back(1);
                            }
                        }
                    }
                    else {
                        for (int j = 0; j < crv[cr][i]; j++) {
                            guarenteedLayout.push_back(0);
                        }
                        guarenteedLayout.push_back(0);
                    }
                }
                for (int i = 0; i < wh; i++) {
                    if (crs) {
                        if (!picross[cr][i]) {
                            picross[cr][i] = guarenteedLayout[i];
                        }
                    }
                    else {
                        if (!picross[i][cr]) {
                            picross[i][cr] = guarenteedLayout[i];
                        }
                    }
                }
            }
        }
    };

    /*
     * Check if an entire row/column is complete and fill the gaps.
     */
    auto checkCompletedRC = [&](unsigned char crs) {
        const unsigned int wh = crs ? height : width;
        const std::vector<unsigned int>* rc = crs ? &rows[0] : &columns[0];
        std::set<unsigned int>& s = crs ? unFilledRows : unFilledColumns;
        for (int i = 0; i < wh; i++) { // for each row/column
            bool complete = true;
            for (int j = 0; j < wh; j++) { // for each tile in row/column
                if (rc[i][j] == 0) {
                    complete = false;
                    break;
                }
            }
            if (complete) {
                s.erase(i);
            }
        }
        for (std::set<unsigned int>::iterator cr = s.begin(); cr != s.end(); cr++) {
            unsigned int expectedSum = sumVector(rc[*cr]);
            unsigned int actualSum = 0;
            for (int i = 0; i < wh; i++) actualSum += (crs) ? (picross[*cr][i] == 1 ? 1 : 0) : (picross[i][*cr] == 1 ? 1 : 0);
            if (actualSum == expectedSum) {
                // set unfilled squares to 0 (X)
                if (crs) { // rows
                    for (int i = 0; i < wh; i++)
                        if (!picross[*cr][i]) picross[*cr][i] = 2;
                }
                else {
                    for (int i = 0; i < wh; i++)
                        if (!picross[i][*cr]) picross[i][*cr] = 2;
                }
                s.erase(cr);
                if (!s.empty()) cr = s.begin();
            }
        }
    };

    /*
     * Check each row for possible gaps.
     */
    auto checkGaps = [&]() {
        for (int i = 0; i < width; i++) {
        }
    };

    auto treadRC = [&](unsigned int crs, std::vector<unsigned int> vec, unsigned int cr, unsigned int length) {
        //printVector(vec);
        unsigned int* firstTile  = crs ? &picross[cr][0]          : &picross[0][cr]; // first
        unsigned int* lastTile   = crs ? &picross[cr][length - 1] : &picross[length - 1][cr]; // last
        unsigned int* secondTile = crs ? &picross[cr][1]          : &picross[1][cr];
        unsigned int* thirdTile  = crs ? &picross[cr][2]          : &picross[2][cr];
        unsigned int* ntlTile    = crs ? &picross[cr][length - 2] : &picross[length - 2][cr]; // next-to-last tile
        unsigned int* nttlTile   = crs ? &picross[cr][length - 3] : &picross[length - 3][cr];
        /*
         * If first tile in row/column is filled:
         */
        if (*firstTile == 1) {
            for (int i = 1; i < vec[0] - 1; i++) {
                unsigned int* iterator = crs ? &picross[cr][i] : &picross[i][cr];
                *iterator = 1;
            }
            vec.erase(vec.begin());
        }
        /*
         * If the last tile in row/column is filled:
         */
        if (*lastTile == 1) {
            for (int i = length - 1; i > length - 1 - vec.back(); i--) {
                unsigned int* iterator = crs ? &picross[cr][i] : &picross[i][cr];
                *iterator = 1;
            }
            vec.pop_back();
        }
        /*
         * If first tile is crossed and the second tile is filled:
         */
        if (*firstTile == 2 && *secondTile == 1) {
            int i = 0;
            for (i = 0; i < vec[0]; i++) {
                unsigned int* iterator = crs ? &picross[cr][i + 1] : &picross[i + 1][cr];
                *iterator = 1;
            }
            if (i + 1 < length) {
                unsigned int* iterator = crs ? &picross[cr][i + 1] : &picross[i + 1][cr];
                *iterator = 2;
            }
            vec.erase(vec.begin());
        }
        /*
         * If last tile is crossed and the next to last tile is filled:
         */
        if (*lastTile == 2 && *ntlTile == 1) {
            for (int i = 0; i < vec.back(); i++) {
                //ntlTile[-i] = 1;
                unsigned int* iterator = crs ? &picross[cr][length - 2 - i] : &picross[length - 2 - i][cr];
                *iterator = 1;
            }
            vec.pop_back();
        }
        if (*firstTile == 0 && *secondTile == 1 && *thirdTile == 2) {
            for (int i = 0; i < vec[0]; i++) {
                unsigned int* iterator = crs ? &picross[cr][1 + i] : &picross[i + 1][cr];
                *iterator = 1;
            }
            *firstTile = *firstTile == 1 ? 1 : 2;
            vec.erase(vec.begin());
        }
        if (*lastTile == 0 && *ntlTile == 1 && *nttlTile == 2) {
            for (int i = 0; i < vec.back(); i++) {
                unsigned int* iterator = crs ? &picross[cr][length - 2 + i] : &picross[length - 2 + i][cr];
                *iterator = 1;
            }
            *lastTile = *lastTile == 1 ? 1 : 2;
            vec.pop_back();
        }

        /*
         * Count how many remaining empty tiles and resolve them:
         */
        unsigned int remainingEmpty = 0;
        for (int i = 0; i < length; i++) {
            unsigned int* tile = crs ? &picross[cr][i] : &picross[i][cr];
            if (*tile == 0) {
                remainingEmpty++;
            }
        }
        if (remainingEmpty == sumRC(vec) && vec.size() == 1) {
            int number = 0;
            unsigned int* iterator = crs ? &picross[cr][number] : &picross[number][cr];
            while (*iterator) {
                number++;
                iterator = crs ? &picross[cr][number] : &picross[number][cr];
            }
            for (int i = 0; i < vec[0]; i++) {
                *iterator = 1;
                number++;
                iterator = crs ? &picross[cr][number] : &picross[number][cr];
            }
        }

        int iterator = 0;
        while (iterator < length && *(crs ? &picross[cr][iterator] : &picross[iterator][cr]) != 1) {
            iterator++;
        }
        int iterator2 = iterator;
        while (iterator2 < length && *(crs ? &picross[cr][iterator2] : &picross[iterator2][cr]) == 1) {
            iterator2++;
        }
        if (iterator > 0 && iterator < length && iterator2 < length && *(crs ? &picross[cr][iterator2] : &picross[iterator2][cr]) == 0) {
            if ((crs ? picross[cr][iterator - 1] : picross[iterator - 1][cr]) == 2) {
                int i;
                for (i = 0; i < vec[0]; i++) {
                    if (crs) {
                        picross[cr][iterator + i] = 1;
                    }
                    else {
                        picross[iterator + i][cr] = 1;
                    }
                }
                (crs ? picross[cr][iterator + i] : picross[iterator + i][cr]) = 2;
            }
        }
    };

    firstSweep(0); // columns
    firstSweep(1); // rows
    firstSweep(0); // columns
    checkCompletedRC(0);
    checkCompletedRC(1);
    while (!isSolved()) {
        for (auto i = unFilledColumns.begin(); i != unFilledColumns.end(); i++) {
            treadRC(0, columns[*i], *i, width);
        }
        for (auto i = unFilledRows.begin(); i != unFilledRows.end(); i++) {
            treadRC(1, rows[*i], *i, width);
        }
        checkCompletedRC(0);
        checkCompletedRC(1);
        checkCompletedRC(0);
        checkCompletedRC(1);
    }
    checkCompletedRC(0);
    checkCompletedRC(1);
    printPicross();
}

int main() {
    // picross[columns][rows]
    unsigned int** picross = new unsigned int*[5];
    for (int i = 0; i < 5; i++)
        picross[i] = new unsigned int[5];

    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            picross[i][j] = 0;

    std::vector<unsigned int> rows[5];
    std::vector<unsigned int> columns[5];

    rows[0] = {2, 2};
    rows[1] = {1};
    rows[2] = {1, 2};
    rows[3] = {1};
    rows[4] = {4};

    columns[0] = {1, 2};
    columns[1] = {1, 1, 1};
    columns[2] = {1};
    columns[3] = {1, 1, 1};
    columns[4] = {3};

    solvePicross(5, 5, &picross[0], rows, columns);
    bm::Array2dToBMP(&picross[0], 5, 5, "output1.bmp");

    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            picross[i][j] = 0;

    rows[0] = {1};
    rows[1] = {5};
    rows[2] = {2, 1};
    rows[3] = {2, 1};
    rows[4] = {1, 1};

    columns[0] = {4};
    columns[1] = {3};
    columns[2] = {1};
    columns[3] = {1, 1};
    columns[4] = {4};

    solvePicross(5, 5, &picross[0], rows, columns);
    bm::Array2dToBMP(&picross[0], 5, 5, "output2.bmp");

    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            picross[i][j] = 0;

    rows[0] = {2};
    rows[1] = {2, 1};
    rows[2] = {1};
    rows[3] = {1, 1};
    rows[4] = {1, 1};

    columns[0] = {0};
    columns[1] = {2, 2};
    columns[2] = {2};
    columns[3] = {1, 1};
    columns[4] = {1, 1};

    solvePicross(5, 5, &picross[0], rows, columns);
    bm::Array2dToBMP(&picross[0], 5, 5, "output3.bmp");

    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            picross[i][j] = 0;

    rows[0] = {1, 3};
    rows[1] = {1, 1};
    rows[2] = {2, 1};
    rows[3] = {2, 1};
    rows[4] = {3, 1};

    columns[0] = {3, 1};
    columns[1] = {3};
    columns[2] = {1, 2};
    columns[3] = {1};
    columns[4] = {5};

    solvePicross(5, 5, &picross[0], rows, columns);
    bm::Array2dToBMP(&picross[0], 5, 5, "output4.bmp");

    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            picross[i][j] = 0;

    rows[0] = {2, 2};
    rows[1] = {1, 2};
    rows[2] = {1, 1};
    rows[3] = {4};
    rows[4] = {2, 1};

    columns[0] = {5};
    columns[1] = {1, 2};
    columns[2] = {1, 1};
    columns[3] = {5};
    columns[4] = {1};

    solvePicross(5, 5, &picross[0], rows, columns);
    //picrossToBmp(5, 5, &picross[0], "output5.bmp");
    bm::Array2dToBMP(&picross[0], 5, 5, "output5.bmp");

    for (int i = 0; i < 5; i++)
        delete[] picross[i];
    delete[] picross;

    return 0;
}
