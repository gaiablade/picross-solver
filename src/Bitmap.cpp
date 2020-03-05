#include "Bitmap.h"

bool bm::Array2dToBMP(uint32_t** array, const uint32_t& width, const uint32_t& height, const char* filename) {
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
            if (array[i][j] == 1) image.write("\0\0\0", 3);
            else image.write("\xff\xff\xff", 3);
        }
        for (int k = 0; k < paddingPerRow; k++) {
            image.write("\0", 1);
        }
    }
}

bool bm::AsciiArt(const std::string& filename) {
    // Open input file in binary mode:
    std::ifstream data(filename, std::ios_base::binary);
    data >> std::noskipws; // Don't interpret whitespace
    if (data.fail()) {
        std::cout << "Could not open " << filename << "." << std::endl;
        return false;
    }

    int numBytesIgnored = 0;
    auto throwAwayBytes = [&](int numBytes) {
        byte temp;
        for (int i = 0; i < numBytes; i++) data >> temp;
        numBytesIgnored += numBytes;
    };
    int numBytesRead = 0;

    // Header: Size is 14 bytes
    throwAwayBytes(2); // Ignore Signature
    unsigned int sizeInBytes; data.read((char*)&sizeInBytes, 4); numBytesRead += 4; // 0x02 -> 0x05
    throwAwayBytes(4); // Ignore reserved space
    unsigned int startingAddress; data.read((char*)&startingAddress, 4); numBytesRead += 4; // 0x0A -> 0x0D

    // Info Header
    unsigned int sizeOfHeader; data.read((char*)&sizeOfHeader, 4); numBytesRead += 4; // This represents the size of the info header! 0x0E -> 0x11
    unsigned int width; data.read((char*)&width, 4); numBytesRead += 4; // 0x12 -> 0x15
    unsigned int height; data.read((char*)&height, 4); numBytesRead += 4; // 0x16 -> 0x19
    throwAwayBytes(2);
    unsigned short bpp; data.read((char*)&bpp, 2); numBytesRead += 2; // 0x1C -> 0x1D 30 bytes total
    unsigned short bytesPerPixel = bpp / 8;
    throwAwayBytes(4);
    unsigned int imageSize; data.read((char*)&imageSize, 4); numBytesRead += 4; // 0x22 -> 0x25 38 bytes total
    throwAwayBytes(startingAddress - numBytesIgnored - numBytesRead);

    auto calculatePadding = [&]() {
        if (!((width * bytesPerPixel) % 4)) return 0;
        int padding = 1;
        while (((width * bytesPerPixel) + padding) % 4) padding++;
        return padding;
    };

    int padding = calculatePadding();
    size_t arrSize = imageSize * sizeof(byte) - (height * padding);
    byte* pixelData = new byte[arrSize];
    for (unsigned int i = 0; i < arrSize; i++) {
        pixelData[i] = 69;
    }

    byte temp;
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width * bytesPerPixel; j++) {
            data.read((char*)&temp, 1);
            pixelData[i * (width * bytesPerPixel) + j] = temp;
        }
        for (int j = 0; j < padding; j++) data.read((char*)&temp, 1);
    }

    data.close();

    unsigned int numPixels = (imageSize - (padding * height)) / bytesPerPixel;
    bm::Pixel* pixels = new bm::Pixel[numPixels];

    for (unsigned int i = 0; i < numPixels; i++) {
        byte* currentPixelData = new byte[bytesPerPixel];
        for (unsigned int j = 0; j < bytesPerPixel; j++) {
            currentPixelData[j] = pixelData[(i * bytesPerPixel) + j];
        }
        pixels[i] = bm::Pixel(bytesPerPixel);
        pixels[i].setRGBA(currentPixelData);
    }

    std::string str = " ^\",:;Il!i~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    const float divider = (255.0f / (float)str.length());

    for (int i = height - 1; i >= 0; i--) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned int arrIndex = pixels[i * width + j].getAverage() / divider;
            arrIndex = (arrIndex == str.length() ? arrIndex - 1 : arrIndex);
            std::cout << str[arrIndex];
        }
        std::cout << std::endl;
    }

    delete[] pixels;
    delete[] pixelData;
    return true;
}
