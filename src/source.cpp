#include <iostream>
#include <vector>
#include <set>
#include <iterator>

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

    auto sumRC = [&](const std::vector<unsigned int>& vec) {
        unsigned int sum = 0;
        for (auto& u : vec) sum += u;
        return sum;
    };

    auto sumVector = [](const std::vector<unsigned int>& vec) {
        unsigned int sum = 0;
        for (auto& v : vec) sum += v;
        return sum;
    };

    // Check if the sum of each row is equal to the expected sum.
    auto isSolved = [&]() {
        for (int column = 0; column < width; column++) { // for each row
            unsigned int sum = sumRC(columns[column]);
            unsigned int total = 0;
            for (int row = 0; row < height; row++) // for each column
                total += (picross[row][column] == 1 ? 1 : 0);
            if (sum - total) return false;
        }
        return true;
    };

    auto firstSweep = [&](unsigned char crs) {
        // crs = 0: columns, crs = 1: rows
        const unsigned int wh = (crs) ? height : width; // wh: width or height
        const std::vector<unsigned int>* crv = (crs) ? &rows[0] : &columns[0]; // crv: column or row vector
        for (int cr = 0; cr < wh; cr++) { // cr: column or row
            const unsigned int sum = sumRC(crv[cr]) + (crv[cr].size() - 1);
            if (sum == wh) {
                unsigned int it = 0;
                for (auto& n : crv[cr]) {
                    for (int i = 0; i < n; i++) {
                        if (crs)
                            picross[cr][i + it] = 1;
                        else
                            picross[i + it][cr] = 1;
                    }
                    it += n + 1;
                }
            }
            else {
                int largestBlank = wh - sum;
                for (int i = 0; i < crv[cr].size(); i++) {
                    if (crv[cr][i] > largestBlank) {
                        unsigned int it = 0;
                        for (int j = 0; j < i; j++)
                            it += crv[cr][j] + 1;
                        for (int j = largestBlank; j < crv[cr][i]; j++) {
                            if (crs)
                                picross[cr][j + it] = 1;
                            else
                                picross[j + it][cr] = 1;
                        }
                    }
                }
            }
        }
    };

    auto checkCompletedRC = [&](unsigned char crs) {
        const unsigned int wh = (crs) ? height : width;
        const std::vector<unsigned int>* crv = (crs) ? &rows[0] : &columns[0];
        std::set<unsigned int>& s = (crs) ? unFilledRows : unFilledColumns;
        for (std::set<unsigned int>::iterator cr = s.begin(); cr != s.end(); cr++) {
            unsigned int expectedSum = sumVector(crv[*cr]);
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

    auto treadRC = [&](unsigned int crs, std::vector<unsigned int> vec, unsigned int cr, unsigned int length) {
        unsigned int* address = (crs ? &picross[cr][0] : &picross[0][cr]); // first
        unsigned int* address2 = (crs ? &picross[cr][length - 1] : &picross[length - 1][cr]); // last
        if (*address == 1) {
            for (int i = 1; i < vec[0] - 1; i++) {
                address = (crs ? &picross[cr][i] : &picross[i][cr]);
                *address = 1;
            }
        }
        else if (*address2 == 1) {
            for (int i = length - 1; i > length - 1 - vec.back(); i--) {
                address = (crs ? &picross[cr][i] : &picross[i][cr]);
                *address = 1;
            }
            vec.pop_back();
        }
        for (int i = length - 1; i >= 0; i--) {
            unsigned int* address3 = (crs ? &picross[cr][i] : &picross[i][cr]);
            while (*address3 != 1 && i) {
                i--;
                address3 = (crs ? &picross[cr][i] : &picross[i][cr]);
            }
            unsigned int* address4 = (crs ? &picross[cr][i + 1] : &picross[i + 1][cr]);
            unsigned int* address5 = (crs ? &picross[cr][i - 1] : &picross[i - 1][cr]);
            if (i + 1 < length && *address4 == 2) {
                for (int j = 0; j < vec.back(); j++) {
                    address3 = (crs ? &picross[cr][i] : &picross[i][cr]);
                    *address3 = 1;
                    i--;
                }
                vec.pop_back();
            }
            else if (i + 1 < length && (i - 1) >= 0 && *address5 == 2 && *address4 == 0) {
                int returnVal = i;
                for (int j = 0; j < vec.back(); j++) {
                    address3 = (crs ? &picross[cr][i] : &picross[i][cr]);
                    *address3 = 1;
                    i++;
                }
                vec.pop_back();
                i = returnVal;
            }
        }
    };

    firstSweep(0);
    firstSweep(1);
    checkCompletedRC(0);
    checkCompletedRC(1);
    while (!isSolved()) {
        for (auto i = unFilledColumns.begin(); i != unFilledColumns.end(); i++)
            treadRC(0, columns[*i], *i, width);
        for (auto i = unFilledRows.begin(); i != unFilledRows.end(); i++)
            treadRC(1, rows[*i], *i, width);
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

    for (int i = 0; i < 5; i++)
        delete[] picross[i];
    delete[] picross;

    return 0;
}
