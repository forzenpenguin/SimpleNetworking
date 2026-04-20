class Cylinder {
public:
	Cylinder() = default;
	Cylinder(double r, double h);
	double volume() const;
	double getRadius() const;
	double getHeight() const;
private:
	double radius, height;
};
