#include "Cylinder.h"
#include "constant.h"
Cylinder::Cylinder(double r, double h) : radius(r), height(h) {}
double Cylinder::volume() const {
	return PI * radius * radius * height;
}
double Cylinder::getRadius() const {
	return radius;
}
double Cylinder::getHeight() const {
	return height;
}