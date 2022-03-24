#include <iostream>
#include <thread>
#include <chrono>
#include <math.h>
#include <GL/glut.h>
#include <list>

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
	void deactivate(){
		this->active = false;
	}

	bool isActive(){
		return active;
	}

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
		setVelocity(velocity.getX(), velocity.getY());
	}

	void setVelocity(double x, double y){
		this->velocity = PlanarVector(x, y);
	}
private:
	bool active{true};
	int bouncesCount{};
	PlanarVector position;
	PlanarVector velocity;
};

void drawCircle(PlanarVector& position, double radius){
	static double SQRT_2 = sqrt(2.0);
	double x = position.getX();
	double y = position.getY();
	double scaledRadius = radius / SQRT_2;
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(x, y, 0.0);
        glVertex3f(x + radius, y, 0.0);
        glVertex3f(x + scaledRadius, y + scaledRadius, 0.0);
        glVertex3f(x, y + radius, 0.0);
        glVertex3f(x - scaledRadius, y + scaledRadius, 0.0);
        glVertex3f(x - radius, y, 0.0);
        glVertex3f(x - scaledRadius, y - scaledRadius, 0.0);
        glVertex3f(x, y - radius, 0.0);
        glVertex3f(x + scaledRadius, y - scaledRadius, 0.0);
        glVertex3f(x + radius, y, 0.0);
    glEnd();
}

void dupa(Ball* ball){
	while(ball->getBounceCount() < 6){
		ball->move();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		PlanarVector position = ball->getPosition();
		if(position.getX() < -1.0 || position.getX() > 1.0){
			ball->bounceVerticalWall();
		}
		if(position.getY() < -1.0 || position.getY() > 1.0){
			ball->bounceHorizontalWall();
		}			
	}
	ball->deactivate();
}

/*int main(){
	Ball* cycki = new Ball();
	cycki->setPosition(0, 0);
	cycki->setVelocity(0.03, 0.06);
	std::thread yetAnotherThread(std::ref(dupa), cycki);
	while(cycki->isActive()){
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		std::cout << cycki->getPosition().getX() << " " << cycki->getPosition().getY() << " " << cycki->getBounceCount() << "\n";
	}
	delete cycki;
	yetAnotherThread.join();
	return 0;
}*/

class ApplicationState{
private:
	static std::list<Ball*> balls;
public:
    static void displayMe(void){
        glClear(GL_COLOR_BUFFER_BIT);
        double radius = 0.02;

        std::list<Ball*>::iterator i = balls.begin();
		while (i != balls.end())
		{
    		bool isActive = (*i)->isActive();
    		if (!isActive)
    		{
    			delete (*i);
        		balls.erase(i++);  // alternatively, i = items.erase(i);
    		}
    		else
    		{
        		drawCircle((*i)->getPosition(), radius);
        		++i;
    		}
		}

        glFlush();
    }

    static void revalidateBalls(){

    }

    static void addBall(Ball* ball){
    	balls.push_back(ball);
		std::thread yetAnotherThread(std::ref(dupa), ball);
		yetAnotherThread.detach();
    }
};

std::list<Ball*> ApplicationState::balls = std::list<Ball*>();

void forceRefresh(int data){
	glutTimerFunc(20, forceRefresh, -1);
	glutPostRedisplay();
}
 
int main(int argc, char** argv)
{
	PlanarVector velocity = PlanarVector(0.006, 0.005);
	Ball* ball = new Ball();
	ball->setVelocity(velocity);
	//std::thread otherThread(ApplicationState::addBall, ball);
	ApplicationState::addBall(ball);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Hello world!");
    glutDisplayFunc(ApplicationState::displayMe);
    glutTimerFunc(20, forceRefresh, -1);
    glutMainLoop();
    return 0;
}
