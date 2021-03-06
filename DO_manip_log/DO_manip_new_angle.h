#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <opencv2/video/tracking.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/imgcodecs.hpp"
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>

using namespace cv;
using namespace std;

struct img_p{
    Point sl;
    Point sr;
    Eigen::Vector2f ne;
};

struct angleFeature3Pts{
    vector<Point2f> sPt;
    Eigen::Matrix<float, 6, 1> s;
    float angle;
    Eigen::Matrix<float, 1, 6> gradient;  // i.e. Jacobian, it is in row form.
};

bool yComparator(Point pt1, Point pt2){
    return (pt1.y < pt2.y);
}

vector<Point> *referencePolygon;
bool distanceComparator(Point pt1, Point pt2)
{
    if ((*referencePolygon).empty()){
        cout << "Warning: The Reference Polygon Is Empty!" << endl;
        return false;
    }
    float minDistance1 = 1e3, minDistance2 = 1e3;
    //int step = (*referencePolygon).size() / 20 + 1;
    int step = 2;
    for (auto itr = (*referencePolygon).begin(); ((*referencePolygon).end() - itr) > step; itr+=step){
        minDistance1 = (minDistance1 < norm(*itr - pt1)) ? minDistance1 : norm(*itr - pt1);
        minDistance2 = (minDistance2 < norm(*itr - pt2)) ? minDistance2 : norm(*itr - pt2);

    }
    return (minDistance1 < minDistance2);
}

Point getProjection(Point pt, vector<Point> &referContour)
{
    if (referContour.empty()){
        cout << "Warning: The Contour Is Empty!" << endl;
        return Point(0,0);
    }
    Point projectionPt = referContour[0];
    int step = 1;
    for (auto itr = referContour.begin(); (referContour.end() - itr) > step; itr+=step){
        projectionPt = (norm(*itr - pt) < norm(projectionPt - pt)) ? *itr : projectionPt;
    }
    return projectionPt;
}

Point2f LK_point;
// vector<Point2f> points[2];
bool addRemovePt = false;
Point2f pTrack_point;
bool pAddRemovePt = false;
Mat saveImg;
RNG cvrng(12345);
int randomInt = cvrng.uniform(0,100);
static void onMouse(int event, int x, int y, int, void*)
{
    //std::cout << "MOUSE MOTION DETECETED1--->>>\n";
    static int savedImgCount = 1;
    if (event == EVENT_LBUTTONDOWN){
        LK_point = Point2f((float)x, (float)y);
        addRemovePt = true;
    }
    if (event == EVENT_LBUTTONDBLCLK){
        pTrack_point = Point2f((float)x, (float)y);
        pAddRemovePt = true;
    }    
    if (event == EVENT_RBUTTONDBLCLK){
        imwrite("/home/huangjing/Pictures/" + to_string(randomInt+savedImgCount) + ".jpg", saveImg);
        savedImgCount++;
    }
}


class LK_tracker{
public: 
    string winTrack;
    TermCriteria termcrit;
    int maxCount;
    Size subPixWinSize, winSize;
    vector<Point2f> points[2];
    Mat originImg;
    bool validFeature = false;
    //bool initialPoint = true;
    //Point2f pointFeatureTarget;
    //Point2f pointFeature;

    LK_tracker() {}
    LK_tracker(const string winName)
    {
        winTrack = winName;
        termcrit = TermCriteria(TermCriteria::COUNT | TermCriteria::EPS, 20, 0.03);
        maxCount = 3;
        subPixWinSize = Size(10, 10);
        winSize = Size(11, 11);
    }

    void track(Mat& image, Mat& gray, Mat& prevGray) // Technically, no diffenence between passing cv::Mat and cv::Mat& 
    {
        originImg = image;  // Just create a matrix header to the original image.
        saveImg = image;
        setMouseCallback(winTrack, onMouse, 0);
        if (!points[0].empty()){
            vector<uchar> status;
            vector<float> err;
            if (prevGray.empty())
                gray.copyTo(prevGray);
            calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
                                 3, termcrit, 0, 0.001);
            size_t i, k;
            for (i = k = 0; i < points[1].size(); i++){
                if (addRemovePt){
                    if (norm(LK_point - points[1][i]) <= 5){
                        addRemovePt = false;
                        continue;
                    }
                }
                if (!status[i])
                    continue;
                points[1][k++] = points[1][i];
                circle(image, points[1][i], 3, Scalar(0, 255, 0), -1, 8);
            }
            points[1].resize(k);

            //if (!points[1].empty()){
            //    pointFeature = points[1][0];
            //    validFeature = true;
            //}
            //else{
            //    validFeature = false;
            //}
        }

