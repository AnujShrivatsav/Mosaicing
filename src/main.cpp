//
//  main.cpp
//  temporalgradient
//
//  Created by Anuj Shrivastav and Anush sriram Ramesh on 3/9/23.
//
// apply a Harris corner detector to find corners in two images, auto- matically find corresponding features, estimate a homography between the two images, and warp one image into the coordinate system of the second one to produce a mosaic containing the union of all pixels in the two images.

#include <iostream>
#include "../inc/cv_factory.hpp"
#include <string>

using namespace std;
using namespace cv;

int main(int argc, const char * argv[])
{
    // create a cv_factory object
    cv_factory cvf;
    // command line arguments to get path of 2 images
    string path1 = argv[1];
    string path2 = argv[2];
    string path3 = argv[3];
    // string path4 = argv[4];
    // create a vector to store the directory of the images
    vector<string> directory;
    // add the directory of the images to
    directory.push_back(path1);
    directory.push_back(path2);
    directory.push_back(path3);
    // read the images from the directory
    vector<Mat> imgs = cvf.read_images(directory);
    // convert images to grayscale

    //vector<Point> corners1, corners2;
    Mat dst1, dst2;

    // find the corners in the images
    tuple<vector<Point>, Mat> corners1 = cvf.find_corners(imgs[0]);
    // take the corner vector from the tuple
    vector<Point> corners1_vec = get<0>(corners1);
    dst1 = get<1>(corners1);
    tuple<vector<Point>, Mat> corners2 = cvf.find_corners(imgs[1]);
    //cout<<corners1_vec.size()<<endl;
    vector<Point> corners2_vec = get<0>(corners2);
    dst2 = get<1>(corners2);

    vector<pair<Point, Point>> corres = cvf.find_correspondences(imgs[0], imgs[1], corners1_vec, corners2_vec);

    //print size of corres
    cout << "Size of corres: " << corres.size() << endl;

    // draw the correspondences
    Mat outputImage_preRansac = cvf.draw_lines(imgs[0], imgs[1], corres);

    // RANSAC to find the homography
    vector<pair<Point, Point>> BestCorres;
    Mat BestH = cvf.RANSAC(corres, BestCorres);


    Mat outputImage = cvf.draw_lines(imgs[0], imgs[1], BestCorres);
    cout << "Size of corres: " << BestCorres.size() << endl;

    // warp the image
    Mat warpedImage = cvf.warpImage(imgs[0], imgs[1], BestH);

    // find correspondances between the new image and a third image
    tuple<vector<Point>, Mat> corners3 = cvf.find_corners(imgs[2]);
    tuple<vector<Point>, Mat> corners4 = cvf.find_corners(warpedImage);
    vector<Point> corners3_vec = get<0>(corners3);
    vector<Point> corners4_vec = get<0>(corners4);

    vector<pair<Point, Point>> corres2 = cvf.find_correspondences(warpedImage, imgs[2], corners4_vec, corners3_vec);

    // RANSAC to find the homography
    vector<pair<Point, Point>> BestCorres2;
    Mat BestH2 = cvf.RANSAC(corres2, BestCorres2);

    // warp the image
    Mat warpedImage2 = cvf.warpImage(warpedImage, imgs[2], BestH2);

    // display the images
    imshow("Image 1", imgs[0]);
    imshow("Image 2", imgs[1]);
    imshow("Image 5", outputImage);
    imshow("Image 6", warpedImage);
    // imshow("Image 7", warpedImage2);

    // save the images to /output directory
    imwrite("../output/correspondances.jpg", outputImage);
    imwrite("../output/correspondances_preRansac.jpg", outputImage_preRansac);
    imwrite("../output/warpedImage.jpg", warpedImage);
    imwrite("../output/warpedImage2.jpg", warpedImage2);
    imwrite("../output/corners1.jpg", dst1);
    imwrite("../output/corners2.jpg", dst2);

    waitKey(0);
}
