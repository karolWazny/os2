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

	void bounceVerticalWall(){
		velocity.setX(-velocity.getX());
		bouncesCount++;
	}

	void bounceHorizontalWall(){
		velocity.setY(-velocity.getY());
		bouncesCount++;
	}

	int getBounceCount(){
		return bouncesCount;
	};

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
	int bouncesCount{};
	PlanarVector position;
	PlanarVector velocity;
};

void dupa(Ball* ball){
	ball->setPosition(0, 0);
	ball->setVelocity(0.03, 0.06);
	while(ball->getBounceCount() < 8){
		ball->move();
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		PlanarVector position = ball->getPosition();
		if(position.getX() <= 0 || position.getX() >= 8){
			ball->bounceVerticalWall();
		}
		if(position.getY() <= 0 || position.getY() >= 5){
			ball->bounceHorizontalWall();
		}
	}
}

int main(){
	Ball cycki;
	cycki.setPosition(0, 0);
	cycki.setVelocity(0.03, 0.06);
	std::thread yetAnotherThread(std::ref(dupa), &cycki);
	while(cycki.getBounceCount() < 8){
		std::this_thread::sleep_for(std::chrono::milliseconds(510));
		std::cout << cycki.getPosition().getX() << " " << cycki.getPosition().getY() << " " << cycki.getBounceCount() << "\n";
	}
	yetAnotherThread.join();
	return 0;
}