        if (addRemovePt && points[1].size() < (size_t) maxCount){
            vector<Point2f> tmp;
            tmp.push_back(LK_point);
            cornerSubPix(gray, tmp, winSize, Size(-1, -1), termcrit);
            points[1].push_back(tmp[0]);
            addRemovePt = false;
            //if (initialPoint){
            //    pointFeatureTarget = tmp[0] - Point2f(50.0, 40.0);
            //    initialPoint = false;
            //}
        }
        //if (!initialPoint)
            //circle(image, pointFeatureTarget, 8, Scalar(255, 0, 0), 2, 8);
        std::swap(points[1], points[0]);
        // cv::swap(prevGray, gray);  // swap finished by p extraction operation
    }

    angleFeature3Pts angle3Pts()
    {
        angleFeature3Pts res;
        if (points[1].size() < 3){
            std::cout << "No Sufficient Reference Pixels!\n";
            validFeature = false;
            return res;
        }
        for (int i = 0; i < 3; i++){
            res.sPt.push_back(points[1][i]);
            res.s(2*i, 0) = points[1][i].x;
            res.s(2*i+1, 0) = points[1][i].y;
        }
        float x_0 = points[1][0].x, y_0 = points[1][0].y,
              x_1 = points[1][1].x, y_1 = points[1][1].y,
              x_2 = points[1][2].x, y_2 = points[1][2].y;
        Eigen::Vector2f v_1(x_1 - x_0, y_1 - y_0),
                        v_2(x_2 - x_0, y_2 - y_0);
        float t = v_1.dot(v_2) / (v_1.norm()*v_2.norm()),
              const_1 = -1 / sqrt(1 - t*t),
              const_2 = v_1.dot(v_2),
              n_1 = v_1.norm(), n_2 = v_2.norm();
        res.angle = acos(t) * 180 / M_PI;
        res.gradient(0,0) =  const_1 * ((2*x_0-x_1-x_2)*pow(n_1,2)*pow(n_2,2) - const_2*
                             ((x_0-x_1)*pow(n_2,2)+(x_0-x_2)*pow(n_1,2))) / (pow(n_1,3)*pow(n_2,3));
        res.gradient(0,1) = const_1 * ((2*y_0-y_1-y_2)*pow(n_1,2)*pow(n_2,2) - const_2*
                            ((y_0-y_1)*pow(n_2,2)+(y_0-y_2)*pow(n_2,2))) / (pow(n_1,3)*pow(n_2,3));
        res.gradient(0,2) = const_1 * ((x_2-x_0)*pow(n_1,2)*n_2 - (x_1-x_0)*const_2*n_2) / (pow(n_1,3)*pow(n_2,2));
        res.gradient(0,3) = const_1 * ((y_2-y_0)*pow(n_1,2)*n_2 - (y_1-y_0)*const_2*n_2) / (pow(n_1,3)*pow(n_2,2));
        res.gradient(0,4) = const_1 * ((x_1-x_0)*pow(n_2,2)*n_1 - (x_2-x_0)*const_2*n_1) / (pow(n_1,2)*pow(n_2,3));
        res.gradient(0,5) = const_1 * ((y_1-y_0)*pow(n_2,2)*n_1 - (y_1-y_0)*const_2*n_1) / (pow(n_1,2)*pow(n_2,3));
        
        cv::line(originImg, points[1][0], points[1][1], Scalar(0, 0, 255), 2);
        cv::line(originImg, points[1][0], points[1][2], Scalar(0, 0, 255), 2);
        validFeature = true;
        return res;
    }
};


class imgExtractor{
public:
    Scalar DOHSVLow;
    Scalar DOHSVHigh;
    Mat originHSVImg;
    vector<vector<Point>> DOContours;
    vector<Point> DOContour; 
    img_p endeffectorP;
    vector<int> segmentationIdx[2];
    int DOLargestCotrIdx = 0;
    bool DOExtractSucceed = false;
    bool PExtractSucceed = false;
    bool extractSucceed = false;
    bool effectorCharacterizeSucceed = false;

    string winTrack;
    TermCriteria termcrit;
    int maxCount;
    Size subPixWinSize, winSize;
    vector<Point2f> pPoints[2];

    vector<float> contactDistances;
    
    imgExtractor() {}
    imgExtractor(const string winName)
    {
        DOHSVLow = Scalar(142, 96, 72);
        DOHSVHigh = Scalar(180, 255, 255);

        winTrack = winName;
        termcrit = TermCriteria(TermCriteria::COUNT | TermCriteria::EPS, 20, 0.03);
        maxCount = 2;
        subPixWinSize = Size(10, 10);
        winSize = Size(11, 11);
    }

    void extract(Mat& image, Mat& HSVImg, Mat& gray, Mat& prevGray, int occlusion = 0)
    {
        originHSVImg = HSVImg;
        if (originHSVImg.empty()){
            cout << "No Valid Image Inputed!\n";
            exit(-1);
        }

        Mat DOdst;
        inRange(originHSVImg, DOHSVLow, DOHSVHigh, DOdst);
        Moments m_dst = moments(DOdst, true);
        //cout << m_dst.m00 <<" " << m_dst.m10 << " " << m_dst.m01 << "\n";
        if (m_dst.m00 < 5000){
            cout << "No DO Detected!\n";
            DOExtractSucceed = false;
        }
        else{
            findContours(DOdst, DOContours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));  
            // cout << "DOContours Size: " << DOContours.size() << " " << DOContours[0].size() << "\n";
            DOLargestCotrIdx = 0;
            for (int i = 1; i < DOContours.size(); i++){
                DOLargestCotrIdx = (DOContours[i].size() > DOContours[DOLargestCotrIdx].size()) ? i : DOLargestCotrIdx;
            }
            DOContour = DOContours[DOLargestCotrIdx];
            DOExtractSucceed = true;
        }

