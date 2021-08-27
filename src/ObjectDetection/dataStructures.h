#ifndef dataStructures_h
#define dataStructures_h

#include <vector>
#include <opencv2/core.hpp>

using namespace cv;
using namespace std;

struct LidarPoint{
    double x, y, z, r;
};

struct boundingBox
{
    int boxID;
    int trackID;

    Rect roi;
    int classID;
    double confidence;

    vector<LidarPoint> lidarPoints;
    vector<KeyPoint> keyPoints;
    vector<DMatch> kptMatches;

};

#endif // dataStructures_h
