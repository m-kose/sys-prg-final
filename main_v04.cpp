#include <iostream>
#include <fstream>
#include <ios>
#include <experimental/filesystem>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <immintrin.h>

const unsigned long key = 0xABC400000000000;
const int NUM_THREADS = 8;
std::vector<std::vector<float>> inputs;
pthread_mutex_t lock;

void* readFileThread(void* arg) {
    pid_t tid = syscall(SYS_gettid);
    std::string* argArray = (std::string*)arg;
    int fileIndex = std::stoi(argArray[0]);
    std::string filename = argArray[1];
    std::ifstream fin(filename);
    fin >> std::fixed >> std::setprecision(4);
    std::vector<float> input;
    float temp;
    while (fin >> temp) {
        input.push_back(temp);
    }
    fin.close();

    pthread_mutex_lock(&lock);
    inputs[fileIndex] = input;
    pthread_mutex_unlock(&lock);
    delete[] argArray;
    pthread_exit(NULL);
}

void readFile(std::string dir) {
    std::vector<std::string> filenames;
    for (const auto& entry : std::experimental::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".txt") {
            filenames.push_back(entry.path().string());
        }
    }
    std::sort(filenames.begin(), filenames.end(), [](const std::string& a, const std::string& b) {
        int num_a = std::stoi(a.substr(a.find_last_of("_") + 1, a.find_last_of(".") - a.find_last_of("_") - 1));
        int num_b = std::stoi(b.substr(b.find_last_of("_") + 1, b.find_last_of(".") - b.find_last_of("_") - 1));
        return num_a < num_b;
    });
    pthread_t threads[NUM_THREADS];
    int rc;

    inputs.resize(filenames.size());

    for (int i = 0; i < filenames.size(); i++) {
        std::string* argArray = new std::string[2];
        argArray[0] = std::to_string(i);
        argArray[1] = filenames[i];
        rc = pthread_create(&(threads[i % NUM_THREADS]), NULL, readFileThread, (void*)argArray);
        if (rc) {
            std::cerr << "Error creating thread: " << rc << std::endl;
            exit(-1);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}

void* writeFileThread(void* arg) {
    pid_t tid = syscall(SYS_gettid);
    std::string* argArray = (std::string*)arg;
    int fileIndex = std::stoi(argArray[0]);
    std::string filename = argArray[1];
    std::ofstream fout(filename);
    fout << std::fixed << std::setprecision(4);
    for (int j = 0; j < inputs[fileIndex].size(); j++) {
        fout << inputs[fileIndex][j] << "\n";
    }
    fout.close();

    delete[] argArray;
    pthread_exit(NULL);
}

void writeFile(std::string dir) {
    std::experimental::filesystem::create_directories(dir);
    pthread_t threads[NUM_THREADS];
    int rc;

    for (int i = 0; i < inputs.size(); i++) {
        std::string* argArray = new std::string[2];
        argArray[0] = std::to_string(i);
        argArray[1] = dir + "/decrypted_" + std::to_string(i) + ".txt";
        rc = pthread_create(&(threads[i % NUM_THREADS]), NULL, writeFileThread, (void*)argArray);
        if (rc) {
            std::cerr << "Error creating thread: " << rc << std::endl;
            exit(-1);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}

void Encryption(std::vector<std::vector<float>>& inputs, const unsigned long key) {
    __m256 keyVec = _mm256_set1_ps(static_cast<float>(key));
    for (int i = 0; i < inputs.size(); i++) {
        for (int j = 0; j < inputs[i].size(); j += 8) {
            __m256 data = _mm256_loadu_ps(&inputs[i][j]);
            __m256 encrypted = _mm256_mul_ps(data, keyVec);
            _mm256_storeu_ps(&inputs[i][j], encrypted);
        }
    }
}

void Decryption(std::vector<std::vector<float>>& inputs, const unsigned long key) {
    __m256 keyVec = _mm256_set1_ps(static_cast<float>(key));
    for (int i = 0; i < inputs.size(); i++) {
        for (int j = 0; j < inputs[i].size(); j += 8) {
            __m256 data = _mm256_loadu_ps(&inputs[i][j]);
            __m256 decrypted = _mm256_div_ps(data, keyVec);
            _mm256_storeu_ps(&inputs[i][j], decrypted);
        }
    }
}

int main(int argc, char* argv[]) {
    std::string decision = argv[1];
    std::string dir = argv[2];

    if (decision == "e") {
        readFile(dir);
        Encryption(inputs, key);
        writeFile("./encryption/");
    } else if (decision == "d") {
        readFile(dir);
        Decryption(inputs, key);
        writeFile("./decryption/");
    } else {
        std::cout << "ERROR" << std::endl << "Encryption: e" << std::endl << "Decryption: d" << std::endl;
    }

    return 0;
}