        if (occlusion != 0){
            int sndLargestCotrIdx = (DOLargestCotrIdx == 0) ? 1 : 0;
            for (int i = 0; i < DOContours.size(); i++){
                sndLargestCotrIdx = (DOContours[i].size() > DOContours[sndLargestCotrIdx].size() && i != DOLargestCotrIdx)
                                    ? i : sndLargestCotrIdx;
            }
            DOContour.insert(DOContour.end(), DOContours[sndLargestCotrIdx].begin(), DOContours[sndLargestCotrIdx].end());
            DOContours[DOLargestCotrIdx] = DOContour; 
        }

        if (!pPoints[0].empty()){
            vector<uchar> status;
            vector<float> err;
            if (prevGray.empty())
                gray.copyTo(prevGray);
            calcOpticalFlowPyrLK(prevGray, gray, pPoints[0], pPoints[1], status, err, winSize, 3, termcrit, 0, 0.001);
            size_t i, k;
            for (i = k = 0; i < pPoints[1].size(); i++){
                if (pAddRemovePt){
                    if (norm(pTrack_point - pPoints[1][i]) <= 5){
                        pAddRemovePt = false;
                        continue;
                    } 
                }
                if (!status[i])
                    continue;
                pPoints[1][k++] = pPoints[1][i];
                circle(image, pPoints[1][i], 4, Scalar(255, 0, 0), -1, 8);
            }
            pPoints[1].resize(k);
        }
        if (pAddRemovePt && pPoints[1].size() < (size_t) maxCount){
            vector<Point2f> tmp;
            tmp.push_back(pTrack_point);
            cornerSubPix(gray, tmp, winSize, Size(-1, -1), termcrit);
            pPoints[1].push_back(tmp[0]);
            pAddRemovePt = false;
        }
        std::swap(pPoints[1], pPoints[0]);
        std::swap(prevGray, gray);
        
