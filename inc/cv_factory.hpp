//
//  cv_factory.hpp
//  temporalgradient
//
//  Created by Anuj Shrivatsav and Anush sriram Ramesh on 3/9/23.
//
// include the header files needed to perform cv operations and read and writing file directories

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <numeric>
#include <filesystem>
#include <math.h>

using namespace std;
using namespace cv;

// define class to perform cv operations
class cv_factory {
    public:
        // function to read all image files in the directory and return a vector
        vector<Mat> read_images(vector<string> directory) {
            cout<<"read_images"<<endl;
            // create a vector to store the images
            vector<Mat> imgs;
            //read images from the directory vector and store them in the imgs vector
            // Mat img = imread(directory[0]);
            // //scale down the image to 1/4th of its original size
            // resize(img, img, Size(), 0.75, 0.75);
            // imgs.push_back(img);
            // img = imread(directory[1]);
            // resize(img, img, Size(), 0.75, 0.75);
            // imgs.push_back(img);

            for(int i = 0; i < directory.size(); i++) {
                Mat img = imread(directory[i]);
                resize(img, img, Size(), 0.5, 0.5);
                imgs.push_back(img);
            }
            // print the size of imput images
            cout << "Size of input images: " << imgs[0].size() << "\t" << imgs[1].size() << endl;
            return imgs;
        }

        // find corners in the image using the harris corner detector
        tuple<vector<Point>, Mat> find_corners(Mat img) {
            cout<<"find_corners"<<endl;
            // convert the image to grayscale
            Mat gray;
            cvtColor(img, gray, COLOR_BGR2GRAY);
            // apply the harris corner detector
            Mat dst, dst_norm, dst_norm_scaled;
            dst = Mat::zeros(gray.size(), CV_32FC1);
            // compute the harris R matrix over the image
            cornerHarris(gray, dst, 5, 5, 0.04, BORDER_DEFAULT);
            // normalize the R matrix
            normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
            convertScaleAbs(dst_norm, dst_norm_scaled);
            // threshold the R matrix to find the corners
            // push corner points into a vector
            vector<Point> corners;
            for(int i = 0; i < dst_norm.rows; i++) {
                for(int j = 0; j < dst_norm.cols; j++) {
                    if((int)dst_norm.at<float>(i,j) > 120) {
                        circle(dst_norm_scaled, Point(j,i), 5, Scalar(0), 2, 8, 0);
                        corners.push_back(Point(j,i));
                    }
                }
            }
            // return the corners vector and the image with the corners marked
            tuple<vector<Point>, Mat> corners_img;
            corners_img = make_tuple(corners, dst_norm_scaled);
            return corners_img;
        }

        // function that does the normalized cross-correlation
        double NCC(Mat t1, Mat t2)
        {   //cout << ">>NCC" << endl;
            // calculate the mean of the template 1
            double mean1 = 0;
            for(int i=0; i<t1.rows; i++)
            {
                for(int j=0; j<t1.cols; j++)
                {
                    mean1 += t1.at<uchar>(i,j);
                }
            }
            mean1 = mean1/(t1.rows*t1.cols);
            // calculate the mean of the template 2
            double mean2 = 0;
            for(int i=0; i<t2.rows; i++)
            {
                for(int j=0; j<t2.cols; j++)
                {
                    mean2 += t2.at<uchar>(i,j);
                }
            }
            mean2 = mean2/(t2.rows*t2.cols);
            // calculate the standard deviation of the template 1
            double std1 = 0;
            for(int i=0; i<t1.rows; i++)
            {
                for(int j=0; j<t1.cols; j++)
                {
                    std1 += pow(t1.at<uchar>(i,j) - mean1, 2);
                }
            }
            std1 = sqrt(std1/(t1.rows*t1.cols));
            // calculate the standard deviation of the template 2
            double std2 = 0;
            for(int i=0; i<t2.rows; i++)
            {
                for(int j=0; j<t2.cols; j++)
                {
                    std2 += pow(t2.at<uchar>(i,j) - mean2, 2);
                }
            }
            std2 = sqrt(std2/(t2.rows*t2.cols));
            // calculate the normalized cross-correlation
            double ncc = 0;
            for(int i=0; i<t1.rows; i++)
            {
                for(int j=0; j<t1.cols; j++)
                {
                    ncc += (t1.at<uchar>(i,j) - mean1)*(t2.at<uchar>(i,j) - mean2);
                }
            }
            ncc = ncc/(t1.rows*t1.cols*std1*std2);
            return ncc;
        }

