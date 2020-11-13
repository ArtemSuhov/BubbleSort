#include "pch.h"
#include "TXLib.h"
#include <math.h>
#include <iostream>

RGBQUAD* Video_memory = NULL;

typedef double(*MathPlot) (double x);
typedef void(*Action) ();

struct Point {
	float x;
	float y;
	float z;

	Point operator+ (Point a) {
		return { a.x + x, a.y + y, a.z + z };
	}

	Point operator- (Point a) {
		return { a.x - x, a.y - y, a.z - z };
	}
};


class Vector {
public:	
	Vector() {
		this->setX(0);
		this->setY(0);
		this->setZ(0);
	}

	Vector(Point a) {
		this->setX(a.x);
		this->setY(a.y);
		this->setZ(a.z);
	}

	Vector(float x, float y) {
		this->setX(x);
		this->setY(y);
		this->setZ(0);
	}

	Vector(float x, float y, float z) {
		this->setX(x);
		this->setY(y);
		this->setZ(z);
	}

	void equalize(Vector a) {
		coords.x = a.getX();
		coords.y = a.getY();
		coords.z = a.getZ();
	}

	void setX(float x) {
		coords.x = x;
	}

	void setY(float y) {
		coords.y = y;
	}

	void setZ(float z) {
		coords.z = z;
	}

	float getX() {
		return coords.x;
	}

	float getY() {
		return coords.y;
	}

	float getZ() {
		return coords.z;
	}

	float length() {
		return sqrt(coords.x * coords.x + coords.y * coords.y + coords.z * coords.z);
	}

	Vector operator+ (Vector a) {
		return Vector(coords.x + a.getX(), coords.y + a.getY(), coords.z + a.getZ());
	}

	Vector operator* (float a) const {
		return Vector(coords.x * a, coords.y * a, coords.z * a);
	}

	Vector operator- (Vector a) {
		return *this + (a * (-1));
	}

	Vector normalize()
	{
		float inv_length = this->length();

		if (inv_length != 0) {
			inv_length = 1.0f / inv_length;
		}

		return *this * (inv_length);
	}

	void DrawVector(Point a) {
		DrawVectorWithoutArrow(a);
		Point end = this->coords + a;
		DrawArrow(end);
	}

	Vector planeOrd() {
		return Vector(this->coords.y, -this->coords.x);
	}

private:
	Point coords;
	const int arrowSize = 10;

	void DrawVectorWithoutArrow(Point a) {
		txLine(a.x, a.y, a.x + coords.x, a.y + coords.y);
	}

	void DrawArrow(Point a) {
		Vector firstArrow  = this->planeOrd();
		Vector secondArrow = this->planeOrd() * - 1;

		((firstArrow.normalize()  + this->normalize()) * -arrowSize).DrawVectorWithoutArrow(a);
		((secondArrow.normalize() + this->normalize()) * -arrowSize).DrawVectorWithoutArrow(a);
	}
};

class Plane {
public:
	Plane(Vector size, Vector xStep, Vector yStep) {
		this->size.equalize(size);
		this->xStep.equalize(xStep);
		this->yStep.equalize(yStep);
		txCreateWindow(size.getX(), size.getY());
		txBegin();
	}

	void setSize(Vector size) {
		this->size.equalize(size);
	}

	void setXStep(Vector xStep) {
		this->xStep.equalize(xStep);
	}

	void setYStep(Vector yStep) {
		this->yStep.equalize(yStep);
	}

	Vector getSize() {
		return size;
	}

	Vector getXStep() {
		return xStep;
	}
	
	Vector getYStep() {
		return yStep;
	}

	void drawPixel(Vector vector, Vector color) {
		Point pixel;
		float x = vector.getX();
		float y = vector.getY();
		pixel.x = xStep.getX() * x + yStep.getX() * y;
		pixel.y = xStep.getY() * x + yStep.getY() * y;
		txSetPixel(pixel.x, pixel.y, RGB(color.getX(), color.getY(), color.getZ()));
	}

	void drawPixel(Point point, Vector color) {
		Point pixel;
		float x = point.x;
		float y = point.y;
		pixel.x = xStep.getX() * x + yStep.getX() * y;
		pixel.y = xStep.getY() * x + yStep.getY() * y;

		txSetPixel(pixel.x, pixel.y, RGB(color.getX(), color.getY(), color.getZ()));
	}


	void drawPixel(float x, float y, Vector color) {
		Point pixel;
		pixel.x = xStep.getX() * x + yStep.getX() * y;
		pixel.y = xStep.getY() * x + yStep.getY() * y;
		txSetPixel(pixel.x, pixel.y, RGB(color.getX(), color.getY(), color.getZ()));
	}

private:
	Vector size;
	Vector xStep;
	Vector yStep;
};

class CoordSys : Plane {
public:
	CoordSys(Vector size, Vector xStep, Vector yStep, Point zeroPoint, Vector shift, Vector scaleStep) 
		: Plane(Vector(size.getX() + zeroPoint.x + shift.getX(), zeroPoint.y + shift.getY()), xStep, yStep) {
		this->zeroPoint = zeroPoint;
		this->size.equalize(size);
		this->shift.equalize(shift);
		this->scaleStep.equalize(scaleStep);
	}

	void drawAxes() {
		Vector xAxis = Vector(size.getX(), 0, 0);
		Vector yAxis = Vector(0, -size.getY(), 0);
		xAxis.DrawVector(zeroPoint);
		yAxis.DrawVector(zeroPoint);
	}

	void drawFunction(MathPlot func) {
		float x = 0;
		while(x < size.getX()) {
			float y = func(x * scaleStep.getX()) * scaleStep.getY();

			if (y < size.getY()) {
				drawPixel(Point{ x + zeroPoint.x, zeroPoint.y - y , 0 }, Vector(255, 255, 255));
			}

			x++;
		}
	}

private:
	Point zeroPoint;
	Vector size;
	Vector shift;
	Vector scaleStep;
};

