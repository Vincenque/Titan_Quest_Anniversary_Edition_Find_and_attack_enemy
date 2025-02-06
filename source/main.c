#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <wingdi.h>

typedef struct {
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	unsigned char *data;
} Image;

void saveBMP(const char *filename, Image *img);
void captureScreenshot(Image *img, const char *filename);
void convertToGrayscale(Image *img, Image *grayImg, const char *filename);
void findDifferenceAndMark(Image *img1, Image *img2, Image *outputImage, int *diffX, int *diffY, const char *filename);

void captureScreenshot(Image *img, const char *filename) {
	HDC hScreen = GetDC(NULL);
	HDC hDC = CreateCompatibleDC(hScreen);
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
	SelectObject(hDC, hBitmap);
	BitBlt(hDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);
	ReleaseDC(NULL, hScreen);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	img->infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	img->infoHeader.biWidth = width;
	img->infoHeader.biHeight = -height; // Negative to ensure top-down DIB
	img->infoHeader.biPlanes = 1;
	img->infoHeader.biBitCount = 32;
	img->infoHeader.biCompression = BI_RGB;
	img->infoHeader.biSizeImage = 0;

	DWORD dwBmpSize = ((width * img->infoHeader.biBitCount + 31) / 32) * 4 * height;
	img->data = (unsigned char *)malloc(dwBmpSize);

	GetDIBits(hDC, hBitmap, 0, (UINT)height, img->data, (BITMAPINFO *)&img->infoHeader, DIB_RGB_COLORS);

	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		free(img->data);
		DeleteObject(hBitmap);
		DeleteDC(hDC);
		return;
	}

	DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	img->fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	img->fileHeader.bfSize = dwSizeofDIB;
	img->fileHeader.bfType = 0x4D42;

	DWORD dwBytesWritten;
	WriteFile(hFile, &img->fileHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, &img->infoHeader, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, img->data, dwBmpSize, &dwBytesWritten, NULL);
	CloseHandle(hFile);

	DeleteObject(hBitmap);
	DeleteDC(hDC);
}

void convertToGrayscale(Image *img, Image *grayImg, const char *filename) {
	*grayImg = *img;
	int imageSize = img->infoHeader.biWidth * abs(img->infoHeader.biHeight) * 4;
	grayImg->data = (unsigned char *)malloc(imageSize);

	for (int i = 0; i < imageSize; i += 4) {
		unsigned char gray = (img->data[i] + img->data[i + 1] + img->data[i + 2]) / 3;
		grayImg->data[i] = gray;
		grayImg->data[i + 1] = gray;
		grayImg->data[i + 2] = gray;
		grayImg->data[i + 3] = 255;
	}
	saveBMP(filename, grayImg);
}

void findDifferenceAndMark(Image *img1, Image *img2, Image *outputImage, int *diffX, int *diffY, const char *filename) {
	*outputImage = *img1;
	int width = img1->infoHeader.biWidth;
	int height = abs(img1->infoHeader.biHeight);
	int imageSize = width * height * 4;
	outputImage->data = (unsigned char *)malloc(imageSize);

	typedef struct {
		int x, y, diff;
	} Difference;

	Difference topDiffs[11] = {0};

	int threshold = 0;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int idx = (y * width + x) * 4;
			if ((x < 90 && y < 80) || (x >= 890 && x < 1040 && y >= 490 && y < 660)) {
				outputImage->data[idx] = 0;
				outputImage->data[idx + 1] = 0;
				outputImage->data[idx + 2] = 0;
				outputImage->data[idx + 3] = 255;
				continue;
			}

			unsigned char diff = abs(img1->data[idx] - img2->data[idx]);
			diff = (diff < threshold) ? 0 : diff;
			outputImage->data[idx] = diff;
			outputImage->data[idx + 1] = diff;
			outputImage->data[idx + 2] = diff;
			outputImage->data[idx + 3] = 255;

			for (int i = 0; i < 11; i++) {
				if (diff > topDiffs[i].diff) {
					for (int j = 10; j > i; j--) {
						topDiffs[j] = topDiffs[j - 1];
					}
					topDiffs[i] = (Difference){x, y, diff};
					break;
				}
			}
		}
	}

	int count[11] = {0};
	int bestIdx = 0;
	for (int i = 0; i < 11; i++) {
		for (int j = 0; j < 11; j++) {
			if (i != j && abs(topDiffs[i].x - topDiffs[j].x) <= 10 && abs(topDiffs[i].y - topDiffs[j].y) <= 10) {
				count[i]++;
			}
		}
		if (count[i] > count[bestIdx]) {
			bestIdx = i;
		}
	}
	*diffX = topDiffs[bestIdx].x;
	*diffY = topDiffs[bestIdx].y;

	for (int i = 0; i < 11; i++) {
		int startX = topDiffs[i].x;
		int startY = topDiffs[i].y;
		for (int y = startY; y < startY + 10 && y < height; y++) {
			for (int x = startX; x < startX + 10 && x < width; x++) {
				int idx = (y * width + x) * 4;
				outputImage->data[idx] = 0;
				outputImage->data[idx + 1] = 0;
				outputImage->data[idx + 2] = 255;
			}
		}
	}

	saveBMP(filename, outputImage);
}

void saveBMP(const char *filename, Image *img) {
	FILE *file = fopen(filename, "wb");
	fwrite(&img->fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
	fwrite(&img->infoHeader, sizeof(BITMAPINFOHEADER), 1, file);
	fwrite(img->data, img->infoHeader.biSizeImage, 1, file);
	fclose(file);
}

int main() {
	Image img1, img2, gray1, gray2, diffImage;
	int diffX = 0, diffY = 0, toggle = 0;

	while (1) {
		if (GetAsyncKeyState(VK_ESCAPE)) {
			break;
		}
		if (GetAsyncKeyState('A')) {
			int i = 0;
			SetCursorPos(1920 / 2 + (toggle ? 100 : -100), 1080 / 2 + 80);
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			toggle = !toggle;

			Sleep(500);
			captureScreenshot(&img1, "assets/screenshot1.bmp");
			Sleep(200);
			captureScreenshot(&img2, "assets/screenshot2.bmp");

			convertToGrayscale(&img1, &gray1, "assets/screenshot1_gray.bmp");
			convertToGrayscale(&img2, &gray2, "assets/screenshot2_gray.bmp");

			findDifferenceAndMark(&gray1, &gray2, &diffImage, &diffX, &diffY, "assets/difference_marked.bmp");

			SetCursorPos(diffX, diffY);
			printf("Mouse moved to highest difference at (%d, %d)\n", diffX, diffY);

			free(img1.data);
			free(img2.data);
			free(gray1.data);
			free(gray2.data);
			free(diffImage.data);
			Sleep(1000);
		}
	}

	return 0;
}