        // find correspondences in the image using the corner points
        vector<pair<Point, Point>> find_correspondences(Mat img1, Mat img2, vector<Point> corners1, vector<Point> corners2)
        {   cout<<"find_correspondences"<<endl;
            // convert the images to grayscale
            // cvtColor(img1, img1, COLOR_BGR2GRAY);
            // cvtColor(img2, img2, COLOR_BGR2GRAY);
            Mat template1, template2;
            double Threshold = 0.8;
            Point p2 = Point(0,0);
            // initialize a vector to store the correspondences as a pair of points
            vector<pair<Point,Point>> corres;
            for(int i=0; i<corners1.size(); i++)
            {
                // check if the template is within the image
                //cout<<"Creating templates"<<endl;
                int WindowSize = 5;
                if(corners1[i].x - WindowSize/2 <= 0 || corners1[i].x + WindowSize/2 >= img1.cols || corners1[i].y - WindowSize/2 <= 0 || corners1[i].y + WindowSize/2 >= img1.rows)
                    continue;
                // create a roi around the corner point
                cv::Rect roi(corners1[i].x - WindowSize/2, corners1[i].y - WindowSize/2, WindowSize, WindowSize);
                template1 = img1(roi);
                p2 = Point(0,0);
                double currentMax = 0;
                for(int j=0; j<corners2.size(); j++)
                {
                    // check if the template is within the image
                    if(corners2[j].x - WindowSize/2 <= 0 || corners2[j].x + WindowSize/2 >= img2.cols || corners2[j].y - WindowSize/2 <= 0 || corners2[j].y + WindowSize/2 >= img2.rows)
                        continue;
                    cv::Rect roi2(corners2[j].x - WindowSize/2, corners2[j].y - WindowSize/2, WindowSize, WindowSize);
                    template2 = img2(roi2);
                    double value = NCC(template1, template2);
                    if(value > Threshold && value > currentMax){
                        currentMax = value;
                        p2 = corners2[j];
                    }
                }
                // push the pair of points into the vector
                if (p2 != Point(0,0))
                {pair<Point, Point> p;
                p.first = corners1[i];
                p.second = p2;
                corres.push_back(p);}
            }
            return corres;
        }

        // find the homography matrix using 4 correspondance points as input
        Mat findHomography(vector<Point> corners1, vector<Point> corners2)
        {
            // create a matrix of 8x9
            Mat A = Mat::zeros(8, 9, CV_64F);
            // fill the matrix with the values
            for(int i=0; i<4; i++)
            {
                A.at<double>(2*i, 0) = corners1[i].x;
                A.at<double>(2*i, 1) = corners1[i].y;
                A.at<double>(2*i, 2) = 1;
                A.at<double>(2*i, 6) = -corners1[i].x*corners2[i].x;
                A.at<double>(2*i, 7) = -corners1[i].y*corners2[i].x;
                A.at<double>(2*i, 8) = -corners2[i].x;
                A.at<double>(2*i+1, 3) = corners1[i].x;
                A.at<double>(2*i+1, 4) = corners1[i].y;
                A.at<double>(2*i+1, 5) = 1;
                A.at<double>(2*i+1, 6) = -corners1[i].x*corners2[i].y;
                A.at<double>(2*i+1, 7) = -corners1[i].y*corners2[i].y;
                A.at<double>(2*i+1, 8) = -corners2[i].y;
            }
            // make the A matrix square
            Mat At = A.t();
            Mat AtA = At*A;
            // find the SVD of the matrix
            SVD svd(AtA);
            // the last column of the V matrix is the solution
            Mat h = svd.vt.row(8);
            // reshape the vector to a 3x3 matrix
            Mat H = h.reshape(1, 3);
            // normalize the matrix
            H = H/H.at<double>(2,2);
            return H;
        }

        // RANSAC algorithm to find the best homography matrix
        Mat RANSAC(vector<pair<Point, Point>> correspondingPoints, vector<pair<Point, Point>>& bestCorrespondingPoints)
        {
            vector<pair<Point, Point>> tempCorrespondingPoints;
            // initialize the best homography matrix
            Mat bestH = Mat::zeros(3, 3, CV_64F);
            // sample 4 points randomly from the vector of corresponding points
            vector<Point> corners1, corners2;
            int maxInliers = 0;
            for(int i=0; i<1000; i++)
            {
                corners1.clear();
                corners2.clear();
                for(int j=0; j<4; j++)
                {
                    int index = rand() % correspondingPoints.size();
                    corners1.push_back(correspondingPoints[index].first);
                    corners2.push_back(correspondingPoints[index].second);
                }
                // find the homography matrix using the 4 points
                Mat H = findHomography(corners1, corners2);
                // find the inliers
                int inliers = 0;
                tempCorrespondingPoints.clear();
                for(int j=0; j<correspondingPoints.size(); j++)
                {
                    // transform the point in img1 using the homography matrix
                    Mat p = Mat::zeros(3, 1, CV_64F);
                    p.at<double>(0, 0) = correspondingPoints[j].first.x;
                    p.at<double>(1, 0) = correspondingPoints[j].first.y;
                    p.at<double>(2, 0) = 1;
                    Mat p2 = H*p;
                    p2 = p2/p2.at<double>(2, 0);
                    // check if the transformed point is within a threshold distance from the corresponding point in img2
                    if(sqrt(pow(p2.at<double>(0, 0) - correspondingPoints[j].second.x, 2) + pow(p2.at<double>(1, 0) - correspondingPoints[j].second.y, 2)) < 1)
                        {
                            inliers++;
                            tempCorrespondingPoints.push_back(correspondingPoints[j]);
                        }
                }
                // if the number of inliers is greater than the previous best, update the best homography matrix
                if(inliers > maxInliers)
                {
                    maxInliers = inliers;
                    bestCorrespondingPoints = tempCorrespondingPoints;
                    bestH = H;
                }
            }
            return bestH;

        }

