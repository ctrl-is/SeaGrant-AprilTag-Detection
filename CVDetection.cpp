#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include "serialhelper.hpp"


int main() {
    std::cout << "PROGRAM START\n";
    auto dict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_APRILTAG_36h11);

    int fd = openSerial("/dev/cu.usbmodem40517814832", 115200);

    if (fd < 0) {
    std::cerr << "Serial open failed\n";
    return 1;
    }

    bool ok0 = writeLine(fd, "ID:TEST\n");
    std::cout << "Sent startup TEST (ok=" << ok0 << ")\n";
   
    // cv::Mat markerImage;
    // for (int i = 0; i < 5; ++i) {
    //     cv::aruco::generateImageMarker(dict, i, 200, markerImage, 1);
    //     cv::imwrite("marker" + std::to_string(i) + ".png", markerImage);
    // }
    // cv::aruco::generateImageMarker(dict, 23, 200, markerImage, 1);
    // cv::imwrite("marker23.png", markerImage);

    cv::aruco::DetectorParameters params;
    cv::aruco::ArucoDetector detector(dict, params);

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Could not open camera\n";
        return 1;
    }

    cv::Mat frame, gray;
    int lastSentId = -1;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        std::vector<int> ids;
        std::vector<std::vector
        <cv::Point2f>> corners, rejected;
        detector.detectMarkers(gray, corners, ids, rejected);

        if (!ids.empty()) {
            cv::aruco::drawDetectedMarkers(frame, corners, ids);
            std::cout << "Detected: ";
            for (int id : ids) std::cout << id << " ";
            std::cout << "\n";

            int id = ids[0];
            if (id != lastSentId) {
                std::string msg = std::to_string(id) + "\n";
                bool ok = writeLine(fd, msg);
            std::cout << "Sent -> " << msg << " (ok=" << ok << ")\n";
                lastSentId = id;
            }
        } else {
            if (lastSentId != -2) {
                bool ok = writeLine(fd, "NONE\n");
                std::cout << "Sent -> NONE (ok=" << ok << ")\n";
                lastSentId = -2;
            }
    }

        cv::imshow("AprilTag Detection (OpenCV newest API)", frame);
        if ((cv::waitKey(1) & 0xFF) == 'q') break;
    }
    return 0;
}