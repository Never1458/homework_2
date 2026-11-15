#include "Detector.hpp"
#include <iostream>

// 构造函数：初始化颜色表（红、橙、黄、绿、蓝、白）
Detector::Detector()
{
    cube_colors[0] = Vec3b(0, 0, 255);
    cube_colors[1] = Vec3b(0, 165, 255);
    cube_colors[2] = Vec3b(0, 255, 255);
    cube_colors[3] = Vec3b(0, 255, 0);
    cube_colors[4] = Vec3b(255, 0, 0);
    cube_colors[5] = Vec3b(255, 255, 255);
}

// 析构函数
Detector::~Detector()
{
}

// 用于识别 AprilTag
void Detector::detect(Mat &frame)
{
    if (frame.empty())
    {
        return;
    }

    Mat gray, binary;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    threshold(gray, binary, 127, 255, THRESH_BINARY);

    auto dict = aruco::getPredefinedDictionary(aruco::DICT_APRILTAG_36h11);
    auto params = aruco::DetectorParameters::create();
    vector<vector<Point2f>> corners, rejected;
    vector<int> ids;
    aruco::detectMarkers(gray, dict, corners, ids, params, rejected);
    if (!ids.empty())
    {
        aruco::drawDetectedMarkers(frame, corners, ids);
    }

    imshow(WINDOW_RAW, frame);
    imshow(WINDOW_BINARY, binary);
    imshow(WINDOW_RESULT, frame);
}

// 重载函数：用于识别魔方视频
void Detector::detect(const string &video_path)
{
    VideoCapture cap(video_path);
    if (!cap.isOpened())
    {
        cout << "Cannot open video: " << video_path << endl;
        return;
    }

    Mat frame, gray_img, binary_img, cube_face;
    while (true)
    {
        cap >> frame;
        if (frame.empty())
        {
            break;
        }

        // 反二值化 + 形态学去噪
        cvtColor(frame, gray_img, COLOR_BGR2GRAY);
        threshold(gray_img, binary_img, CUBE_BINARY_THRESH, 255, THRESH_BINARY_INV);
        morphologyEx(binary_img, binary_img, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(2, 2)));
        morphologyEx(binary_img, binary_img, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(9, 9)));

        // 找最大外轮廓
        vector<vector<Point>> contour_list;
        findContours(binary_img, contour_list, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        double max_area = 0;
        vector<Point> max_contour;
        for (size_t i = 0; i < contour_list.size(); i++)
        {
            double area = contourArea(contour_list[i]);
            if (area > max_area)
            {
                max_area = area;
                max_contour = contour_list[i];
            }
        }

        if (!max_contour.empty() && max_area > CUBE_MIN_AREA)
        {
            vector<Point> approx;
            approxPolyDP(max_contour, approx, 12, true);
            Point2f src[4];
            if (approx.size() == 4)
            {
                for (int i = 0; i < 4; i++) src[i] = Point2f(approx[i].x, approx[i].y);
            }
            else
            {
                Rect box = boundingRect(max_contour);
                src[0] = Point2f(box.x, box.y);
                src[1] = Point2f(box.x + box.width, box.y);
                src[2] = Point2f(box.x + box.width, box.y + box.height);
                src[3] = Point2f(box.x, box.y + box.height);
            }

            // 透视校正为 4×4 正方形
            Point2f dst[4] = {Point2f(0, 0), Point2f(CUBE_SIZE - 1, 0),
                              Point2f(CUBE_SIZE - 1, CUBE_SIZE - 1), Point2f(0, CUBE_SIZE - 1)};
            warpPerspective(frame, cube_face, getPerspectiveTransform(src, dst), Size(CUBE_SIZE, CUBE_SIZE));

            // 逐格采样 HSV 判色
            Mat hsv_face;
            cvtColor(cube_face, hsv_face, COLOR_BGR2HSV);
            Vec3b cell_color[CUBE_GRID][CUBE_GRID];
            for (int r = 0; r < CUBE_GRID; r++)
            {
                for (int c = 0; c < CUBE_GRID; c++)
                {
                    int x = c * CUBE_CELL + CUBE_CELL / 2;
                    int y = r * CUBE_CELL + CUBE_CELL / 2;
                    Scalar mean_hsv = mean(hsv_face(Rect(x - 10, y - 10, 20, 20)));
                    cell_color[r][c] = Detector_ClassifyHsv((int)mean_hsv[0], (int)mean_hsv[1], (int)mean_hsv[2]);
                    circle(cube_face, Point(x, y), 10, cell_color[r][c], -1);
                }
            }
            // 外圈投票决定外框颜色
            int cnt[6] = {0};
            int edge_idx[EDGE_SAMPLE_NUM] = {0, 1, 2, 3, 4, 7, 8, 11, 12, 13, 14, 15};
            for (int k = 0; k < EDGE_SAMPLE_NUM; k++)
            {
                int r = edge_idx[k] / CUBE_GRID;
                int c = edge_idx[k] % CUBE_GRID;
                for (int ci = 0; ci < 6; ci++)
                {
                    if (cell_color[r][c] == cube_colors[ci])
                    {
                        cnt[ci]++;
                        break;
                    }
                }
            }
            int best = 0;
            for (int i = 1; i < 6; i++)
            {
                if (cnt[i] > cnt[best]) best = i;
            }

            vector<Point> outer = {Point(src[0]), Point(src[1]), Point(src[2]), Point(src[3])};
            drawContours(frame, vector<vector<Point>>{outer}, -1, cube_colors[best], 3);
            imshow(WINDOW_CUBE_FACE, cube_face);
        }

        imshow(WINDOW_RAW, frame);
        imshow(WINDOW_BINARY, binary_img);
        imshow(WINDOW_RESULT, frame);
        if (waitKey(30) == 27) break;
    }
    cap.release();
}
// 按 HSV 均值返回对应颜色的 BGR 值
Vec3b Detector::Detector_ClassifyHsv(int h, int s, int v)
{
    if (((h >= 0 && h <= 10) || (h >= 170 && h <= 179)) && s >= 130 && v >= 70) return cube_colors[0];
    else if (h >= 11 && h <= 22 && s >= 130 && v >= 100) return cube_colors[1];
    else if (h >= 23 && h <= 38 && s >= 100 && v >= 120) return cube_colors[2];
    else if (h >= 39 && h <= 85 && s >= 100 && v >= 70) return cube_colors[3];
    else if (h >= 90 && h <= 130 && s >= 100 && v >= 70) return cube_colors[4];
    else if (s <= 90 && v >= 170) return cube_colors[5];
    return Vec3b(200, 200, 200);
}
