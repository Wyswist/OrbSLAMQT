///home/bartlomiej/Praca/NowySlam/ORB_SLAM2/Vocabulary/ORBvoc.txt /home/bartlomiej/Praca/NowySlam/ORB_SLAM2/Examples/Monocular/TUM1.yaml /home/bartlomiej/Praca/SLAM/ORB_SLAM2/Examples/RGB-D/rgbd_dataset_freiburg1_xyz
//Dobra komentarze będe robił po polsku. Doxygen po angielsku.

#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>

#include<opencv2/core/core.hpp>

#include<System.h>

using namespace std;
/*!
 * This function takes as a parameters: string that is the path to file where are pictures to analysis and two vectors.Into the first one image names are pushed. To the second one
 * timestamps between taking a pictures are pushed in.
 * @param strFile path to file with pictures
 * @param vstrImageFilenames vector for pictures
 * @param vTimestamps vector for timestamps between pics
 */
void LoadImages(const string &strFile, vector<string> &vstrImageFilenames, vector<double> &vTimestamps);

int main(int argc, char **argv)
{

    //Podawane są 4 argumenty podczas uruchamania programu.Są one wpisywane w konsoli. W Qt podwanae
    //Jako domyślne argumenty
    if(argc != 4)
    {
        cerr << endl << "Usage: ./mono_tum path_to_vocabulary path_to_settings path_to_sequence" << endl;
        return 1;
    }

    //To są wektoy wykorzystywane w przetwarzaniu pliku w którym są zapisane dane do zdjęć
    //które są przetwarzane jako sekwencje/klatki(analiza poszczególnych obrazów) filmu.
    vector<string> vstrImageFilenames;                              // nazwa danego zdjęcia
    vector<double> vTimestamps;                                     // Tu zpisywany jest czas od klatki do klatki
    // Retrieve paths to images
    string strFile = string(argv[3])+"/rgb.txt";                    //Tak naprawde analizowany jest plik .txt nie raw zdjecia rgb
    LoadImages(strFile, vstrImageFilenames, vTimestamps);           //Zwykłe ładowanie do wektorów informacji z pliku :)

    //Pobieranie liczby przetwarzanych zdjęć
    int nImages = vstrImageFilenames.size();

/****************************************************************************************************/
/*******************WEJSCIE DO SYSTEMU przetwarzanie obrazu tracking i maping************************/
/****************************************************************************************************/
/**/    // Create SLAM system. It initializes all system threads and gets ready to process frames./**/
/**/    // Tu wchodzi do systemu zarządzajacego ()                                                /**/
/**/    ORB_SLAM2::System SLAM(argv[1],argv[2],ORB_SLAM2::System::MONOCULAR,true);                /**/
/****************************************************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/

    // Vector for tracking time statistics
    vector<float> vTimesTrack;
    vTimesTrack.resize(nImages);

    cout << endl << "-------" << endl;
    cout << "Start processing sequence ..." << endl;
    cout << "Images in the sequence: " << nImages << endl << endl;

    // Main loop
    cv::Mat im;
    for(int ni=0; ni<nImages; ni++)
    {
        // Read image from file
        im = cv::imread(string(argv[3])+"/"+vstrImageFilenames[ni],CV_LOAD_IMAGE_UNCHANGED);
        double tframe = vTimestamps[ni];

        if(im.empty())
        {
            cerr << endl << "Failed to load image at: "
                 << string(argv[3]) << "/" << vstrImageFilenames[ni] << endl;
            return 1;
        }
    /**
     * This an object of class that represent a point in time. It is implemented as if it stores
     * a value of time from start of the Clock's epoch
     *
     * now() - return current time
     * */
#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t1 = std::chrono::monotonic_clock::now();
#endif

        // Pass the image to the SLAM system
        SLAM.TrackMonocular(im,tframe);

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t2 = std::chrono::monotonic_clock::now();
#endif

        double ttrack= std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();

        vTimesTrack[ni]=ttrack;

        // Wait to load the next frame
        double T=0;
        if(ni<nImages-1)
            T = vTimestamps[ni+1]-tframe;
        else if(ni>0)
            T = tframe-vTimestamps[ni-1];

        if(ttrack<T)
            usleep((T-ttrack)*1e6);
    }

    // Stop all threads
    SLAM.Shutdown();

    // Tracking time statistics
    sort(vTimesTrack.begin(),vTimesTrack.end());
    float totaltime = 0;
    for(int ni=0; ni<nImages; ni++)
    {
        totaltime+=vTimesTrack[ni];
    }
    cout << "-------" << endl << endl;
    cout << "median tracking time: " << vTimesTrack[nImages/2] << endl;
    cout << "mean tracking time: " << totaltime/nImages << endl;

    // Save camera trajectory
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");

    return 0;
}

void LoadImages(const string &strFile, vector<string> &vstrImageFilenames, vector<double> &vTimestamps)
{
    ifstream f;
    f.open(strFile.c_str());

    // skip first three lines
    string s0;
    getline(f,s0);
    getline(f,s0);
    getline(f,s0);

    while(!f.eof())
    {
        string s;
        getline(f,s);
        if(!s.empty())
        {
            //Sometimes it is very convenient to use stringstream to convert between strings and other numerical types.
            //The usage of stringstream is similar to the usage of iostream,
            //so it is not a burden to learn.Stringstreams can be used to both read strings and write data into strings.
            //It mainly functions with a string buffer, but without an real I/O channel.
            //To answer the question. stringstream basically allows you to treat a
            //string object like a stream, and use all stream functions and operators on it.
            // https://stackoverflow.com/questions/20594520/what-exactly-does-stringstream-do
            /*!
               stringstream ma fajne zachowanie. Jeżeli dany string jest liczbą to można go łatwo zrzutować
               na int'a albo z powrotem na string
             */
            stringstream ss;
            ss << s;
            double t;
            string sRGB;
            //if stringstream is a numeber get it as an number
            ss >> t;
            //and put it back to the vTimestamps vector
            vTimestamps.push_back(t);
            //if stringstream is a string keep it as a string
            ss >> sRGB;
            //and put it back to fileName vector
            vstrImageFilenames.push_back(sRGB);
        }
    }
}
