#include <iostream>
#include <opencv2/opencv.hpp>
#include <unordered_map>
#include <string>
#include <fstream>
#include <chrono>

using namespace cv;

typedef Point3_<uint8_t> Pixel;
void threshold(Pixel &pixel);
void labThreshold(Pixel &pixel, Pixel &averages, Mat &motion, const int*);
Pixel calculateAverageValues(Mat &image);
bool analyseMotionDft(const int *motionHistory, const int averageVal);
void motionThreshold(Pixel &pixel, Pixel &averages, Mat &motion, const int *position);
void detectFire(Mat &image, std::ostream& file);


void initialiseMotionTracker(int, int);


#define MOTION_WINDOW_SIZE 30

int motionWindowPosition = 0;


int main(int argc, char** argv)
{
    //argument parsing. 
    if(argc < 2) {
        std::cout << "usage: ./flametrace path-to-video optional: -step, -hide" << std::endl;
        exit(0);
    }
    std::string arg3;
    if(argc == 3) {
        arg3 = argv[2];
    }

    bool stepFlag = false;
    bool show = true;
    if(arg3 == "-step") {
        stepFlag = true;
    } else if(arg3 == "-hide") {
        show = false;
    }

    //some string manipulation for results output. 

    std::string path(argv[1]);
    VideoCapture cap(path); //initialise video
    
    std::string resultsDir("./results/");
    std::string extension(".out");
    std::size_t pos = path.find_last_of("/\\");
    std::string file = path.substr(pos + 1);
    std::string resultsFile = file.substr(0, file.size() - 5);
    std::string outputFile = resultsDir + resultsFile + extension;

    std::ofstream output;
    output.open(outputFile);

    //Start of interesting stuff
    // double fps = cap.get(CAP_PROP_FPS);
    // std::cout << "fps: " << fps << std::endl;

    Mat frame;

    Ptr<BackgroundSubtractor> pBackSub;     //initialise background subtraction
    pBackSub = createBackgroundSubtractorKNN();


    if(!cap.read(frame)) {      //open video.
        std::cout << "problem with first frame " << std::endl;
    }

    int frameNo = 0;
    int dimensions[3] = {frame.rows, frame.cols, MOTION_WINDOW_SIZE};   //actually way too big but it doesn't matter, as long as there's space
    Mat motion(3, dimensions, CV_8U);   // matrix that stores temporal motion information
    
    {   //a whole new scope

    //initialise all the Mats we need for images
    Mat temp;
    Mat shrunkImage;
    Mat frameInLabSpace;

    Mat generalMotionDetect;
    Mat forColourTest;
    Mat fftMotionDetect;

    Mat combined;
    Mat secondCombined;

    Pixel averagePixelValues;

    std::vector<Mat> colourChannels(3); //temporary variables for splitting images into channels
    std::vector<Mat> channels(3);

    Mat dilateKernel;
    Mat erodeKernel;

    Mat finalImage;
    while(true) {

        
        if(!cap.read(frame)) break;
        // resize(frame, shrunkImage, Size(), 0.25, 0.25, CV_INTER_AREA);  //for uni
        resize(frame, shrunkImage, Size(), 0.25, 0.25, INTER_AREA);        //for vm

        if (show) imshow("frame", shrunkImage);
        motionWindowPosition = (motionWindowPosition + 1) % MOTION_WINDOW_SIZE; // move along to next index of buffer

        cvtColor(shrunkImage, frameInLabSpace, COLOR_BGR2Lab);  //convert from RBG to CIE L*a*b*
        // if (show) imshow("raw", frame);
        if (show) imshow("frame L*a*b*", frameInLabSpace);
        
        averagePixelValues = calculateAverageValues(frameInLabSpace);
        forColourTest = frameInLabSpace.clone();
        fftMotionDetect = frameInLabSpace.clone();
        // Mat generalMotionDetect = frameInLabSpace.clone();


        forColourTest.forEach<Pixel>    //run the pixel colour threshold over the frame
        (
        [&averagePixelValues, &motion](Pixel &pixel, const int * position) -> void
        {
            labThreshold(pixel, averagePixelValues, motion, position);
        }
        );

        pBackSub->apply(frameInLabSpace, generalMotionDetect);      //do background subtraction on the frame

        /**
         * instead of this have a forEach update motion, then a foreach on the combined image
         * to find actually likely ones
         */
        fftMotionDetect.forEach<Pixel>  // detect high frequency motion in the frame, should change to run only on pixels that are moving and fire coloured
        (
        [&averagePixelValues, &motion](Pixel &pixel, const int * position) -> void
        {
            motionThreshold(pixel, averagePixelValues, motion, position);
        }
        );



        
        std::vector<Mat> motionChannels;
        split(fftMotionDetect, motionChannels);


        // if(show) imshow("colour", forColourTest);
        split(forColourTest, colourChannels);

        dilateKernel = Mat::ones(4,4,CV_8U);
        dilate(colourChannels[0], colourChannels[0], dilateKernel);
        dilate(generalMotionDetect, generalMotionDetect, dilateKernel);
        if(show) imshow("Motion", generalMotionDetect);
        if(show) imshow("Fourier Motion" , fftMotionDetect);
        if(show) imshow("Colour", colourChannels[0]);
        //combine all of the tests together
        bitwise_and(colourChannels[0], motionChannels[0], combined);

        bitwise_and(generalMotionDetect, combined, secondCombined);

        // if(show) imshow("combined", secondCombined);
        

        split(secondCombined, channels);

        
        // do a threshold to convert into format for eroding and dilating.
        threshold(channels[0], finalImage, 100, 255, THRESH_BINARY);
        
        dilateKernel = Mat::ones(5,5,CV_8U);
        erodeKernel = Mat::ones(2,2, CV_8U);

        // perform an erode and dilate to filter out very small detected areas. 
        erode(finalImage, finalImage, erodeKernel);
        dilate(finalImage, finalImage, dilateKernel);
        
        if(show) imshow("Output", finalImage);

        detectFire(finalImage, output);

        if(!show) {
            continue;
        }

        if(stepFlag) {
            if(waitKey(0) == 27) {
                break; 
            }
        } else {
            if(waitKey(1) == 27) {
                break;
            }
        }
    }
    output << std::endl;
    std::cout << std::endl;
    }
    return 0;
}

