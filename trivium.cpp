#include <iostream>
#include <vector>
#include <bitset>
#include <random>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <chrono>

std::string text_to_bits(const std::string& text) {
    std::stringstream bitStream;
    for (char c : text) {
        bitStream << std::bitset<8>(c);
    }
    return bitStream.str();
}

std::string text_from_bits(const std::string& bits) {
    std::stringstream textStream;
    for (size_t i = 0; i < bits.size(); i += 8) {
        textStream << static_cast<char>(std::bitset<8>(bits.substr(i, 8)).to_ulong());
    }
    return textStream.str();
}

char LTTXOR(char a, char b) {
    return (a == b) ? '0' : '1';
}

char LTTAND(char a, char b) {
    return (a == '1' && b == '1') ? '1' : '0';
}

void initShifts(std::vector<char>& shiftA, std::vector<char>& shiftB, std::vector<char>& shiftC, const std::vector<int>& key, const std::vector<int>& IV) {
    for (int i = 0; i < 80; ++i) {
        shiftA[i] = key[i] + '0';
        shiftB[i] = IV[i] + '0';
    }
    shiftC[110] = shiftC[109] = shiftC[108] = '1';
    for (int i = 0; i < 4 * 288; ++i) {
        char t1 = LTTXOR(shiftB[77], LTTXOR(LTTAND(shiftA[90], shiftA[91]), LTTXOR(shiftA[65], shiftA[92])));
        char t2 = LTTXOR(shiftC[86], LTTXOR(LTTAND(shiftB[81], shiftB[82]), LTTXOR(shiftB[68], shiftB[83])));
        char t3 = LTTXOR(shiftA[68], LTTXOR(LTTAND(shiftC[108], shiftC[109]), LTTXOR(shiftC[65], shiftC[110])));
        shiftA.insert(shiftA.begin(), t3);
        shiftB.insert(shiftB.begin(), t1);
        shiftC.insert(shiftC.begin(), t2);
        shiftA.pop_back();
        shiftB.pop_back();
        shiftC.pop_back();
    }
}

std::vector<char> keyGeneration(std::vector<char>& shiftA, std::vector<char>& shiftB, std::vector<char>& shiftC, const std::vector<int>& key, const std::vector<int>& IV, int size) {
    std::vector<char> z;
    for (int i = 0; i < size; ++i) {
        char t1 = LTTXOR(shiftA[65], shiftA[92]);
        char t2 = LTTXOR(shiftB[68], shiftB[83]);
        char t3 = LTTXOR(shiftC[65], shiftC[110]);
        z.push_back(LTTXOR(t1, LTTXOR(t2, t3)));
        t1 = LTTXOR(LTTXOR(t1, shiftB[77]), LTTAND(shiftA[90], shiftA[91]));
        t2 = LTTXOR(LTTXOR(t2, shiftC[86]), LTTAND(shiftB[81], shiftB[82]));
        t3 = LTTXOR(LTTXOR(t3, shiftA[68]), LTTAND(shiftC[108], shiftC[109]));
        shiftA.insert(shiftA.begin(), t3);
        shiftB.insert(shiftB.begin(), t1);
        shiftC.insert(shiftC.begin(), t2);
        shiftA.pop_back();
        shiftB.pop_back();
        shiftC.pop_back();
    }
    return z;
}

std::string encryption(const std::string& plainText, const std::vector<int>& key, const std::vector<int>& IV) {
    std::string binaryText = text_to_bits(plainText);

    // shift registers
    std::vector<char> shiftA(93, '0');
    std::vector<char> shiftB(84, '0');
    std::vector<char> shiftC(111, '0');

    // setting key and IV
    initShifts(shiftA, shiftB, shiftC, key, IV);

    // key stream generation
    std::vector<char> keyStream = keyGeneration(shiftA, shiftB, shiftC, key, IV, binaryText.size());

    std::string encryptedText;
    for (size_t i = 0; i < binaryText.size(); ++i) {
        encryptedText.push_back(LTTXOR(keyStream[i], binaryText[i]));
    }
    return encryptedText;
}

std::string decryption(const std::string& encryptedText, const std::vector<int>& key, const std::vector<int>& IV) {
    // shift registers
    std::vector<char> shiftA(93, '0');
    std::vector<char> shiftB(84, '0');
    std::vector<char> shiftC(111, '0');

    // setting key and IV
    initShifts(shiftA, shiftB, shiftC, key, IV);

    // key stream generation
    std::vector<char> keyStream = keyGeneration(shiftA, shiftB, shiftC, key, IV, encryptedText.size());

    std::string decryptedText;
    for (size_t i = 0; i < encryptedText.size(); ++i) {
        decryptedText.push_back(LTTXOR(keyStream[i], encryptedText[i]));
    }
    return text_from_bits(decryptedText);
}

// Thêm vào hàm main
int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    std::vector<int> IV(80);
    std::vector<int> key(80);
    for (int i = 0; i < 80; ++i) {
        IV[i] = dis(gen);
        key[i] = dis(gen);
    }

    std::string fileName;
    std::cout << "Enter the name of the input file: ";
    std::cin >> fileName;

    // Bắt đầu đo thời gian
    auto start = std::chrono::high_resolution_clock::now();

   // Tạo tên file đầu ra bằng cách thay thế dấu chấm đầu tiên trong tên file bằng dấu gạch dưới
    std::string outputFileName = fileName;
    size_t lastdot = outputFileName.find_last_of(".");
    if (lastdot != std::string::npos) {
        outputFileName[lastdot] = '_'; // Thay thế dấu chấm bằng dấu gạch dưới
    }

    std::string encryptedFileName = "./encodeData/encode_" + outputFileName  + ".bin";
    std::string inputFileName = "./testData/" + fileName;

    std::ifstream inputFile(inputFileName, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Could not open the file - '" << fileName << "'" << std::endl;
        return EXIT_FAILURE;
    }

    // Đọc nội dung file vào một vector<char>
    std::vector<char> inputContents((std::istreambuf_iterator<char>(inputFile)),
                                    std::istreambuf_iterator<char>());
    inputFile.close();

    // Chuyển đổi nội dung file thành chuỗi bit
    std::string binaryText;
    for (char c : inputContents) {
        binaryText += std::bitset<8>(c).to_string();
    }

    // Mã hóa nội dung file
    std::string encryptedBinaryText = encryption(binaryText, key, IV);

    // Chuyển đổi chuỗi bit mã hóa thành vector<char> để ghi file
    std::vector<char> encryptedContents;
    for (size_t i = 0; i < encryptedBinaryText.size(); i += 8) {
        std::bitset<8> bits(encryptedBinaryText.substr(i, 8));
        encryptedContents.push_back(static_cast<char>(bits.to_ulong()));
    }

    
    std::ofstream outputFile(encryptedFileName, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Could not open the file - '" << encryptedFileName << "'" << std::endl;
        return EXIT_FAILURE;
    }

    // Ghi nội dung đã mã hóa vào file mới
    outputFile.write(&encryptedContents[0], encryptedContents.size());
    outputFile.close();
    // Kết thúc đo thời gian
    auto finish = std::chrono::high_resolution_clock::now();

    std::cout << "Encrypted data has been written to " << encryptedFileName << std::endl;
    // Tính toán và in ra thời gian mất để mã hóa
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";

    return 0;
}