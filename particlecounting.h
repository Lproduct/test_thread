#ifndef PARTICLECOUNTING_H
#define PARTICLECOUNTING_H

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <string>

#include "cvblob.h"

enum LaneOrientation
{
  LO_NONE       = 0,
  LO_HORIZONTAL = 1,
  LO_VERTICAL   = 2
};

enum VehiclePosition
{
  VP_NONE = 0,
  VP_A  = 1,
  VP_B  = 2
};

struct Count
{
    int countAB;
    int countBA;
};

class ParticleCounting
{
public:
    ParticleCounting();
    ~ParticleCounting();

    void setInput(const cv::Mat &i);
    void setTracks(const cvb::CvTracks &t);
    void process();
    int getCountAB();
    int getCountBA();
    Count getCount();
    void setLineHorizontal();
    void setLineVertical();
    void clearCount();

private:
    VehiclePosition getVehiclePosition(const CvPoint2D64f centroid);

    bool firstTime;
    bool showOutput;
    int key;
    cv::Mat img_input;
    cvb::CvTracks tracks;
    std::map<cvb::CvID, std::vector<CvPoint2D64f>> points;
    LaneOrientation LaneOrientation;
    std::map<cvb::CvID, VehiclePosition> positions;
    long countAB;
    long countBA;
    int img_w;
    int img_h;
    int showAB;
    Count count;
};

#endif // PARTICLECOUNTING_H
