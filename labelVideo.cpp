#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

int main(int argc, char **argv) {
    if(argc < 2) {
        std::cout << "usage: ./labelVideo path-to-video" << std::endl;
        exit(0);
    }
    std::string filename(argv[1]);
    cv::VideoCapture cap(filename);
    cv::Mat frame;

    std::ofstream fireDataFile;
    filename = filename.substr(0, filename.size() - 5);
    fireDataFile.open(filename + ".lbl");
    
    while(true) {
        if(!cap.read(frame)) break;
        cv::imshow("frame",frame);

        // if(cv::waitKey(10) == 27) {
            // break; 
        // }
        // std::cout << cv::waitKey(0) << std::endl;
        int firePresent = cv::waitKey(0);

        if(firePresent == 27) { //escape
            exit(0);
        } else if(firePresent == 177 || firePresent == 49) {
            //fire present
            fireDataFile << "1";
        } else if(firePresent == 176 || firePresent == 48) {
            //no fire
            fireDataFile << "0";
        }
        
    }
    fireDataFile << std::endl;
}