        PExtractSucceed = (pPoints[1].size() == 2);
        extractSucceed = DOExtractSucceed && PExtractSucceed;
    }

    void effectorCharacterize()
    {
        effectorCharacterizeSucceed = false;
        if (pPoints[1].size() == 2){  
            if (pPoints[1][0].x < pPoints[1][1].x){
                endeffectorP.sl = pPoints[1][0];
                endeffectorP.sr = pPoints[1][1];
            }
            else{
                endeffectorP.sl = pPoints[1][1];
                endeffectorP.sr = pPoints[1][0];
            }
            Eigen::Vector2f endeffectorDirt(endeffectorP.sr.x - endeffectorP.sl.x, 
                                            endeffectorP.sr.y - endeffectorP.sl.y);
            endeffectorDirt = endeffectorDirt / endeffectorDirt.norm();
            endeffectorP.ne(0) = endeffectorDirt(1);    // Make sure ne points upwards.
            endeffectorP.ne(1) = -endeffectorDirt(0);
            effectorCharacterizeSucceed = true;
        }
    }

    bool segment(vector<Point> &contour)
    {
        if (!effectorCharacterizeSucceed){
            cout << "No End-effector Info On The Image Available!" << endl;
            return false;
        }
        float a = endeffectorP.sr.x - endeffectorP.sl.x,    // line: ax + by + c = 0
              b = endeffectorP.sr.y - endeffectorP.sl.y,
              cl = -a*endeffectorP.sl.x - b*endeffectorP.sl.y,
              cr = -a*endeffectorP.sr.x - b*endeffectorP.sr.y,
              divCof = 1 / norm(endeffectorP.sr - endeffectorP.sl), 
              distancel, distancer;
        vector<int> idxl, idxr;
        // vector<float> disLog;
        int distanceThreshold = 10, step = 1;
        for (int i = 0; contour.size() - i > step; i+=step){
            distancel = abs(a*contour[i].x + b*contour[i].y + cl) * divCof;
            distancer = abs(a*contour[i].x + b*contour[i].y + cr) * divCof;
            // disLog.push_back(distancel);
            if (distancel < distanceThreshold)
                idxl.push_back(i);
            if (distancer < distanceThreshold)
                idxr.push_back(i);
        }
        if (idxl.empty() || idxr.empty()){
            cout << "Invalid Segmentation. Maybe A Samll Threshold Distance Or A Large Step Is Given." << endl;
            return false;
        }
        
        int  segIdx1 = idxl[0], segIdx2;
        float distanceSeg;
        bool gap = false;
        for (int gapPos = 0; gapPos+1 < idxl.size(); gapPos++){
            distancel = abs(a*contour[idxl[gapPos]].x + b*contour[idxl[gapPos]].y + cl) * divCof;
            if (!gap){
                distanceSeg = abs(a*contour[segIdx1].x + b*contour[segIdx1].y + cl) * divCof; 
                segIdx1 = (distanceSeg < distancel) ? segIdx1 : idxl[gapPos];
                if (idxl[gapPos+1] - idxl[gapPos] != 1){
                    gap = !gap;
                    segIdx2 = idxl[gapPos+1];
                }
            }
            else{
                distanceSeg = abs(a*contour[segIdx2].x + b*contour[segIdx2].y + cl) * divCof; 
                segIdx2 = (distanceSeg < distancel) ? segIdx2 : idxl[gapPos];
                if (idxl[gapPos+1] - idxl[gapPos] != 1)
                    gap = !gap;
            }
        }
        
        segmentationIdx[0].clear();
        if (norm(contour[segIdx1] - endeffectorP.sl) < norm(contour[segIdx2] - endeffectorP.sl)){
            segmentationIdx[0].push_back(segIdx1);
            segmentationIdx[0].push_back(segIdx2);
        }
        else{
            segmentationIdx[0].push_back(segIdx2);
            segmentationIdx[0].push_back(segIdx1);           
        }

        segIdx1 = idxr[0];
        gap = false;
        for (int gapPos = 0; gapPos+1 < idxr.size(); gapPos++){
            distancer = abs(a*contour[idxr[gapPos]].x + b*contour[idxr[gapPos]].y + cr) * divCof;
            if (!gap){
                distanceSeg = abs(a*contour[segIdx1].x + b*contour[segIdx1].y + cr) * divCof; 
                segIdx1 = (distanceSeg < distancer) ? segIdx1 : idxr[gapPos];
                if (idxr[gapPos+1] - idxr[gapPos] != 1){
                    gap = !gap;
                    segIdx2 = idxr[gapPos+1];
                }
            }
            else{
                distanceSeg = abs(a*contour[segIdx2].x + b*contour[segIdx2].y + cr) * divCof; 
                segIdx2 = (distanceSeg < distancer) ? segIdx2 : idxr[gapPos];
                if (idxr[gapPos+1] - idxr[gapPos] != 1)
                    gap = !gap;
            }
        }

        segmentationIdx[1].clear();
        if (norm(contour[segIdx1] - endeffectorP.sr) < norm(contour[segIdx2] - endeffectorP.sr)){
            segmentationIdx[1].push_back(segIdx1);
            segmentationIdx[1].push_back(segIdx2);
        }
        else{
            segmentationIdx[1].push_back(segIdx2);
            segmentationIdx[1].push_back(segIdx1);           
        }

        contactDistances.clear();
        Point closestPt = getProjection(endeffectorP.sl, contour);
        contactDistances.push_back(norm(endeffectorP.sl - closestPt));
        closestPt = getProjection(endeffectorP.sr, contour);
        contactDistances.push_back(norm(endeffectorP.sr - closestPt));

        return true;
        /*sort(disLog.begin(), disLog.end());
        for (auto itr = disLog.begin(); itr != disLog.end(); itr++)
            cout << *i```tr << "\n";
        cout << contour << endl;
        segmentationIdx[0] = idxl;
        segmentationIdx[1] = idxr;*/
        
        /*int minIdxl = 0, maxIdxl = 0;
        for (int i = 1; i < idxl.size(); i++){
            minIdxl = (norm(contour[idxl[i]] - endeffectorP.sl) < norm(contour[idxl[minIdxl]] - endeffectorP.sl)) ? i:minIdxl;
            maxIdxl = (norm(contour[idxl[i]] - endeffectorP.sl) > norm(contour[idxl[maxIdxl]] - endeffectorP.sl)) ? i:maxIdxl;
        }
        segmentationIdx[0].clear(); 
        segmentationIdx[0].push_back(idxl[minIdxl]);
        segmentationIdx[0].push_back(idxl[maxIdxl]);
        cout << norm(contour[minIdxl] - endeffectorP.sl) << " " << norm(contour[maxIdxl] - endeffectorP.sl) << "\n";

        int minIdxr = 0, maxIdxr = 0;
        for (int i = 1; i < idxr.size(); i++){
            minIdxr = (norm(contour[idxr[i]] - endeffectorP.sr) < norm(contour[idxr[minIdxr]] - endeffectorP.sr)) ? i:minIdxr;
            maxIdxr = (norm(contour[idxr[i]] - endeffectorP.sr) > norm(contour[idxr[maxIdxr]] - endeffectorP.sr)) ? i:maxIdxr;
        }
        segmentationIdx[1].clear(); 
        segmentationIdx[1].push_back(idxr[minIdxr]);
        segmentationIdx[1].push_back(idxr[maxIdxr]);
        return true;*/
    }

    vector<Point> feaibleMotionSearch(int searchBase = 0) // searchBase == 0, rightwards, searchBase == 1, leftwards
    {
        vector<Point> newPConfig;
        if (segmentationIdx[0].empty() || segmentationIdx[1].empty()){
            cout << "Segmentation Has Not Been Performed." << endl;
            return newPConfig;
        }

        Point basePt, endPt;
        float pLength = norm(endeffectorP.sl - endeffectorP.sr);
        int basePtIdx, endPtIdx, 
            motionStep = 10;  // The motion step is determined by the contour points for convenience.
        if (searchBase == 0){
            basePtIdx = (segmentationIdx[0][0] + motionStep) > (DOContour.size()-1) ? 0 : segmentationIdx[0][0] + motionStep;
            basePt = DOContour[basePtIdx];           
            if (basePtIdx == DOContour.size() - 1)
                endPtIdx = 0;
            else 
                endPtIdx = basePtIdx + 1;

            for (int i = endPtIdx; DOContour[i].x > DOContour[i-1].x; i++){  // Igonre the extreme case where basePt is the left-most point.
                if (i == DOContour.size() - 1)
                    i = 0;
                endPtIdx = (abs(norm(DOContour[i]-basePt) - pLength) < abs(norm(DOContour[endPtIdx]-basePt) - pLength))
                            ? i : endPtIdx;
            }
            endPt = DOContour[endPtIdx];
        }     
        else{
            basePtIdx = (segmentationIdx[1][0] + motionStep) > (DOContour.size()-1) ? 0 : segmentationIdx[1][0] + motionStep;
            basePt = DOContour[basePtIdx];
            if (basePtIdx == 0)
                endPtIdx = DOContour.size() - 1;
            else
                endPtIdx = basePtIdx - 1;

            for (int i = endPtIdx; DOContour[i].x < DOContour[i+1].x; i--){ // Igonre the extreme case where basePt is the right-most point.
                if (i == 0)
                    i = DOContour.size() - 2;
                endPtIdx = (abs(norm(DOContour[i]-basePt) - pLength) < abs(norm(DOContour[endPtIdx]-basePt) - pLength))
                            ? i : endPtIdx;
            }
            endPt = DOContour[endPtIdx];
        }
        newPConfig.push_back(basePt);
        newPConfig.push_back(endPt);
        return newPConfig;
    }
};

