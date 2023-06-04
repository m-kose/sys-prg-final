#include<iostream>
#include<fstream>
#include<ios>
#include<experimental/filesystem> // lstdc++fs flag
#include<string>
#include<vector>
#include<iomanip>
#include<algorithm>

std::vector<std::vector<float>> readFile(std::string dir){
    std::vector<std::vector<float>> inputs;
    std::vector<std::string> filenames; // dosyaları(isimlerini) saklamak için vektör
    for(const auto& entry: std::experimental::filesystem::directory_iterator(dir)){
        if(entry.path().extension() == ".txt"){
            filenames.push_back(entry.path().string()); // arg olarak verilen adresteki dosyalar .txt ise vektöre kaydet
        }
    }
    std::sort(filenames.begin(), filenames.end(), [](const std::string& a, const std::string& b) {
        int num_a = std::stoi(a.substr(a.find_last_of("_") + 1, a.find_last_of(".") - a.find_last_of("_") - 1));
        int num_b = std::stoi(b.substr(b.find_last_of("_") + 1, b.find_last_of(".") - b.find_last_of("_") - 1));
        return num_a < num_b;
    }); // örnek bir input dosyası numbers_0.txt için _ ve . arasındaki sayıları karşılaştırıp sortla
    for(const auto& filename: filenames){
            std::ifstream fin(filename);
            fin >> std::fixed>>std::setprecision(4);
            std::vector<float> input;
            float temp;
            while(fin >> temp) {
                input.push_back(temp); // .txt dosyasını okuyup vektöre kaydet
            }
            fin.close();
            inputs.push_back(input); // dosya içeriklerini, ait olduğu dosyanın vector of vectors'üne ekle
            input.clear(); // sonraki dosya içeriklerini kaydetmeden önce vektörü temizle
    }
    return inputs; // tüm dosyaların ve her bir dosyanın içeriğini vector of vectors olarak return et
}

void Encryption(std::vector<std::vector<float>> inputs, float key){
    std::experimental::filesystem::create_directories("./encryption/"); // encryption klasörü oluştur
    for(int i = 0; i<inputs.size();i++){
        std::string filename ="./encryption/encrypted_" + std::to_string(i) + ".txt";
        std::ofstream fout(filename);
        fout << std::fixed<<std::setprecision(4);
        for(int j = 0; j<inputs[i].size(); j++){
            fout<<inputs[i][j]*key<<"\n"; // readFiles fonksiyonunda return edilen vector of vectors'ü key ile çarpıp out et
        }
        fout.close();
    }
}


int main(int argc, char* argv[]){
    float key = 0.5f; // başlangıç için basit bir encryption
    std::string decision = argv[1]; // encryption için e, decryption için d
    std::string dir = argv[2]; // input txt'leri adresi
    if(decision == "e"){
        std::vector<std::vector<float>> inputs = readFile(dir);
        Encryption(inputs, key) ;
    }
    else if(*argv[1] == 'd'){
        std::cout<<"Not available!"<<std::endl; // henüz kodlanmadı
    }
    else{
        std::cout<<"ERROR"<<std::endl<<"Encryption: e"<<std::endl<<"Decryption: d"<<std::endl;
    }
    return 0;
}