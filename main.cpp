#include <iostream>
#include <thread>
#include <chrono>

class PlanarVector{
private:
	double x{};
	double y{};
public:
	PlanarVector(const double x = 0.0, const double y = 0.0) : x(x), y(y){};

	PlanarVector operator+(PlanarVector& other){
		return PlanarVector(x + other.x, y + other.y);
	}

	PlanarVector& operator+=(PlanarVector& other){
		x += other.x;
		y += other.y;
		return *this;
	}

	void setX(double x){
		this->x = x;
	}

	void setY(double y){
		this->y = y;
	}

	double getX(){
		return x;
	}

	double getY(){
		return y;
	}
};

class Ball{
public:
	void move(){
		position += velocity;
	}

	PlanarVector& getPosition(){
		return position;
	}

	PlanarVector& getVelocity(){
		return velocity;
	}

	void setPosition(PlanarVector& position){
		setPosition(position.getX(), position.getY());
	}

	void setPosition(double x, double y){
		this->position = PlanarVector(x, y);
	}

	void setVelocity(PlanarVector& velocity){
		setVelocity(position.getX(), position.getY());
	}

	void setVelocity(double x, double y){
		this->velocity = PlanarVector(x, y);
	}


private:
	PlanarVector position;
	PlanarVector velocity;
};

int main(){
	Ball ball;
	ball.setPosition(0, 0);
	ball.setVelocity(0.01, 0.02);
	while(true){
		ball.move();
		std::this_thread::sleep_for(std::chrono::milliseconds(750));
		std::cout << "dupa: " << ball.getPosition().getX() << " " << ball.getPosition().getY() << "\n";
	}
	return 0;
}