class optConstructor{
public:
    imgExtractor extractedImg;
    float T_CP;
    Eigen::Matrix<float, 6, 1> pT_pP;   // partial T partial P
    Point2f ElCentroid;
    Point2f ECentroid;
    Point2f ErCentroid;
    Point2f PrialDirtl;
    Point2f PrialDirtr;
    vector<float> deformAngles;
    Point2f sw;  // Weighted equivalent visual feedback point
    int indicatorw;
    float distancew;
    vector<float> distanceLinelrp;

    optConstructor() {}
    optConstructor(imgExtractor &extractor)
    {
        extractedImg = extractor;
    }

    void getLocalContact()
    {
        Point projPtl = getProjection(extractedImg.endeffectorP.sl, extractedImg.DOContour),
              projPtr = getProjection(extractedImg.endeffectorP.sr, extractedImg.DOContour),
              projPtm = getProjection((extractedImg.endeffectorP.sl + extractedImg.endeffectorP.sr)/2, extractedImg.DOContour),
              gradtl = projPtl - extractedImg.endeffectorP.sl,
              gradtr = projPtr - extractedImg.endeffectorP.sr,
              gradtm = projPtm - (extractedImg.endeffectorP.sl + extractedImg.endeffectorP.sr)/2;
        T_CP  = max(norm(projPtl - extractedImg.endeffectorP.sl), norm(projPtr - extractedImg.endeffectorP.sr));
        pT_pP << gradtl.x, gradtl.y, gradtr.x, gradtr.y, gradtm.x, gradtm.y;
    }