struct Light {
	Vector color;
	Vector position;
};

Vector channelMul(Vector a, Vector b) {
	return Vector(a.getX() * b.getX(), a.getY() * b.getY(), a.getZ() * b.getZ());
}

float operator^(Vector a, Vector b) {
	return a.getX() * b.getX() + a.getY() * b.getY() + a.getZ() * b.getZ();
};

Vector colorise(Vector color) {
	float r = color.getX();
	float g = color.getY();
	float b = color.getZ();
	r = r > 255 ? 255 : r;
	g = g > 255 ? 255 : g;
	b = b > 255 ? 255 : b;

	return Vector(r, g, b);
}

const Vector FillingLight = Vector(51, 51, 51);
const float MaterialShape = 40;

Vector getLightedPointColor(Light* lights, int lightsCount, Vector camera, Vector point, Vector center, Vector sphereColor) {
	Vector resultColor = channelMul(sphereColor * (1. / 255), FillingLight * (1. / 255)) * 255;
	Vector normalVector = (point - center).normalize();

	for (int i = 0; i < lightsCount; i++) {
		Vector pointToLight = (lights[i].position - point).normalize();
		Vector pointToCamera = (camera - point).normalize();
		Vector reflectedLight = (normalVector * 2 - pointToLight).normalize();

		float reflCos = pointToCamera ^ reflectedLight;
		float angleCos = pointToLight ^ normalVector;

		float reflIntensity = reflCos > 0 ? pow(reflCos, MaterialShape) : 0;
		float intensity = angleCos > 0 ? angleCos : 0;

		Vector currentColor = channelMul(sphereColor * (1. / 255), lights[i].color * (1. / 255)) * 255;

		resultColor.equalize(colorise(resultColor + currentColor * intensity + lights[i].color * reflIntensity));
	}

	return resultColor;
}

void drawLightedSphere(Plane plane, Point center, float radius, Vector camera, Light* lights, int lightsCount, Vector sphereColor) {
	Vector size = plane.getSize();
	float xSize = size.getX();
	float ySize = size.getY();

	for (int x = 0; x < xSize; x++) {
		for (int y = 0; y < ySize; y++) {
			Point point = { x, y };
			Point spherePoint = point - center;

			if (Vector(spherePoint).length() < radius) {
				float z = sqrt(radius * radius - spherePoint.x * spherePoint.x - spherePoint.y * spherePoint.y);
				Vector color = getLightedPointColor(lights, lightsCount, camera, Vector(point.x, point.y, z), Vector(center), sphereColor);
				plane.drawPixel(point, color);
			}
		}
	}
}

class Button {
public:
	Button(const char* name, Point position, Vector size, Action func) :
		name(name), 
		position(position),
		func(func)
	{
		this->size.equalize(size);
	}

	void drawButton() {
		txSetFillColor(TX_WHITE);
		txRectangle(position.x, position.y, position.x + size.getX(), position.y + size.getY());
		txSetColor(TX_BLACK);
		txDrawText(position.x, position.y, position.x + size.getX(), position.y + size.getY(), name);
	}

	bool isPressed(float x, float y) {
		float eqX = x - position.x;
		float eqY = y - position.y;

		return eqX > 0 && eqY > 0 && eqX < size.getX() && eqY < size.getX();
	}
	
	void press() {
		func();
	}

private:
	const char* name;
	Point position;
	Vector size;
	Action func;
};

class ButtonPresser {
public:
	ButtonPresser(Button* buttons, int count) {
		this->count = count;
		this->buttons = buttons;
	}

	void press() {
		for (int i = 0; i < count; i++) {
			if (buttons[i].isPressed(txMouseX(), txMouseY())) {
				buttons[i].press();
			}
		 }
	}

private:
	Button* buttons;
	int count;
};

CoordSys* GlobalCoord = NULL;

void drawSin() {
	GlobalCoord->drawFunction(sin);
}

void drawCos() {
	GlobalCoord->drawFunction(cos);
}

void drawTan() {
	GlobalCoord->drawFunction(tan);
}

void clear() {
	txSetFillColor(TX_BLACK);
	txClear();
	txSetColor(TX_WHITE);
	GlobalCoord->drawAxes();
}

int main()
{
	Vector size = Vector(700, 400);
	Vector xStep = Vector(1, 0);
	Vector yStep = Vector(0, 1);
	Point zeroOfAxis = Point{ 50, 650 };
	Vector shift = Vector(50, 50);

	Video_memory = txVideoMemory();

	CoordSys coordSys = CoordSys(size, xStep, yStep, zeroOfAxis, shift, Vector(0.002, 150));
	GlobalCoord = &coordSys;

	txSetColor(TX_WHITE);
	GlobalCoord->drawAxes();

	const int count = 4;
	Button sinButton = Button("SIN",   Point{ 100, 100 }, Vector(50, 50), drawSin);
	Button cosButton = Button("COS",   Point{ 300, 100 }, Vector(50, 50), drawCos);
	Button tanButton = Button("TAN",   Point{ 500, 100 }, Vector(50, 50), drawTan);
	Button clrButton = Button("CLEAR", Point{ 700, 100 }, Vector(50, 50), clear);
	Button buttons[count] = { sinButton, cosButton, tanButton, clrButton };

	ButtonPresser presser = ButtonPresser(buttons, count);
	while (true) {
		for (int i = 0; i < count; i++) {
			buttons[i].drawButton();
		}

		if (txMouseButtons() == 1) {
			presser.press();
		}
		txSleep();
	}
}
