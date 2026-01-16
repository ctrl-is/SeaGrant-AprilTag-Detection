#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>


int main() {
    auto dict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_APRILTAG_36h11);
    cv::Mat markerImage;
    cv::aruco::generateImageMarker(dict, 23, 200, markerImage, 1);
    cv::imwrite("marker23.png", markerImage);

    cv::aruco::DetectorParameters params;
    cv::aruco::ArucoDetector detector(dict, params);

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Could not open camera\n";
        return 1;
    }

    cv::Mat frame, gray;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners, rejected;
        detector.detectMarkers(gray, corners, ids, rejected);

        if (!ids.empty()) {
            cv::aruco::drawDetectedMarkers(frame, corners, ids);
            std::cout << "Detected: ";
            for (int id : ids) std::cout << id << " ";
            std::cout << "\n";
        }

        cv::imshow("AprilTag Detection (OpenCV newest API)", frame);
        if ((cv::waitKey(1) & 0xFF) == 'q') break;
    }
    return 0;
}