    void getDeformConstraint()
    {   
        vector<Point> *contourPtr = &extractedImg.DOContour;
        vector<Point> El, E, Er;
        if (extractedImg.segmentationIdx[0].empty() || extractedImg.segmentationIdx[1].empty()){
            cout << "No Valid Segmentation Info Available From The imgExtractor！" << endl;
            return;
        }
        int k1 = extractedImg.segmentationIdx[0][0], k2 = extractedImg.segmentationIdx[0][1],
            k3 = extractedImg.segmentationIdx[1][0], k4 = extractedImg.segmentationIdx[1][1];
        cout << "k1-k2-k3-k4:\n" << k1 << "-" << k2 << "-" << k3 << "-" << k4 << "\n";
        if (k2 <= k1 && k1 <= k3 && k3 <= k4){
            El = vector<Point>((*contourPtr).begin() + k2, (*contourPtr).begin() + k1 + 1);
            Er = vector<Point>((*contourPtr).begin() + k3, (*contourPtr).begin() + k4 + 1);
            E = vector<Point>((*contourPtr).begin(), (*contourPtr).begin() + k2 + 1);
            E.insert(E.end(), (*contourPtr).begin() + k1, (*contourPtr).begin() + k3 + 1);
            E.insert(E.end(), (*contourPtr).begin() + k4, (*contourPtr).end());
        }
        else if (k1 <= k3 && k3 <= k4 && k4 <= k2){
            El = vector<Point>((*contourPtr).begin() + k2, (*contourPtr).end());
            El.insert(El.end(), (*contourPtr).begin(), (*contourPtr).begin() + k1 + 1);
            Er = vector<Point>((*contourPtr).begin() + k3, (*contourPtr).begin() + k4 + 1);
            E = vector<Point>((*contourPtr).begin() + k1, (*contourPtr).begin() + k3 + 1);
            E.insert(E.end(), (*contourPtr).begin() + k4, (*contourPtr).end());
            E.insert(E.end(), (*contourPtr).begin(), (*contourPtr).begin() + k2 + 1);
        }
        else if (k3 <= k4 && k4 <= k2 && k2 <= k1){
            El = vector<Point>((*contourPtr).begin() + k2, (*contourPtr).begin() + k1 + 1);
            Er = vector<Point>((*contourPtr).begin() + k3, (*contourPtr).begin() + k4 + 1);
            E = vector<Point>((*contourPtr).begin(), (*contourPtr).begin() + k3 + 1);
            E.insert(E.end(), (*contourPtr).begin() + k4, (*contourPtr).begin() + k2 + 1);
            E.insert(E.end(), (*contourPtr).begin() + k1, (*contourPtr).end());
        }
        else if (k4 <= k2 && k2 <= k1 && k1 <= k3){
            El = vector<Point>((*contourPtr).begin() + k2, (*contourPtr).begin() + k1 + 1);
            Er = vector<Point>((*contourPtr).begin(), (*contourPtr).begin() + k4 + 1);
            Er.insert(Er.end(), (*contourPtr).begin() + k3, (*contourPtr).end());
            E = vector<Point>((*contourPtr).begin() + k4, (*contourPtr).begin() + k2 + 1);
            E.insert(E.end(), (*contourPtr).begin() + k1, (*contourPtr).begin() + k3 + 1);
        }   // Igonre the extreme case where basePt is the left-most point.
        else{
            cout << "Bad Segmentation Of The Contour!" << endl;
            return;
        }
        Moments ElMoments = moments(El), EMoments = moments(E), ErMoments = moments(Er);
        ElCentroid = Point2f(ElMoments.m10 / ElMoments.m00, ElMoments.m01 / ElMoments.m00);
        ECentroid = Point2f(EMoments.m10 / EMoments.m00, EMoments.m01 / EMoments.m00);
        ErCentroid = Point2f(ErMoments.m10 / ErMoments.m00, ErMoments.m01 / ErMoments.m00);

        Eigen::Matrix2f Il, I, Ir;
        Il << ElMoments.mu20, ElMoments.mu11, ElMoments.mu11, ElMoments.mu02;
        I << EMoments.mu20, EMoments.mu11, EMoments.mu11, EMoments.mu02;
        Ir << ErMoments.mu20, ErMoments.mu11, ErMoments.mu11, ErMoments.mu02;
        Eigen::EigenSolver<Eigen::Matrix2f> eigSloverl(Il), eigSolverr(Ir);
        Eigen::Vector2f eigVecl1(eigSloverl.eigenvectors().col(0).real()[0], 
                                 eigSloverl.eigenvectors().col(0).real()[1]),
                        eigVecl2(eigSloverl.eigenvectors().col(1).real()[0], 
                                 eigSloverl.eigenvectors().col(1).real()[1]),
                        eigVecr1(eigSolverr.eigenvectors().col(0).real()[0],
                                 eigSolverr.eigenvectors().col(0).real()[1]),
                        eigVecr2(eigSolverr.eigenvectors().col(1).real()[0],
                                 eigSolverr.eigenvectors().col(1).real()[1]);
        PrialDirtl = Point2f(eigVecl1[0], eigVecl1[1]);
        PrialDirtr = Point2f(eigVecr1[0], eigVecr1[1]);
        Eigen::Vector2f planeDirt(extractedImg.endeffectorP.sr.x - extractedImg.endeffectorP.sl.x,
                                  extractedImg.endeffectorP.sr.y - extractedImg.endeffectorP.sl.y);
        float anglel = acos(planeDirt.dot(eigVecl1) / (planeDirt.norm()*eigVecl1.norm())) * 180 / M_PI,
              angler = acos(planeDirt.dot(eigVecr1) / (planeDirt.norm()*eigVecr1.norm())) * 180 / M_PI;
        deformAngles.clear();
        deformAngles.push_back(anglel);
        deformAngles.push_back(angler);
    }

