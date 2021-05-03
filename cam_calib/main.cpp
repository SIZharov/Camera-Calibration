#include <sstream>
#include "opencv2/opencv.hpp"
#include <opencv2/aruco.hpp>
#include <opencv2/ccalib/randpattern.hpp>
#include <opencv2/ccalib/multicalib.hpp>

#define PATTERNWIDTH 289
#define PATTERNHAIGHT 163    //Add yor own pattern size!

using namespace cv;

/*
cv::Mat GetRandPattern(const int& width, const int& height)
{
    cv::Mat pattern;
    cv::randpattern::RandomPatternGenerator generator(width, height);
    generator.generatePattern();
    pattern = generator.getPattern();
    return pattern;
}

cv::Mat GetMarker ()
{
    cv::Mat markerImage;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    cv::aruco::drawMarker(dictionary, 23, 200, markerImage, 1);
    cv::imwrite("../marker23.png", markerImage);
    return markerImage;
}
*/

std::string GetCoordinates(std::vector<Vec3d>& rvecs, std::vector<Vec3d>& tvecs) {
    //Get coordinates of a marker in camera coordinate system
    FileStorage fs ("Coordinates.yaml", FileStorage::WRITE);
    fs<<"coordinates"<<"[";
    fs<<tvecs[0][0]; //X coordinate
    fs<<tvecs[0][1]; // Y coordinate
    fs<<tvecs[0][2]; //Z coordinate
    fs<<"]";
    fs.release();
    //Return string with coordinates
    std::stringstream sstr;
    sstr<<"Camera:  ";
    sstr<<"X: "<<tvecs[0][0]<<' ';
    sstr<<"Y: "<<tvecs[0][1]<<' ';
    sstr<<"Z: "<<tvecs[0][2]<<' '<<'\n';
    std::string tmp_str;
    std::getline(sstr, tmp_str,'\n');
    return tmp_str;
}

int main(int argc, char **argv)
{
    if (argc>1)
    {
        if (argc==2)
        {
            //Read intrinsic parameters
            Mat cameraMatrix, distCoeffs;
            FileStorage fs("im_storage/CalibParams.yaml",FileStorage::READ);
            fs["A"]>>cameraMatrix;
            fs["dist"]>>distCoeffs;
            fs.release();
            if (cameraMatrix.empty() && distCoeffs.empty()) {
                std::cout<<"im_storage/CalibParams.yaml is empty! Calibrate yor camera"<<'\n';
                return 0;
            }

            //Detect marker on the input image and compute coordinates in a camera system
            std::string path=argv[1];
            Mat inputImage=imread(path);

            Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::DICT_6X6_250);
            std::vector<cv::Vec3d> rvecs, tvecs;
            std::string cam_coord;
            std::vector<int> markerIds;
            std::vector<std::vector<Point2f>> markerCorners, rejectedCandidates;
            Ptr<aruco::DetectorParameters> parameters = aruco::DetectorParameters::create();
            aruco::detectMarkers(inputImage, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
            Mat outputImage = inputImage.clone();

            if (markerIds.size()>0)
            {
                aruco::drawDetectedMarkers(outputImage, markerCorners, markerIds);
                aruco::estimatePoseSingleMarkers(markerCorners, 0.05, cameraMatrix, distCoeffs, rvecs, tvecs);

                // draw axis for each marker
                for (int i=0; i<markerIds.size(); i++)
                {
                    aruco::drawAxis(outputImage, cameraMatrix, distCoeffs,rvecs[i], tvecs[i], 0.05);
                }
                cam_coord=GetCoordinates(rvecs, tvecs); // needs to output coordinates on the screen
            }

            Point pt(10,30);
            Scalar sc(0,255,0);
            putText(outputImage, cam_coord, pt,1,1,sc,2);
            imshow("test", outputImage);
            waitKey(0);
        }
        else
        {
            // Calibrate camera and store intrinsic parameters

            //Read images
            FileStorage fs("im_storage/images_to_calib.yaml", FileStorage::WRITE);
            fs << "images" << "[";
            for(int i = 1; i < argc; i++){
                fs << std::string(argv[i]);
            }
            fs << "]";
            fs.release();

            //Calibrate
            multicalib::MultiCameraCalibration multiCalib( multicalib::MultiCameraCalibration::PINHOLE,2, "im_storage/images_to_calib.yaml",PATTERNWIDTH, PATTERNHAIGHT);
            multiCalib.run();
            multiCalib.writeParameters("im_storage/CalibParams.yaml");
            std::cout<<"Сalibration is complete. The data is stored in im_storage/CalibParams.yaml"<<'\n';
            return 0;
        }
    }

    std::cout<<"No image to read"<<'\n';
    return 0;
}
