#include "stdafx.h"
#include "common.h"
#include <queue>

bool inBounds(int i, int j, Mat src)
{
	if (i < 0 || i > src.rows)
	{
		return false;
	}
	if (j < 0 || j > src.cols)
	{
		return false;
	}
	return true;
}

void ignoreLabel(Mat labels, int k)
{
	for (int i = 0; i < labels.rows; i++)
	{
		for (int j = 0; j < labels.cols; j++)
		{
			if (labels.at<int>(i, j) == k)
			{
				labels.at<int>(i, j) = -1;
			}
		}
	}
}

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

void openImage(Mat src, Mat openedImage)
{
	int di[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	int dj[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	bool toErode = false;

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (src.at<uchar>(i, j) == 0)
			{
				for (int k = 0; k < 8; k++)
				{
					if (i + di[k] < src.rows && i + di[k] > 0 && j + dj[k] < src.cols && j + dj[k] > 0)
					{
						if (src.at<uchar>(i + di[k], j + dj[k]) == 255)
						{
							toErode = true;
						}
					}
				}
			}
			if (toErode)
			{
				openedImage.at<uchar>(i, j) = 255;
			}
			toErode = false;
		}
	}

	Mat temp = openedImage.clone();

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (temp.at<uchar>(i, j) == 0)
			{
				for (int k = 0; k < 8; k++)
				{
					if (i + di[k] < src.rows && i + di[k] > 0 && j + dj[k] < src.cols && j + dj[k] > 0)
					{
						openedImage.at<uchar>(i + di[k], j + dj[k]) = 0;
					}
				}
			}
		}
	}
}

void filterSmallAreas(Mat mask)
{
	int di[8] = { -1,-1,-1,0,1,1,1,0 };
	int dj[8] = { -1,0,1,1,1,0,-1,-1 };
	int label = 0;

	Mat_<int> labels(mask.rows, mask.cols, 0);

	//label objects
	for (int i = 0; i < mask.rows; i++)
	{
		for (int j = 0; j < mask.cols; j++)
		{
			if (mask.at<uchar>(i, j) == 255 && labels(i, j) == 0)
			{
				label++;
				std::queue<Point> Q;
				labels(i, j) = label;
				Q.push(Point(j, i));
				while (!Q.empty())
				{
					Point p = Q.front();
					Q.pop();
					for (int k = 0; k < 7; k++)
					{
						int posi = p.y + di[k];
						int posj = p.x + dj[k];
						if (posi >= 0 && posi < mask.rows && posj >= 0 && posj < mask.cols)
						{
							if (mask.at<uchar>(posi, posj) == 255 && labels(posi, posj) == 0)
							{
								labels(posi, posj) = label;
								Q.push(Point(posj, posi));
							}
						}
					}
				}
			}
		}
	}

	int toIgnore[100];
	int countK = 0; //area of labeled object
	float t = 0; //thinness ratio
	bool ok; //border flag
	float p; //perimeter

	for (int k = 1; k < label; k++)
	{ 
		ok = false;
		p = 0;
		countK = 0;

		for (int i = 0; i < labels.rows; i++)
		{
			for (int j = 0; j < labels.cols; j++)
			{
				if (labels.at<int>(i, j) == k)
				{
					countK++;	//increment area

					//check if pixel is on the border
					for (int kk = 0; kk <= 7; kk++)
					{
						int posi = i + di[kk];
						int posj = j + dj[kk];
						if (posi >= 0 && posi < mask.rows && posj >= 0 && posj < mask.cols)
						{
							if (labels.at<int>(posi, posj) != k)
							{
								ok = true;
							}
						}
					}
					if (ok)
					{
						p++;	//increment permimeter
					}

					ok = false;
				}
			}
		}

		//p *= CV_PI / 4;

		// thinness ratio
		t = 4 * CV_PI * countK / (p * p);
		//printf("%d -> %f -> %f\n", countK, p, t);

		if (countK < 100 || t < 0.21) //check if labeled object has a small area or isnt round
		{
			ignoreLabel(labels, k); //ignore the object
		}
	}

	//get rid off unwanted areas
	for (int i = 0; i < mask.rows; i++)
	{
		for (int j = 0; j < mask.cols; j++)
		{
			if (labels.at<int>(i, j) == -1)
			{
				mask.at<uchar>(i, j) = 0;
			}
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

	//Mat openedMask = redMask.clone();
	//openImage(src, openedMask);
	imshow("Mask", redMask);

	filterSmallAreas(redMask);

	//fix red eye effect
	Mat fixedImage = fixRedEye(redMask, src);

	imshow("Original image", src);
	imshow("Red mask", redMask);
	//imshow("Opened mask", openedMask);
	imshow("Fixed image", fixedImage);
	waitKey();
}