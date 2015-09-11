#include "particlecounting.h"

namespace FAV1
{
  IplImage* img_input1 = 0;
  IplImage* img_input2 = 0;
  int roi_x0 = 1400;
  int roi_y0 = 600;
  int roi_x1 = 200;
  int roi_y1 = 600;
  int numOfRec = 0;
  int startDraw = 0;
  bool roi_defined = true;//false
  bool use_roi = true;
  void VehicleCouting_on_mouse(int evt, int x, int y, int flag, void* param)
  {
    if(!use_roi)
      return;

    if(evt == CV_EVENT_LBUTTONDOWN)
    {
      if(!startDraw)
      {
        roi_x0 = x;
        roi_y0 = y;
        startDraw = 1;
      }
      else
      {
        roi_x1 = x;
        roi_y1 = y;
        startDraw = 0;
        roi_defined = true;
      }
    }

    if(evt == CV_EVENT_MOUSEMOVE && startDraw)
    {
      //redraw ROI selection
      img_input2 = cvCloneImage(img_input1);
      cvLine(img_input2, cvPoint(roi_x0,roi_y0), cvPoint(x,y), CV_RGB(255,0,255));
      cvShowImage("VehicleCouting", img_input2);
      cvReleaseImage(&img_input2);
    }
  }
}

ParticleCounting::ParticleCounting(): firstTime(true), showOutput(true), key(0), countAB(0), countBA(0), showAB(0)
{
    //namedWindow( "VehicleCouting", cv::WINDOW_AUTOSIZE );
}

ParticleCounting::~ParticleCounting()
{

}

void ParticleCounting::setLineHorizontal()
{
    FAV1::roi_x0 = 1400;
    FAV1::roi_y0 = 600;
    FAV1::roi_x1 = 200;
    FAV1::roi_y1 = 600;
}

void ParticleCounting::setLineVertical()
{
    FAV1::roi_x0 = 800;
    FAV1::roi_y0 = 1100;
    FAV1::roi_x1 = 800;
    FAV1::roi_y1 = 100;
}

void ParticleCounting::setInput(const cv::Mat &i)
{
    img_input = i;
}

void ParticleCounting::setTracks(const cvb::CvTracks &t)
{
    tracks = t;
}

VehiclePosition ParticleCounting::getVehiclePosition(const CvPoint2D64f centroid)
{
    VehiclePosition vehiclePosition = VP_NONE;

    if(LaneOrientation == LO_HORIZONTAL)
    {
        if(centroid.x < FAV1::roi_x0)
        {
          //cv::putText(img_input, "STATE: A", cv::Point(10,img_h/2), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
          vehiclePosition = VP_A;
        }

        if(centroid.x > FAV1::roi_x0)
        {
          //cv::putText(img_input, "STATE: B", cv::Point(10,img_h/2), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
          vehiclePosition = VP_B;
        }
    }

    if(LaneOrientation == LO_VERTICAL)
    {
        if(centroid.y > FAV1::roi_y0)
        {
          //cv::putText(img_input, "STATE: A", cv::Point(10,img_h/2), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
          vehiclePosition = VP_A;
        }

        if(centroid.y < FAV1::roi_y0)
        {
          //cv::putText(img_input, "STATE: B", cv::Point(10,img_h/2), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
          vehiclePosition = VP_B;
        }
    }

    return vehiclePosition;
}

int ParticleCounting::getCountAB()
{
    return (int) countAB;
}

int ParticleCounting::getCountBA()
{
    return (int) countBA;
}

Count ParticleCounting::getCount()
{
    count.countAB = (int) countAB;
    count.countBA = (int) countBA;
    return count;
}

void ParticleCounting::clearCount()
{
    countAB = 0;
    countBA = 0;
}

