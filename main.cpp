#include <iostream>
#include <cmath>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <baseapi.h>
#include <allheaders.h>


using namespace std;
using namespace cv;

double angle(Point pt1, Point pt2, Point pt0);
void findSquares(const Mat& pic, vector<vector<Point> >& squares);
void findLargestSquare(const vector<vector<Point> >& squares, vector<Point>& biggest_square);
void warpInit(vector<Point>& biggest_square, Mat pic);
void bubbleSort(vector<Point>& largest_square);

int main()
{
    //원본 이미지 불러오기
    Mat pic = imread("/Users/economy/Desktop/pic3.jpeg");
    if (pic.empty())
    {
        cout << "!!! Failed to open image" << endl;
        return -1;
    }

    //에지 검출
    vector<vector<Point> > squares;
    findSquares(pic, squares);

    //에지 표시
    Mat src_squares = pic.clone();
    for (size_t i = 0; i < squares.size(); i++) {
        const Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(src_squares, &p, &n, 1, true, Scalar(0, 255, 0), 2, CV_AA);
    }

    //꼭짓점 검출
    vector<Point> largest_square;
    findLargestSquare(squares, largest_square);

    for (size_t i = 0; i < largest_square.size(); i++ ) {
        circle(pic, largest_square[i], 4, Scalar(0, 0, 255), FILLED);
        
    }
    
    //기하학 변형
    warpInit(largest_square, pic);
    waitKey(0);

    return 0;
}


double angle(Point pt1, Point pt2, Point pt0)
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}


void findSquares(const Mat& pic, vector<vector<Point> >& squares)
{
    Mat matGray;
    cvtColor(pic, matGray, COLOR_BGR2GRAY);

    Mat matBlur;
    blur(matGray, matBlur, Size(3, 3));

    Mat matCanny;
    Canny(matBlur, matCanny, 128, 255, 3);

    Mat matDilate;
    dilate(matCanny, matDilate, Mat(), Point(-1, -1), 2, 1, 1);

    vector<vector<Point> > contours;
    findContours(matDilate, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

    vector<Point> approx;
    for (size_t i = 0; i < contours.size(); i++)
    {
        approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
        if (approx.size() == 4 && fabs(contourArea(Mat(approx))) > 1000 &&
            isContourConvex(Mat(approx)))
        {
            double maxCosine = 0;
            for (int j = 2; j < 5; j++)
            {
                double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                maxCosine = MAX(maxCosine, cosine);
            }

            if (maxCosine < 0.3)
                squares.push_back(approx);
        }
    }
}


void findLargestSquare(const vector<vector<Point> >& squares, vector<Point>& biggest_square)
{
    if (!squares.size())
    {
        cout << "findLargestSquare !!! No squares detect, nothing to do." << endl;
        return;
    }

    int max_width = 0;
    int max_height = 0;
    int max_square_idx = 0;
    for (size_t i = 0; i < squares.size(); i++)
    {
        Rect rectangle = boundingRect(Mat(squares[i]));
        if ((rectangle.width >= max_width) && (rectangle.height >= max_height))
        {
            max_width = rectangle.width;
            max_height = rectangle.height;
            max_square_idx = i;
        }
    }

    biggest_square = squares[max_square_idx];
}

void warpInit(vector<Point>& largest_square, Mat pic) {
    
    //기하학 변형을 위한 정렬
    bubbleSort(largest_square);
    
    vector<Point2f> corners(4);
    for (size_t i = 0; i < largest_square.size(); i++ ) {
        corners[i] = Point2f(largest_square[i].x, largest_square[i].y);
    }
    
    //출력될 사이즈
    Size warpSize(674, 424);
    Mat warpImg(warpSize, pic.type());

    vector<Point2f> warpCorners(4);
    warpCorners[0]=Point2f(0, 0);
    warpCorners[1]=Point2f(warpImg.cols, 0);
    warpCorners[2]=Point2f(0, warpImg.rows);
    warpCorners[3]=Point2f(warpImg.cols, warpImg.rows);

    //변형
    Mat trans=getPerspectiveTransform(corners, warpCorners);
    
    warpPerspective(pic, warpImg, trans, warpSize);
    imshow("warpImg", warpImg);
}

void bubbleSort(vector<Point>& largest_square) {
    int tempX;
    int tempY;
    for(int i =0; i < 4; i ++) {
        for(int j = 0; j < 3-i; j ++) {
            if(largest_square[j].x + largest_square[j].y > largest_square[j+1].x + largest_square[j+1].y){
                tempX = largest_square[j].x;
                tempY = largest_square[j].y;
                
                largest_square[j].x = largest_square[j+1].x;
                largest_square[j].y = largest_square[j+1].y;
                
                largest_square[j+1].x = tempX;
                largest_square[j+1].y = tempY;
            }
        }
    }
    
    if(largest_square[1].x < largest_square[2].x || largest_square[1].y > largest_square[2].y) {
        tempX = largest_square[1].x;
        tempY = largest_square[1].y;
        
        largest_square[1].x = largest_square[2].x;
        largest_square[1].y = largest_square[2].y;
        
        largest_square[2].x = tempX;
        largest_square[2].y = tempY;
    }
}