    void getManipulabilityAngle(angleFeature3Pts angFeature){
        Point2f sm = (extractedImg.endeffectorP.sl+extractedImg.endeffectorP.sr) / 2;
        float a = extractedImg.endeffectorP.sr.x - extractedImg.endeffectorP.sl.x,    // line: ax + by + c = 0
              b = extractedImg.endeffectorP.sr.y - extractedImg.endeffectorP.sl.y,
              cl = -a*extractedImg.endeffectorP.sl.x - b*extractedImg.endeffectorP.sl.y,
              cr = -a*extractedImg.endeffectorP.sr.x - b*extractedImg.endeffectorP.sr.y,
              divCof = 1 / norm(extractedImg.endeffectorP.sr - extractedImg.endeffectorP.sl),
              refl = a*sm.x + b*sm.y + cl, 
              refr = a*sm.x + b*sm.y + cr,
              episilonIn = 20,
              episilonOut = 100;  // The distance is in the range of several tens pixels.
        vector<int> EIndicator(3,0);
        vector<float> distance(3,0), Wi(3,0);
        for (int i = 0; i < 3; i++){
            float linel = a*angFeature.sPt[i].x + b*angFeature.sPt[i].y + cl,
                  liner = a*angFeature.sPt[i].x + b*angFeature.sPt[i].y + cr;
            if (linel*refl >= 0 && liner*refr >= 0){
                EIndicator[i] = 1;
                distance[i] = divCof * abs(b*angFeature.sPt[i].x - a*angFeature.sPt[i].y - 
                              b*extractedImg.endeffectorP.sl.x + a*extractedImg.endeffectorP.sl.y);
                Wi[i] = 1 / (distance[i] + episilonIn);
            }
            else if (linel*refl < 0){
                EIndicator[i] = 0;
                distance[i] = abs(linel) * divCof;
                Wi[i] = 1 / (distance[i] + episilonOut);
            }
            else{
                EIndicator[i] = 0;
                distance[i] = abs(liner) * divCof;
                Wi[i] = 1 / (distance[i] + episilonOut); 
            }
        }
        Eigen::MatrixXf Dw = Eigen::MatrixXf::Zero(6,6);
        for (int i = 0; i<3; i++){
            Dw(2*i, 2*i) = Wi[i];
            Dw(2*i+1, 2*i+1) = Wi[i];
        }
        cout << "Line Distance:\n";
        for (auto x:distance)
            cout << x << "  ";
        cout << "\nDw =\n" << Dw << "\n";
        cout << "Angle Feature Jacobian:\n" << angFeature.gradient << "\n";
        Eigen::Matrix<float, 1, 6> weightedGradient = angFeature.gradient * Dw;
        cout << "weightedGradient:\n" << weightedGradient << "\n";
        
        sw = Point2f(0,0);
        for (int i = 0; i < 3; i++){
            cout << "weight:\n"
                 << (weightedGradient.col(2*i).lpNorm<1>() + weightedGradient.col(2*i+1).lpNorm<1>()) / weightedGradient.lpNorm<1>()
                 << "\n";
            sw += (weightedGradient.col(2*i).lpNorm<1>() + weightedGradient.col(2*i+1).lpNorm<1>())
                  / weightedGradient.lpNorm<1>() * angFeature.sPt[i];
        }
        float linelw = a*sw.x + b*sw.y + cl,
              linerw = a*sw.x + b*sw.y + cr,
              linepw = b*sw.x - a*sw.y - b*extractedImg.endeffectorP.sl.x + a*extractedImg.endeffectorP.sl.y;
        if (linelw*refl >= 0 && linerw*refr >= 0){
            indicatorw = 1;
            distancew = divCof * abs(b*sw.x - a*sw.y - 
                        b*extractedImg.endeffectorP.sl.x + a*extractedImg.endeffectorP.sl.y);
        }
        else if (linelw*refl < 0){
            indicatorw = 2;
            distancew = abs(linelw) * divCof;
        }
        else{
            indicatorw = 3;
            distancew = abs(linerw) * divCof;
        }
        distanceLinelrp.clear();
        distanceLinelrp.push_back(abs(linelw) * divCof);
        distanceLinelrp.push_back(abs(linerw) * divCof);
        distanceLinelrp.push_back(abs(linepw) * divCof);        
    }

    void getManipulabilityPoint(Point2f featurePt){
        sw = featurePt;
        Point2f sm = (extractedImg.endeffectorP.sl+extractedImg.endeffectorP.sr) / 2;
        float a = extractedImg.endeffectorP.sr.x - extractedImg.endeffectorP.sl.x,    // line: ax + by + c = 0
              b = extractedImg.endeffectorP.sr.y - extractedImg.endeffectorP.sl.y,
              cl = -a*extractedImg.endeffectorP.sl.x - b*extractedImg.endeffectorP.sl.y,
              cr = -a*extractedImg.endeffectorP.sr.x - b*extractedImg.endeffectorP.sr.y,
              divCof = 1 / norm(extractedImg.endeffectorP.sr - extractedImg.endeffectorP.sl),
              refl = a*sm.x + b*sm.y + cl, 
              refr = a*sm.x + b*sm.y + cr,
              episilonIn = 20,
              episilonOut = 100;  // The distance is in the range of several tens pixels.
        float linel = a*featurePt.x + b*featurePt.y + cl,
              liner = a*featurePt.x + b*featurePt.y + cr,
              linep = b*featurePt.x - a*featurePt.y - b*extractedImg.endeffectorP.sl.x + a*extractedImg.endeffectorP.sl.y;
        if (linel*refl >= 0 && liner*refr >= 0){
            distancew = divCof * abs(b*featurePt.x - a*featurePt.y - 
                            b*extractedImg.endeffectorP.sl.x + a*extractedImg.endeffectorP.sl.y);
            indicatorw = 1;
        }
        else if (linel*refl < 0){
            distancew = abs(linel) * divCof;
            indicatorw = 2;
        }
        else{
            distancew = abs(liner) * divCof;
            indicatorw = 3; 
        }

        distanceLinelrp.clear();
        distanceLinelrp.push_back(abs(linel) * divCof);
        distanceLinelrp.push_back(abs(liner) * divCof);
        distanceLinelrp.push_back(abs(linep) * divCof);
    }
};