void ParticleCounting::process()
{
    if(img_input.empty())
    return;

    img_w = img_input.size().width;
    img_h = img_input.size().height;

    //--------------------------------------------------------------------------
/*
    if(FAV1::use_roi == true && FAV1::roi_defined == false && firstTime == true)
    {
        do
        {
          cv::putText(img_input, "Please, set the counting line", cv::Point(10,15), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0,0,255));
          cv::imshow("VehicleCouting", img_input);
          FAV1::img_input1 = new IplImage(img_input);
          cvSetMouseCallback("VehicleCouting", FAV1::VehicleCouting_on_mouse, NULL);
          key = cvWaitKey(0);
          delete FAV1::img_input1;

          if(FAV1::roi_defined)
          {
            std::cout << "Counting line defined (" << FAV1::roi_x0 << "," << FAV1::roi_y0 << "," << FAV1::roi_x1 << "," << FAV1::roi_y1 << ")" << std::endl;
            break;
          }
          else
            std::cout << "Counting line undefined!" << std::endl;
        }while(1);
    }
*/
    if(FAV1::use_roi == true && FAV1::roi_defined == true)
    cv::line(img_input, cv::Point(FAV1::roi_x0,FAV1::roi_y0), cv::Point(FAV1::roi_x1,FAV1::roi_y1), cv::Scalar(0,0,255));

    bool ROI_OK = false;

    if(FAV1::use_roi == true && FAV1::roi_defined == true)
    ROI_OK = true;

    if(ROI_OK)
    {
        LaneOrientation = LO_NONE;

        if(abs(FAV1::roi_x0 - FAV1::roi_x1) < abs(FAV1::roi_y0 - FAV1::roi_y1))
        {
            if(!firstTime)
            //cv::putText(img_input, "HORIZONTAL", cv::Point(10,15), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
            LaneOrientation = LO_HORIZONTAL;

            cv::putText(img_input, "Left", cv::Point(FAV1::roi_x0-50,FAV1::roi_y0), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
            cv::putText(img_input, "Right", cv::Point(FAV1::roi_x0+15,FAV1::roi_y0), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
        }

        if(abs(FAV1::roi_x0 - FAV1::roi_x1) > abs(FAV1::roi_y0 - FAV1::roi_y1))
        {
            if(!firstTime)
            //cv::putText(img_input, "VERTICAL", cv::Point(10,15), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
            LaneOrientation = LO_VERTICAL;

            cv::putText(img_input, "Down", cv::Point(FAV1::roi_x0,FAV1::roi_y0+22), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
            cv::putText(img_input, "Up", cv::Point(FAV1::roi_x0,FAV1::roi_y0-12), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
        }
    }

    //--------------------------------------------------------------------------
    for(std::map<cvb::CvID,cvb::CvTrack*>::iterator it = tracks.begin() ; it != tracks.end(); it++)
    {
        ///Get track informations
        cvb::CvID id = (*it).first;
        cvb::CvTrack* track = (*it).second;

        CvPoint2D64f centroid = track->centroid;

        if(track->inactive == 0)
        {
            if(positions.count(id) > 0)
            {
                std::map<cvb::CvID, VehiclePosition>::iterator it2 = positions.find(id);
                VehiclePosition old_position = it2->second;

                VehiclePosition current_position = getVehiclePosition(centroid);

                if(current_position != old_position)
                {
                    if(old_position == VP_A && current_position == VP_B)
                    countAB++;

                    if(old_position == VP_B && current_position == VP_A)
                    countBA++;

                    positions.erase(positions.find(id));
                }
            }
            else
            {
                VehiclePosition vehiclePosition = getVehiclePosition(centroid);

                if(vehiclePosition != VP_NONE)
                    positions.insert(std::pair<cvb::CvID, VehiclePosition>(id,vehiclePosition));
            }
        }
        else
        {
            if(positions.count(id) > 0)
                positions.erase(positions.find(id));
        }

        //----------------------------------------------------------------------------

        if(points.count(id) > 0)
        {
            std::map<cvb::CvID, std::vector<CvPoint2D64f>>::iterator it2 = points.find(id);
            std::vector<CvPoint2D64f> centroids = it2->second;

            std::vector<CvPoint2D64f> centroids2;
            if(track->inactive == 0 && centroids.size() < 30)
            {
                centroids2.push_back(centroid);

                for(std::vector<CvPoint2D64f>::iterator it3 = centroids.begin() ; it3 != centroids.end(); it3++)
                {
                    centroids2.push_back(*it3);
                }

                for(std::vector<CvPoint2D64f>::iterator it3 = centroids2.begin() ; it3 != centroids2.end(); it3++)
                {
                    cv::circle(img_input, cv::Point((*it3).x,(*it3).y), 3, cv::Scalar(255,255,255), -1);
                }

                points.erase(it2);
                points.insert(std::pair<cvb::CvID, std::vector<CvPoint2D64f>>(id,centroids2));
            }
            else
            {
                points.erase(it2);
            }
        }
        else
        {
            if(track->inactive == 0)
            {
                std::vector<CvPoint2D64f> centroids;
                centroids.push_back(centroid);
                points.insert(std::pair<cvb::CvID, std::vector<CvPoint2D64f>>(id,centroids));
            }
        }
    }

    //--------------------------------------------------------------------------
/*
    std::string countABstr = "A->B: " + std::to_string(countAB);
    std::string countBAstr = "B->A: " + std::to_string(countBA);

    if(showAB == 0)
    {
        cv::putText(img_input, countABstr, cv::Point(10, img_h - 20), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
        cv::putText(img_input, countBAstr, cv::Point(10, img_h - 8), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
    }

    if(showAB == 1)
    cv::putText(img_input, countABstr, cv::Point(10, img_h - 8), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

    if(showAB == 2)
    cv::putText(img_input, countBAstr, cv::Point(10, img_h - 8), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

    if(showOutput)
    cv::imshow("VehicleCouting", img_input);
*/
    firstTime = false;
}




