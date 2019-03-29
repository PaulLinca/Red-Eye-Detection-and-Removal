#include "stdafx.h"
#include "common.h"

void maskRedColor(Mat src, Mat redMask)
{
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			Vec3b pixel = src.at<Vec3b>(i, j); //current pixel of the original image

			float h; //the angle between the current color and the ray corresponding to the color Red

			float r = (float)pixel[2] / 255; //normalized R component
			float g = (float)pixel[1] / 255; //normalized G component
			float b = (float)pixel[0] / 255; //normalized B component

			float maximum = max(r, max(g, b));
			float minimum = min(r, min(g, b));
			float C = maximum - minimum;

			//calculate hue value
			if (C != 0)
			{
				if (maximum == r) h = 60 * (g - b) / C;
				if (maximum == g) h = 120 + 60 * (b - r) / C;
				if (maximum == b) h = 240 + 60 * (r - g) / C;
			}
			else
			{
				h = 0;
			}

			//saturate the hue
			if (h < 0)
			{
				h += 360;
			}

			//check if the hue is in the range of the red color
			if ((h >= 350 && h <= 360) || (h >= 0 && h <= 5))
			{
				redMask.at<uchar>(i, j) = 255; //color the pixel with white
			}
			else redMask.at<uchar>(i, j) = 0; //color the pixel with black
		}
	}
}

int main()
{
	//open original image
	Mat src;
	char file[MAX_PATH];
	if (openFileDlg(file))
	{
		src = imread(file, CV_LOAD_IMAGE_COLOR);
	}

	//create a mask for the red colors
	Mat redMask(src.rows, src.cols, CV_8UC1);

	maskRedColor(src, redMask);

	imshow("Red mask", redMask);
	imshow("Original image", src);
	waitKey();
}