class deformJacobianAngle{
public: 
    Eigen::MatrixXf JdPrev;
    Eigen::MatrixXf JdCurr;
    float alpha;
    img_p pPrev;
    angleFeature3Pts anglePrev;
    Eigen::Vector2f pVecPrev;
    Eigen::Vector2f pVecCurr;  // Vector of the middle point of sl, sr is used as the equivalent end-effector contact point.
    bool initialized = false;

    deformJacobianAngle() {}
    deformJacobianAngle(img_p pInitial, angleFeature3Pts angleInitial)
    {
        alpha = 0.2;
        JdPrev = Eigen::Matrix<float, 1, 2>::Ones();
        pPrev = pInitial;
        anglePrev = angleInitial;
        pVecPrev << (pInitial.sl.x + pInitial.sr.x) / 2,
                    (pInitial.sl.y + pInitial.sr.x) / 2;
        initialized = true;
    }
    
    void update(img_p pCurr,angleFeature3Pts angleCurr)
    {
        Eigen::Matrix<float, 1, 1> delta_y;
        delta_y << angleCurr.angle - anglePrev.angle;
        pVecCurr << (pCurr.sl.x + pCurr.sr.x) / 2,
                    (pCurr.sl.y + pCurr.sr.y) / 2;
        Eigen::Vector2f delta_p = pVecCurr - pVecPrev;
        cout << "Jd Update Data:\n"
            << "Previous Jd:\n" << JdPrev << "\n"
            << "delta_y:\n" << delta_y << "\n"
            << "delta_p:\n" << delta_p << "\n";
        if (delta_p.norm() < 0.005)
            JdCurr = JdPrev;
        else
            JdCurr = JdPrev + alpha * (delta_y - JdPrev*delta_p) / (delta_p.transpose()*delta_p) * delta_p.transpose();       
        JdPrev = JdCurr;
        anglePrev = angleCurr;
        pPrev = pCurr;
        pVecPrev = pVecCurr;
    }
};


class deformJacobianPoint{
public: 
    Eigen::MatrixXf JdPrev;
    Eigen::MatrixXf JdCurr;
    float alpha;
    img_p pPrev;
    Point2f ptPrev;
    Eigen::Vector4f pVecPrev;
    Eigen::Vector4f pVecCurr; 
    bool initialized = false;

    deformJacobianPoint() {}
    deformJacobianPoint(img_p pInitial, Point2f initialPt)
    {
        alpha = 0.2;
        JdPrev = Eigen::Matrix<float, 2, 4>::Ones();
        pPrev = pInitial;
        ptPrev = initialPt;
        pVecPrev << (pInitial.sl.x + pInitial.sr.x) / 2,
                    (pInitial.sl.y + pInitial.sr.x) / 2,
                    pInitial.ne[0],
                    pInitial.ne[1];
        initialized = true;
    }
    
    void update(img_p pCurr, Point2f ptCurr)
    {
        Eigen::Vector2f delta_y;
        delta_y << ptCurr.x - ptPrev.x, ptCurr.y - ptCurr.y;
        pVecCurr << (pCurr.sl.x + pCurr.sr.x) / 2,
                    (pCurr.sl.y + pCurr.sr.y) / 2,
                    pCurr.ne[0],
                    pCurr.ne[1];
        Eigen::Vector4f delta_p = pVecCurr - pVecPrev;
        Eigen::Vector2f delta_ne = pCurr.ne - pPrev.ne;
        if (delta_ne.norm() < 0.005){
            delta_p[2] = 0;
            delta_p[3] = 0;
        }
        else{
            delta_ne = delta_ne / delta_ne.norm();
            delta_p[2] = delta_ne[0];
            delta_p[3] = delta_ne[1];
        }
        
        cout << "Jd Update Data:\n"
             << "Previous Jd:\n" << JdPrev << "\n"
             << "delta_y:\n" << delta_y << "\n"
             << "delta_p:\n" << delta_p << "\n";
        if (delta_p.norm() < 0.005)
            JdCurr = JdPrev;
        else
            JdCurr = JdPrev + alpha * (delta_y - JdPrev*delta_p) / (delta_p.transpose()*delta_p) * delta_p.transpose();       
        JdPrev = JdCurr;
        ptPrev = ptCurr;
        pPrev = pCurr;
        pVecPrev = pVecCurr;
    } 
};