/**
 * Count how many fire pixels have been detected in the frame.
 * If it is above a threshold output a 1 to the results to
 * denote fire detected. 
 */
void detectFire(Mat& image, std::ostream& file) {
    int count = 0;
    for(int r = 0; r < image.rows; r++) {
        for(int c = 0; c < image.cols; c++) {
            uchar p = image.at<uchar>(r, c);
            if(p > 0) {
                count++;
            }
        }
    }

    if(count > 10) {
        file << "1";
        std::cout << "1";
    } else {
        file << "0";
        std::cout << "0";
    }
}
 
/**
 * Find the average value in each colour channel. 
 */
Pixel calculateAverageValues(Mat &image) {
    long sumX2 = 0;
    long sumY2 = 0;
    long sumZ2 = 0;

    for(int r = 0; r < image.rows; r++) {
        for(int c = 0; c < image.cols; c++) {
            Pixel p = image.at<Pixel>(r, c);
            sumX2 += p.x;
            sumY2 += p.y;
            sumZ2 += p.z;
        }
    }

    long totalPixels = image.cols * image.rows;

    sumX2 = sumX2 / totalPixels;
    sumY2 = sumY2 / totalPixels;
    sumZ2 = sumZ2 / totalPixels;

    return Pixel(sumX2, sumY2, sumZ2);
}

/**
 *
 */
void threshold(Pixel &pixel) {
    if(pixel.z > pixel.y && pixel.y > pixel.x && pixel.z > 110) {
        pixel.x = 255;
        pixel.y = 255;
        pixel.z = 255;
    } else {
        pixel.x = 0;
        pixel.y = 0;
        pixel.z = 0;
    }
}

/**
 * Do a dft on the given pixel's temporal data. 
 */
bool analyseMotionDft(const int *motionHistory, const int averageVal) {

    std::vector<float> motion;

    //motion data is kept in a circular buffer, this is un-circling it.
    for (int i = 0; i < MOTION_WINDOW_SIZE; i++) { 
        int start = motionWindowPosition;
        int offset = i;
        int actualPosition = (start + offset) % MOTION_WINDOW_SIZE;

        motion.push_back(motionHistory[actualPosition]);
    }

    Mat ff;
    dft(motion, ff, DFT_ROWS | DFT_COMPLEX_OUTPUT);

    Mat planes[] = {Mat::zeros(motion.size(), 1, CV_32F), Mat::zeros(motion.size(), 1, CV_32F)};

    split(ff, planes);  //expensive, is there a way around this?

    magnitude(planes[0], planes[1], planes[0]);
    

    // very simple, can probably be improved on. 
    // if (planes[0].at<float>(10) > 400) {
    //     return true;
    // }

    if(planes[0].at<float>(8) > 500.0) {
        return true;
    }

    // if(planes[0].at<float>(7) > 800.0) {
    //     return true;
    // }



    return false;
        
}


/**
 * Threshold a given pixel based on its 
 * current colour values.
 * x is the L channel, y is the a* channel, z is the b* channel
 */
void labThreshold(Pixel &pixel, Pixel &averages, Mat &motion, const int * position) {
    //maybe this would be better? play with it
    if(
        (
        pixel.x >= averages.x &&
        pixel.y >= averages.y &&
        pixel.z <= pixel.y &&
        pixel.z >= averages.z
        ) 
        || 
        pixel.x > 200 &&
        pixel.y > pixel.z        
        ) 
        {
        pixel.x = 255;
        pixel.y = 255;
        pixel.z = 255;
    } else {
        pixel.x = 0;
        pixel.y = 0;
        pixel.z = 0;
    }
}

/**
 * Updates the temporal information for the pixel and then
 * calls the dft analysis function. 
 */
void motionThreshold(Pixel &pixel, Pixel &averages, Mat &motion, const int *position) {
    motion.at<int[MOTION_WINDOW_SIZE]>(position[0], position[1])[motionWindowPosition] = pixel.x;
    if(
        analyseMotionDft(motion.at<int[MOTION_WINDOW_SIZE]>(position[0],position[1]), averages.x)
    ) {
        pixel.x = 255;
        pixel.y = 255;
        pixel.z = 255;
    } else {
        pixel.x = 0;
        pixel.y = 0;
        pixel.z = 0;
    }
}

// Parallel execution with function object.
struct Operator
{
  void operator ()(Pixel &pixel, const int * position) const
  {
    // Perform a simple threshold operation
    threshold(pixel);
  }
};