#include "stdafx.h"
#include "common.h"

int main()
{
	//open original image
	Mat src;
	char file[MAX_PATH];
	if (openFileDlg(file))
	{
		src = imread(file, CV_LOAD_IMAGE_COLOR);
	}

	//clone original image
	Mat redEyeRemoved = src.clone();

	CascadeClassifier eyes_cascade("C:\\opencv\\opencv\\build\\etc\\haarcascades\\haarcascade_eye.xml");

	std::vector<Rect> eyes;

	eyes_cascade.detectMultiScale(src, eyes, 1.3, 4, 0 | CASCADE_SCALE_IMAGE, Size(100, 100));
	
	for (size_t j = 0; j < eyes.size(); j++)
	{
		Point center(eyes[j].x + eyes[j].width*0.5, eyes[j].y + eyes[j].height*0.5);
		int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
		circle(src, center, radius, Scalar(255, 0, 0), 4, 8, 0);
	}

	imshow("Original image", src);
	waitKey();
}