        // warp the image using the homography matrix
        Mat warpImage(Mat img1, Mat img2, Mat H)
        {
            cout<<"Warping the image..."<<endl;
            // find the corners of the image
            vector<Point> corners1, corners2;
            corners1.push_back(Point(0, 0));
            corners1.push_back(Point(img1.cols, 0));
            corners1.push_back(Point(img1.cols, img1.rows));
            corners1.push_back(Point(0, img1.rows));
            // transform the corners using the homography matrix
            cout<<"Transforming the corners..."<<endl;
            for(int i=0; i<4; i++)
            {
                Mat p = Mat::zeros(3, 1, CV_64F);
                p.at<double>(0, 0) = corners1[i].x;
                p.at<double>(1, 0) = corners1[i].y;
                p.at<double>(2, 0) = 1;
                Mat p2 = H*p;
                p2 = p2/p2.at<double>(2, 0);
                corners2.push_back(Point(p2.at<double>(0, 0), p2.at<double>(1, 0)));
            }
            cout<<"Done"<<endl;
            // find the minimum and maximum x and y values
            int minX = min(min(corners2[0].x, corners2[1].x), min(corners2[2].x, corners2[3].x));
            int maxX = max(max(corners2[0].x, corners2[1].x), max(corners2[2].x, corners2[3].x));
            int minY = min(min(corners2[0].y, corners2[1].y), min(corners2[2].y, corners2[3].y));
            int maxY = max(max(corners2[0].y, corners2[1].y), max(corners2[2].y, corners2[3].y));
            cout<<"Minimum x: "<<minX<<endl;
            // create a new image with the size of the minimum and maximum values
            Mat img3 = Mat::zeros(maxY - minY, maxX - minX, img1.type());
            cout<<"New image size: "<<img3.rows<<"x"<<img3.cols<<endl;
            // warp the image using the homography matrix
            warpPerspective(img1, img3, H, img3.size());
            cout<<"Done"<<endl;
            // blend the second image onto the new image
            cout<<"Blending the images..."<<endl;
            //img2.copyTo(img3(Rect(0, 0, img2.cols, img2.rows)));
            // traverse the new image pixel by pixel
            for(int i=0; i<img3.rows; i++)
            {
                for(int j=0; j<img3.cols; j++)
                {
                    // if the pixel is black, copy the pixel from the second image
                    if(img3.at<Vec3b>(i, j) == Vec3b(0, 0, 0))
                    {
                        if(i < img2.rows && j < img2.cols)
                            img3.at<Vec3b>(i, j) = img2.at<Vec3b>(i, j);
                    }

                    // if the pixel is not black, average the pixel with the pixel from the second image
                    else
                    {
                        if(i < img2.rows && j < img2.cols)
                        {
                            img3.at<Vec3b>(i, j)[0] = (img3.at<Vec3b>(i, j)[0] + img2.at<Vec3b>(i, j)[0])/2;
                            img3.at<Vec3b>(i, j)[1] = (img3.at<Vec3b>(i, j)[1] + img2.at<Vec3b>(i, j)[1])/2;
                            img3.at<Vec3b>(i, j)[2] = (img3.at<Vec3b>(i, j)[2] + img2.at<Vec3b>(i, j)[2])/2;
                        }
                    }
                }
            }
            cout<<"Done"<<endl;
            return img3;
        }

        // draw lines between the corresponding points in the two images and return single with two input images one above the other
        Mat draw_lines(Mat img1, Mat img2, vector<pair<Point,Point>> correspondingPoints)
        {
            cout<<"Drawing lines between corresponding points"<<endl;
            // create a new image with size of the two input images one top of another
            Mat img3 = Mat::zeros(img1.rows + img2.rows, img1.cols, img1.type());
            // copy the two input images into the new image
            img1.copyTo(img3(Rect(0, 0, img1.cols, img1.rows)));
            img2.copyTo(img3(Rect(0, img1.rows, img2.cols, img2.rows)));
            // mark the corresponding points in the two images
            for(int i=0; i<correspondingPoints.size(); i++)
            {
                // draw a circle at the corresponding points in img1
                circle(img3, correspondingPoints[i].first, 3, Scalar(0, 0, 255), 2, 8, 0);
                // draw a circle at the corresponding points in img2
                circle(img3, Point(correspondingPoints[i].second.x, correspondingPoints[i].second.y + img1.rows), 3, Scalar(0, 0, 255), 2, 8, 0);
            }
            // draw lines between the corresponding points
            for(int i=0; i<correspondingPoints.size(); i++)
            {
                line(img3, correspondingPoints[i].first, Point(correspondingPoints[i].second.x, correspondingPoints[i].second.y + img1.rows), Scalar(0, 255, 0), 1, 8, 0);
            }
            return img3;
        }

        // get the clicked pixel coordinates and store them in the correspondingPoints vector
};
