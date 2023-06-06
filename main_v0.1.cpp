#include<iostream>
#include<fstream>
#include<ios>
#include<experimental/filesystem> // lstdc++fs flag
#include<string>
#include<vector>
#include<iomanip>
#include<algorithm>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

const int NUM_THREADS = 8; // programda kullanılacak thread sayısı

std::vector<std::vector<float>> inputs; // hem thread fonksiyonu hem de
pthread_mutex_t lock; // global mutex

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

    // Store the input vector at the correct index
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

    inputs.resize(filenames.size()); // Resize the inputs vector to the correct size

    for (int i = 0; i < filenames.size(); i++) {
        std::string* argArray = new std::string[2];
        argArray[0] = std::to_string(i); // Pass the file index as well
        argArray[1] = filenames[i];
        rc = pthread_create(&(threads[i % NUM_THREADS]), NULL, readFileThread, (void*)argArray);
        if (rc) {
            std::cerr << "Error creating thread: " << rc << std::endl;
            exit(-1);
        }
    }

    // Join all threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}


/*
void* readFileThread(void* arg) { // her bir thread'in ayrı kullanabileceği readFile fonksiyonu
    pid_t tid = syscall(SYS_gettid);
    //std::cout<<tid<<std::endl;
    std::string filename = *(std::string*) arg;
    std::ifstream fin(filename);
    fin >> std::fixed>>std::setprecision(4);
    std::vector<float> input;
    float temp;
    while(fin >> temp) {
        input.push_back(temp);
    }
    fin.close();
    pthread_mutex_lock(&lock);
    inputs.push_back(input);
    pthread_mutex_unlock(&lock);
    delete (std::string*) arg;
    pthread_exit(NULL);
}

void readFile(std::string dir) {
    std::vector <std::string> filenames;
    for (const auto &entry: std::experimental::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".txt") {
            filenames.push_back(entry.path().string());
        }
    }
    std::sort(filenames.begin(), filenames.end(), [](const std::string &a, const std::string &b) {
        int num_a = std::stoi(a.substr(a.find_last_of("_") + 1, a.find_last_of(".") - a.find_last_of("_") - 1));
        int num_b = std::stoi(b.substr(b.find_last_of("_") + 1, b.find_last_of(".") - b.find_last_of("_") - 1));
        return num_a < num_b;
    });
    pthread_t threads[NUM_THREADS];
    int rc;
    int threadCount = 0; // Keep track of the number of threads created

    for (int i = 0; i < filenames.size(); i++) {
        std::string *arg = new std::string(filenames[i]);
        rc = pthread_create(&(threads[threadCount]), NULL, readFileThread, (void *) arg);
        if (rc) {
            std::cerr << "Error creating thread: " << rc << std::endl;
            exit(-1);
        }
        threadCount++;

        if (threadCount == NUM_THREADS || i == filenames.size() - 1) {
            // Join all created threads
            for (int j = 0; j < threadCount; j++) {
                pthread_join(threads[j], NULL);
            }
            threadCount = 0; // Reset thread count
        }
    }
}
*/


/*
std::vector<std::vector<float>> readFile(std::string dir) {
    std::vector <std::string> filenames;
    for (const auto &entry: std::experimental::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".txt") {
            filenames.push_back(entry.path().string());
        }
    }
    std::sort(filenames.begin(), filenames.end(), [](const std::string &a, const std::string &b) {
        int num_a = std::stoi(a.substr(a.find_last_of("_") + 1, a.find_last_of(".") - a.find_last_of("_") - 1));
        int num_b = std::stoi(b.substr(b.find_last_of("_") + 1, b.find_last_of(".") - b.find_last_of("_") - 1));
        return num_a < num_b;
    });
    pthread_t threads[NUM_THREADS];
    int rc;

    for (int i = 0; i < filenames.size(); i++) {
        std::string *arg = new std::string(filenames[i]); // dosya listesini thread fonksiyonuna göndermek için
        rc = pthread_create(&(threads[i % NUM_THREADS]), NULL, readFileThread, (void *) arg);
        if (rc) {
            std::cerr << "Error creating thread: " << rc << std::endl;
            exit(-1);
        }
        if (i % NUM_THREADS == 0 && i > 0) {
            std::cout << "I1: " << i << std::endl;
            for (int i = 0; i < NUM_THREADS; i++) {
                std::cout << "I: " << i << std::endl;
                pthread_join(threads[i], NULL);
            }
        } else if ((filenames.size() - 1) % NUM_THREADS != 0 && (filenames.size() - 1) - i < NUM_THREADS &&
                   i % NUM_THREADS == 7) {
            std::cout << "I2: " << i << std::endl;
            for (int x = 0; x < NUM_THREADS; x++) {
                std::cout << "X: " << x << std::endl;
                pthread_join(threads[x], NULL);
            }
        }
    }
    return inputs;
}
*/


void Encryption(std::vector<std::vector<float>> inputs, float key){
    std::experimental::filesystem::create_directories("./encryption/");
    for(int i = 0; i<inputs.size();i++){
        std::string filename ="./encryption/encrypted_" + std::to_string(i) + ".txt";
        std::ofstream fout(filename);
        fout << std::fixed<<std::setprecision(4);
        for(int j = 0; j<inputs[i].size(); j++){
            fout<<inputs[i][j]*key<<"\n";
        }
        fout.close();
    }
}


int main(int argc, char* argv[]){
    float key = 0.5f;
    std::string decision = argv[1];
    std::string dir = argv[2];
    if(decision == "e"){
        //std::vector<std::vector<float>> inputs = readFile(dir);
        readFile(dir);
        Encryption(inputs, key) ;
    }
    else if(decision == "d"){
        std::cout<<"Not available!"<<std::endl;
    }
    else{
        std::cout<<"ERROR"<<std::endl<<"Encryption: e"<<std::endl<<"Decryption: d"<<std::endl;
    }
    return 0;
}