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
			if ((h >= 320 && h <= 360) || (h >= 0 && h <= 5))
			{
				redMask.at<uchar>(i, j) = 255; //color the pixel with white
			}
			else redMask.at<uchar>(i, j) = 0; //color the pixel with black
		}
	}
}

Mat fixRedEye(Mat redMask, Mat src)
{
	Mat fixedImage = src.clone();

	//for each masked pixel average the red and blue channels of the original image and give the resulting value to the fixed image pixel
	for (int i = 0; i < redMask.rows; i++)
	{
		for (int j = 0; j < redMask.cols; j++)
		{
			Vec3b pixel = src.at<Vec3b>(i, j);

			if (redMask.at<uchar>(i, j) == 255)
			{
				int blue = pixel[0];
				int green = pixel[1];
				int average = (blue + green) / 2;

				fixedImage.at<Vec3b>(i, j) = Vec3b(average, average, average);
			}
			else
			{
				fixedImage.at<Vec3b>(i, j) = pixel;
			}
		}
	}

	return fixedImage;
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

	//fix red eye effect
	Mat fixedImage = fixRedEye(redMask, src);

	imshow("Original image", src);
	imshow("Red mask", redMask);
	imshow("Fixed image", fixedImage);
	waitKey();
}