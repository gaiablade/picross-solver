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

typedef unsigned char byte;

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
            if (sum == wh) {
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

bool picrossToBmp(const unsigned int width, const unsigned int height, unsigned int** picross, const char* filename) {
    std::ofstream image(filename, std::ofstream::binary);
    if (image.fail()) return false;

    // Generate header:
    int address = 0x7a, sizeOfHeader = 0x6c, paddingPerRow = 4 - ((width * 3) % 4);
    int imageSize = 3 * (width * height) + (height * paddingPerRow), resolution = 0xb13;
    int fileSize = address + imageSize;
    short colorPlanes = 1, bitsPerPixel = 24;
    image.write("BM", 2 * sizeof(byte)); // header field 0x00
    image.write((const char*)&fileSize, 4 * sizeof(byte)); // PLACEHOLDER: total size of file in bytes
    image.write("\0\0\0\0", 4 * sizeof(byte)); // Reserved
    image.write((const char*)&address, 4 * sizeof(byte)); // Address of pixel data
    image.write((const char*)&sizeOfHeader, 4 * sizeof(byte)); // Size of header in bytes 0x0E
    image.write((const char*)&width, 4 * sizeof(byte)); // 0x12
    image.write((const char*)&height, 4 * sizeof(byte)); // 0x16
    image.write((const char*)&colorPlanes, 2 * sizeof(byte)); // Number of color planes (must be 1) 0x1A
    image.write((const char*)&bitsPerPixel, 2 * sizeof(byte)); // 0x1C
    image.write("\0\0\0\0", 4); // compression method none 0x1E
    image.write((const char*)&imageSize, 4); // image size: 0x22
    image.write((const char*)&resolution, 4); // horizontal resolution 0x26
    image.write((const char*)&resolution, 4); // vertical resolution 0x2A
    image.write("\0\0\0\0", 4); // num colors in pallete 0x2E
    image.write("\0\0\0\0", 4); // num of important colors 0x32
    for (int i = 0; i < 68; i++) image.write("\0", 1 * sizeof(byte));

    // Pixel array: black if 1, white otherwise
    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < width; j++) {
            if (picross[i][j] == 1) image.write("\0\0\0", 3);
            else image.write("\xff\xff\xff", 3);
        }
        for (int k = 0; k < paddingPerRow; k++) {
            image.write("\0", 1);
        }
    }
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
    picrossToBmp(5, 5, &picross[0], "output1.bmp");

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
    picrossToBmp(5, 5, &picross[0], "output2.bmp");

    for (int i = 0; i < 5; i++)
        delete[] picross[i];
    delete[] picross;

    return 0;
}
