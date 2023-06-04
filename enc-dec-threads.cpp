#include<iostream>
#include<fstream>
#include<ios>
#include<experimental/filesystem> // lstdc++fs flag
#include<string>
#include<vector>
#include<iomanip>
#include<algorithm>

const int NUM_THREADS = 8; // programda kullanılacak thread sayısı

std::vector<std::vector<float>> inputs; // hem thread fonksiyonu hem de
pthread_mutex_t lock; // global mutex

void* readFileThread(void* arg) { // her bir thread'in ayrı kullanabileceği readFile fonksiyonu
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
    return NULL;
}

std::vector<std::vector<float>> readFile(std::string dir){
    std::vector<std::string> filenames;
    for(const auto& entry: std::experimental::filesystem::directory_iterator(dir)){
        if(entry.path().extension() == ".txt"){
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
    for(int i = 0; i < filenames.size(); i++){
        std::string* arg = new std::string(filenames[i]); // dosya listesini thread fonksiyonuna göndermek için
        rc = pthread_create(&threads[i % NUM_THREADS], NULL, readFileThread, (void*) arg);
        if(rc) {
            std::cerr << "Error creating thread: " << rc << std::endl;
            exit(-1);
        }
    }
    for(int i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
    return inputs;
}


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
        std::vector<std::vector<float>> inputs = readFile(dir);
        Encryption(inputs, key) ;
    }
    else if(*argv[1] == 'd'){
        std::cout<<"Not available!"<<std::endl;
    }
    else{
        std::cout<<"ERROR"<<std::endl<<"Encryption: e"<<std::endl<<"Decryption: d"<<std::endl;
    }
    return 0;
}